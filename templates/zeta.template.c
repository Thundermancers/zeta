/**
 * @file   zeta.template.c
 * @author Lucas Peixoto <lucaspeixotoac@gmail.com>
 *
 *
 */

#include <fs/nvs.h>
#include <logging/log.h>
#include <string.h>
#include <zephyr.h>

#include "devicetree_fixups.h"
#include "zeta.h"
#include "zeta_callbacks.h"
#include "zeta_custom_functions.h"
#include "zeta_threads.h"

LOG_MODULE_REGISTER(ZETA, CONFIG_LOG_DEFAULT_LEVEL);

#define NVS_SECTOR_SIZE $nvs_sector_size
#define NVS_SECTOR_COUNT $nvs_sector_count
#define NVS_STORAGE_OFFSET $nvs_storage_offset

$channels_sems


    void
    zeta_thread(void);
void zeta_thread_nvs(void);
static int zeta_channel_get_private(zeta_channel_e id, u8_t *channel_value, size_t size);
static int zeta_channel_set_private(zeta_channel_e id, u8_t *channel_value, size_t size);

K_THREAD_DEFINE(zeta_thread_id, ZETA_THREAD_NVS_STACK_SIZE, zeta_thread, NULL, NULL, NULL,
                ZETA_THREAD_PRIORITY, 0, K_NO_WAIT);

K_THREAD_DEFINE(zeta_thread_nvs_id, ZETA_THREAD_NVS_STACK_SIZE, zeta_thread_nvs, NULL, NULL, NULL,
                ZETA_THREAD_PRIORITY, 0, K_NO_WAIT);
K_MSGQ_DEFINE(zeta_channels_changed_msgq, sizeof(u8_t), 30, 4);

$arrays_init

    static struct nvs_fs zeta_fs = {
        .sector_size  = NVS_SECTOR_SIZE,
        .sector_count = NVS_SECTOR_COUNT,
        .offset       = NVS_STORAGE_OFFSET,
};

$channels_creation

    const char *
    zeta_channel_name(zeta_channel_e id, int *error)
{
    if (id < ZETA_CHANNEL_COUNT) {
        zeta_channel_t *p = &__zeta_channels[id];
        if (error) {
            *error = 0;
        }
        return p->name;
    } else {
        LOG_INF("The channel #%d there isn't!", id);
        if (error) {
            *error = -EINVAL;
        }
        return NULL;
    }
}

size_t zeta_channel_size(zeta_channel_e id, int *error)
{
    if (id < ZETA_CHANNEL_COUNT) {
        zeta_channel_t *p = &__zeta_channels[id];
        if (error) {
            *error = 0;
        }
        return (size_t) p->size;
    } else {
        LOG_INF("The channel #%d there isn't!", id);
        if (error) {
            *error = -EINVAL;
        }
        return 0;
    }
}

int zeta_channel_get(zeta_channel_e id, u8_t *channel_value, size_t size)
{
    if (id < ZETA_CHANNEL_COUNT) {
        int error               = 0;
        zeta_channel_t *channel = &__zeta_channels[id];
        ZETA_CHECK_VAL(channel->get, NULL, -EPERM,
                       "channel #%d does not have get implementation!\n", id);
        ZETA_CHECK(size != channel->size, -EINVAL, "channel #%d has a different size!(%d)(%d)\n",
                   id, size, channel->size);
        if (channel->pre_get) {
            error = channel->pre_get(id, channel_value, size);
            ZETA_CHECK(error, error, "Error(code %d) in pre-get function of channel #%d\n", error,
                       id);
        }
        error = channel->get(id, channel_value, size);
        ZETA_CHECK(error, error, "Current channel #%d, error code: %d\n", id, error);
        if (channel->pos_get) {
            error = channel->pos_get(id, channel_value, size);
            ZETA_CHECK(error, error, "Error(code %d) in pos-get function of channel #%d!\n", error,
                       id);
        }
        return error;
    } else {
        LOG_INF("The channel #%d was not found!\n", id);
        return -ENODATA;
    }
}

static int zeta_channel_get_private(zeta_channel_e id, u8_t *channel_value, size_t size)
{
    int ret                 = 0;
    zeta_channel_t *channel = &__zeta_channels[id];
    if (k_sem_take(channel->sem, K_MSEC(200))) {
        LOG_INF("Could not get the channel. Channel is busy\n");
        ret = -EBUSY;
    } else {
        memcpy(channel_value, channel->data, channel->size);
        k_sem_give(channel->sem);
    }
    return ret;
}

