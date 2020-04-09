#include <fs/nvs.h>
#include <string.h>
#include <zephyr.h>

#include "pdb.h"

#include "devicetree_fixups.h"
#include "pdb_threads.h"
#include "pdb_custom_functions.h"
#include "pdb_callbacks.h"

int pdb_thread(void);

#define NVS_SECTOR_SIZE $nvs_sector_size
#define NVS_SECTOR_COUNT $nvs_sector_count
#define NVS_STORAGE_OFFSET $nvs_storage_offset

$channels_sems

K_THREAD_DEFINE(pdb_thread_id, PDB_THREAD_SIZE, pdb_thread, NULL, NULL, NULL,
                    PDB_THREAD_PRIORITY, 0, K_NO_WAIT);


$arrays_init

static struct nvs_fs pdb_fs = {
    .sector_size  = NVS_SECTOR_SIZE,
    .sector_count = NVS_SECTOR_COUNT,
    .offset       = NVS_STORAGE_OFFSET,
};

$channels_creation

static pdb_channel_t *pdb_channel_get_ref(pdb_channel_e id)
{
    if (id < PDB_CHANNEL_COUNT) {
        return &__pdb_channels[id];
    } else {
        return NULL;
    }
}

size_t pdb_channel_size(pdb_channel_e id, int *error)
{
    size_t size      = 0;
    pdb_channel_t *p = pdb_channel_get_ref(id);
    if (p) {
        size = (size_t) p->size;
    } else {
        if (error) {
            *error = -EINVAL;
        }
    }
    return size;
}

int pdb_channel_get(pdb_channel_e id, u8_t *channel_value, size_t size)
{
    int error              = 0;
    pdb_channel_t *channel = pdb_channel_get_ref(id);
    PDB_CHECK(channel, -ENODEV, "The channel %d was not found!\n", id);
    PDB_CHECK(channel->get, -EPERM, "The channel %d does not have get implementation!\n", id);
    PDB_CHECK(channel->pre_get(id), -EIO, "Error in pre-get function of channel %d\n", id);
    error = channel->get(id, channel_value, size);
    if (error) {
        printk("Current channel get: %d, error code: %d\n", id, error);
    }
    return error;
}

int pdb_channel_get_private(pdb_channel_e id, u8_t *channel_value, size_t size)
{
    int ret = 0;
    if (id < PDB_CHANNEL_COUNT) {
        pdb_channel_t *current_channel = &__pdb_channels[id];
        if (current_channel->size == size) {
            if (k_sem_take(current_channel->sem, K_MSEC(200))) {
                printk("Could not get the channel. Channel is busy\n");
                ret = -EBUSY;
            } else {
                memcpy(channel_value, current_channel->data, current_channel->size);
                k_sem_give(current_channel->sem);
            }
        } else {
            ret = -EINVAL;
        }
    } else {
        ret = -ENODATA;
    }
    return ret;
}

int pdb_channel_set(pdb_channel_e id, u8_t *channel_value, size_t size)
{
    int error              = 0;
    int valid              = 1;
    pdb_channel_t *channel = pdb_channel_get_ref(id);
    const k_tid_t *p_id;

    for(p_id = channel->publishers_id ; *p_id != NULL ; ++p_id) {
        if (*p_id == k_current_get()) {
            break;
        }
    }

    PDB_CHECK(p_id, -EPERM, "The current thread has not the permission to change channel %d!\n", id);
    PDB_CHECK_VAL(channel, NULL, -ENODEV, "The channel %d was not found!\n", id);
    PDB_CHECK_VAL(channel->set, NULL, -EPERM, "The channel %d is read only!\n", id);
    if (channel->validate) {
        valid = channel->validate(channel_value, size);
    }
    PDB_CHECK(valid, -EINVAL, "The value doesn't satisfy valid function of channel %d!\n", id);
    PDB_CHECK(channel->pre_set(id), -EINVAL, "Error on pre_set function of channel %d!\n", id);
    error = channel->set(id, channel_value, size);
    if (error) {
        printk("Current channel set: %d, error code: %d!\n", id, error);
    }
    PDB_CHECK(channel->pos_set(id), -EINVAL, "Error on pre_set function of channel %d!\n", id);
    return error;
}