int zeta_channel_set(zeta_channel_e id, u8_t *channel_value, size_t size)
{
    if (id < ZETA_CHANNEL_COUNT) {
        int error               = 0;
        int valid               = 1;
        zeta_channel_t *channel = &__zeta_channels[id];
        const k_tid_t *pub_id;

        for (pub_id = channel->publishers_id; *pub_id != NULL; ++pub_id) {
            if (*pub_id == k_current_get()) {
                break;
            }
        }
        ZETA_CHECK_VAL(*pub_id, NULL, -EPERM,
                       "The current thread has not the permission to change channel #%d!\n", id);
        ZETA_CHECK_VAL(channel->set, NULL, -EPERM, "The channel #%d is read only!\n", id);
        ZETA_CHECK(size != channel->size, -EINVAL, "The channel #%d has a different size!\n", id);
        if (channel->validate) {
            valid = channel->validate(channel_value, size);
        }
        ZETA_CHECK(!valid, -EINVAL, "The value doesn't satisfy valid function of channel #%d!\n",
                   id);
        if (channel->pre_set) {
            error = channel->pre_set(id, channel_value, size);
        }
        ZETA_CHECK(error, error, "Error on pre_set function of channel #%d!\n", id);
        error = channel->set(id, channel_value, size);
        ZETA_CHECK(error, error, "Current channel #%d, error code: %d!\n", id, error);
        if (channel->pos_set) {
            error = channel->pos_set(id, channel_value, size);
        }
        ZETA_CHECK(error, error, "Error on pos_set function of channel #%d!\n", id);
        return error;
    } else {
        LOG_INF("The channel #%d was not found!\n", id);
        return -ENODATA;
    }
}

static int zeta_channel_set_private(zeta_channel_e id, u8_t *channel_value, size_t size)
{
    int ret                 = 0;
    zeta_channel_t *channel = &__zeta_channels[id];
    if (k_sem_take(channel->sem, K_MSEC(200))) {
        LOG_INF("Could not set the channel. Channel is busy\n");
        ret = -EBUSY;
    } else {
        if (memcmp(channel->data, channel_value, size)) {
            memcpy(channel->data, channel_value, channel->size);
            channel->opt.field.pend_callback = 1;
            if (k_msgq_put(&zeta_channels_changed_msgq, (u8_t *) &id, K_MSEC(500))) {
                LOG_INF("[Channel #%d] Error sending channels change message to ZETA thread!\n",
                        id);
            }
            channel->opt.field.pend_persistent = (channel->persistent) ? 1 : 0;
            k_sem_give(channel->sem);
        } else {
            k_sem_give(channel->sem);
        }
    }
    return ret;
}

static void __zeta_recover_data_from_flash(void)
{
    int rc = 0;
    LOG_INF("[ ] Recovering data from flash\n");
    for (u16_t id = 0; id < ZETA_CHANNEL_COUNT; ++id) {
        if (__zeta_channels[id].persistent) {
            if (!k_sem_take(__zeta_channels[id].sem, K_SECONDS(5))) {
                rc = nvs_read(&zeta_fs, id, __zeta_channels[id].data, __zeta_channels[id].size);
                if (rc > 0) { /* item was found, show it */
                    LOG_INF("Id: %d", id);
                    LOG_HEXDUMP_INF(__zeta_channels[id].data, __zeta_channels[id].size, "Value: ");
                } else { /* item was not found, add it */
                    LOG_INF("No values found for channel #%d\n", id);
                }
                k_sem_give(__zeta_channels[id].sem);
            } else {
                LOG_INF("Could not recover the channel. Channel is busy\n");
            }
        }
    }
    LOG_INF("[X] Recovering data from flash\n");
}

static void __zeta_persist_data_on_flash(void)
{
    int bytes_written = 0;
    for (u16_t id = 0; id < ZETA_CHANNEL_COUNT; ++id) {
        if (__zeta_channels[id].persistent && __zeta_channels[id].opt.field.pend_persistent) {
            // LOG_INF("Store changes for channel #%d", id);
            bytes_written =
                nvs_write(&zeta_fs, id, __zeta_channels[id].data, __zeta_channels[id].size);
            if (bytes_written > 0) { /* item was found and updated*/
                __zeta_channels[id].opt.field.pend_persistent = 0;
                LOG_INF("channel #%d value updated on the flash\n", id);
            } else if (bytes_written == 0) {
                /* LOG_INF("channel #%d value is already on the flash.", id); */
            } else { /* item was not found, add it */
                LOG_INF("channel #%d could not be stored\n", id);
            }
        }
    }
}

void zeta_thread(void)
{
    $set_publishers

        u8_t id = 0;
    while (1) {
        k_msgq_get(&zeta_channels_changed_msgq, &id, K_FOREVER);
        if (id < ZETA_CHANNEL_COUNT) {
            if (__zeta_channels[id].opt.field.pend_callback) {
                for (zeta_callback_f *f = __zeta_channels[id].subscribers_cbs; *f != NULL; ++f) {
                    (*f)(id);
                }
                __zeta_channels[id].opt.field.pend_callback = 0;
            } else {
                LOG_INF(
                    "[ZETA-THREAD]: Received pend_callback from a channel(#%d) without changes!\n",
                    id);
            }
        } else {
            LOG_INF("[ZETA-THREAD]: Received an invalid ID channel #%d\n", id);
        }
    }
}

void zeta_thread_nvs(void)
{
    int error = nvs_init(&zeta_fs, DT_FLASH_DEV_NAME);
    if (error) {
        LOG_INF("Flash Init failed\n");
    } else {
        LOG_INF("NVS started...[OK]\n");
    }
    __zeta_recover_data_from_flash();

    while (1) {
        k_sleep(K_SECONDS(10));
        __zeta_persist_data_on_flash();
    }
}