int pdb_channel_set_private(pdb_channel_e id, u8_t *channel_value, size_t size)
{
    int ret = 0;
    if (id < PDB_CHANNEL_COUNT) {
        pdb_channel_t *current_channel = &__pdb_channels[id];
        if (current_channel->size == size) {
            if (k_sem_take(current_channel->sem, K_MSEC(200))) {
                printk("Could not set the channel. Channel is busy\n");
                ret = -EBUSY;
            } else {
                if (memcmp(current_channel->data, channel_value, size)) {
                    memcpy(current_channel->data, channel_value, current_channel->size);
                    if (current_channel->changed < 255) {
                        current_channel->changed++;
                    }
                    k_sem_give(current_channel->sem);
                } else {
                    k_sem_give(current_channel->sem);
                }
            }
        } else {
            ret = -EINVAL;
        }
    } else {
        ret = -ENODATA;
    }
    return ret;
}

static void __pdb_recover_data_from_flash(void)
{
    int rc = 0;
    printk("Start...\n");
    for (u16_t id = 0; id < PDB_CHANNEL_COUNT; ++id) {
        if (__pdb_channels[id].persistent) {
            if (!k_sem_take(__pdb_channels[id].sem, K_SECONDS(5))) {
                rc = nvs_read(&pdb_fs, id, __pdb_channels[id].data, __pdb_channels[id].size);
                if (rc > 0) { /* item was found, show it */
                    printk("Id: %d, value:", id);
                    for (size_t i = 0; i < __pdb_channels[id].size; i++) {
                        printk(" %02X", __pdb_channels[id].data[i]);
                    }
                    printk("|");
                    for (size_t i = 0; i < __pdb_channels[id].size; i++) {
                        if (32 <= __pdb_channels[id].data[i]
                            && __pdb_channels[id].data[i] <= 126) {
                            printk("%c", __pdb_channels[id].data[i]);
                        } else {
                            printk(".");
                        }
                    }
                    printk("\n");
                } else { /* item was not found, add it */
                    printk("No values found for channel #%d\n", id);
                }
                k_sem_give(__pdb_channels[id].sem);
            }
            else {
                printk("Could not recover the channel. Channel is busy\n");                 }
        }
    }
}

static void __pdb_persist_data_on_flash(void)
{
    int bytes_written = 0;
    for (u16_t id = 0; id < PDB_CHANNEL_COUNT; ++id) {
        if (__pdb_channels[id].persistent) {
            if (__pdb_channels[id].changed) {
                // printk("Store changes for channel: %d", id);
                bytes_written =
                    nvs_write(&pdb_fs, id, __pdb_channels[id].data, __pdb_channels[id].size);
                if (bytes_written > 0) { /* item was found and updated*/
                    __pdb_channels[id].changed = 0;
                    printk("Channel #%d value updated on the flash\n", id);
                } else if (bytes_written == 0) {
                    // printk("Channel #%d value is already on the flash.", id);
                } else { /* item was not found, add it */
                    printk("Channel #%d could not be stored\n", id);
                }
            }
        } else {
            __pdb_channels[id].changed = 0;
        }
    }
}

int pdb_thread(void)
{
    /* #TODO: Alterar o funcionamento da thread do pdb */
    $set_publishers
    int error = nvs_init(&pdb_fs, DT_FLASH_DEV_NAME);
    if (error) {
        printk("Flash Init failed\n");
    } else {
        printk("NVS started...[OK]\n");
    }
    __pdb_recover_data_from_flash();

    while (1) {
        k_sleep(K_SECONDS(10));
        __pdb_persist_data_on_flash();
    }
    return 0;
}

const char *pdb_channel_name(pdb_channel_e id)
{
    pdb_channel_t *p = pdb_channel_get_ref(id);

    if (p) {
        return p->name;
    }

    return (const char *) p;
}
