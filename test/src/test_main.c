#include <string.h>
#include <zephyr.h>
#include <ztest.h>

#include "autoconf.h"
#include "pdb.h"


void test_channels_name(void)
{
    int error        = 0;
    const char *name = NULL;

    /* Checking an invalid call */
    name = pdb_channel_name(PDB_CHANNEL_COUNT, &error);
    zassert_equal(error, -EINVAL, "[%s] pdb_channel_name is allowing an invalid channel id!\n",
                  __FUNCTION__);
    zassert_equal(name, NULL,
                  "[%s] pdb_channel_name is not returning NULL pointer in an invalid call!\n",
                  __FUNCTION__);
    /* Checking FIRMWARE_VERSION call */
    error = 1;
    name  = pdb_channel_name(PDB_FIRMWARE_VERSION_CHANNEL, &error);
    zassert_equal(error, 0, "[%s] pdb_channel_name is not setting error to 0 in a valid call!\n",
                  __FUNCTION__);
    zassert_true(!strcmp(name, "FIRMWARE_VERSION"),
                 "[%s] channel #%d was created with a wrong name: %s\n", __FUNCTION__,
                 PDB_FIRMWARE_VERSION_CHANNEL, name);

    /* Checking COUNT_REACH_LIMIT call */
    error = 1;
    name  = pdb_channel_name(PDB_COUNT_REACH_LIMIT_CHANNEL, &error);
    zassert_equal(error, 0, "[%s] pdb_channel_name is not setting error to 0 in a valid call!\n",
                  __FUNCTION__);
    zassert_true(!strcmp(name, "COUNT_REACH_LIMIT"),
                 "[%s] channel #%d was created with a wrong name: %s\n", __FUNCTION__,
                 PDB_COUNT_REACH_LIMIT_CHANNEL, name);

    /* Checking PMS_SENSOR_VAL call */
    error = 1;
    name  = pdb_channel_name(PDB_PMS_SENSOR_VAL_CHANNEL, &error);
    zassert_equal(error, 0, "[%s] pdb_channel_name is not setting error to 0 in a valid call!\n",
                  __FUNCTION__);
    zassert_true(!strcmp(name, "PMS_SENSOR_VAL"),
                 "[%s] channel #%d was created with a wrong name: %s\n", __FUNCTION__,
                 PDB_PMS_SENSOR_VAL_CHANNEL, name);

    /* Checking POWER_VAL call */
    error = 1;
    name  = pdb_channel_name(PDB_POWER_VAL_CHANNEL, &error);
    zassert_equal(error, 0, "[%s] pdb_channel_name is not setting error to 0 in a valid call!\n",
                  __FUNCTION__);
    zassert_true(!strcmp(name, "POWER_VAL"), "[%s] channel #%d was created with a wrong name: %s\n",
                 __FUNCTION__, PDB_POWER_VAL_CHANNEL, name);
}

void test_properties_set(void)
{
    /* u32_t new_firmware_version = 0xABCDED0F; */
    /* u8_t value                 = 0; */
    /* int error                  = 0; */

    /* /\* Testing if set functions was correct assigned *\/ */
    /* error = pdb_property_set(PDB_FIRMWARE_VERSION_PROPERTY, PDB_VALUE_REF(new_firmware_version));
     */
    /* zassert_equal(error, -ENODEV, */
    /*               "FIRMWARE_VERSION property was defined with a non null set pointer!"); */

    /* error = pdb_property_set(PDB_PERSISTENT_VAL_PROPERTY, PDB_VALUE_REF(value)); */
    /* zassert_equal(error, -EINVAL, */
    /*               "PERSISTENT_VAL property is allowing set a value zero to property: %d!",
     * error); */
    /* value = 0xF1; */
    /* error = pdb_property_set(PDB_PERSISTENT_VAL_PROPERTY, PDB_VALUE_REF(value)); */
    /* zassert_equal(error, 0, "PERSISTENT_VAL property is not setting a valid value(%X), error:
     * %d", */
    /*               value, error); */

    /* error = pdb_property_set(PDB_SET_GET_PROPERTY, PDB_VALUE_REF(value)); */
    /* zassert_equal(error, 0, "SET_GET property is not setting a valid value(%X), error: %d",
     * value, */
    /*               error); */
    /* error = pdb_property_set(PDB_ECHO_HAL_PROPERTY, PDB_VALUE_REF(value)); */
    /* zassert_equal(error, 0, "ECHO_HAL property is not setting a valid value(%X), error: %d",
     * value, */
    /*               error); */
}

void test_properties_get(void)
{
    /* u8_t firmware_version[4] = {0}; */
    /* u8_t value               = 0; */
    /* int error                = 0; */

    /* error = */
    /*     pdb_property_get(PDB_FIRMWARE_VERSION_PROPERTY, firmware_version,
     * sizeof(firmware_version)); */
    /* zassert_false(error, */
    /*               "PDB is not allowing get value from property FIRMWARE_VERSION, error code:
     * %d!", */
    /*               error); */
    /* zassert_equal(0xF1, firmware_version[0], */
    /*               "FIRMWARE_VERSION property must to has 0xF1 as the value of the first byte, "
     */
    /*               "value found %02X!", */
    /*               firmware_version[0]); */
    /* zassert_equal(0xF2, firmware_version[1], */
    /*               "FIRMWARE_VERSION property must to has 0xF2 as the value of the second byte, "
     */
    /*               "value found %02X!", */
    /*               firmware_version[1]); */
    /* zassert_equal(0xF3, firmware_version[2], */
    /*               "FIRMWARE_VERSION property must to has 0xF3 as the value of the third byte, "
     */
    /*               "value found %02X!", */
    /*               firmware_version[2]); */
    /* zassert_equal(0xF4, firmware_version[3], */
    /*               "FIRMWARE_VERSION property must to has 0xF4 as the value of the fourth byte, "
     */
    /*               "value found %02X!", */
    /*               firmware_version[3]); */

    /* error = pdb_property_get(PDB_PERSISTENT_VAL_PROPERTY, PDB_VALUE_REF(value)); */
    /* zassert_equal(0xF1, value, */
    /*               "PERSISTENT_VAL property must to has 0xF1 as the value, value found %02X!", */
    /*               value); */

    /* error = pdb_property_get(PDB_SET_GET_PROPERTY, PDB_VALUE_REF(value)); */
    /* zassert_equal(0xF3, value, "SET_GET property must to has 0xF3 as the value, value found
     * %02X!", */
    /*               value); */
}

void test_channels_size(void)
{
    size_t sz = 0;
    int error = 0;

    /* Checking an invalid call */
    sz = pdb_channel_size(PDB_CHANNEL_COUNT, &error);
    zassert_equal(error, -EINVAL, "[%s] pdb_channel_size is not setting error to -EINVAL!\n",
                  __FUNCTION__);
    zassert_equal(sz, 0, "[%s] pdb_channel_size is not returning 0 in an invalid call!\n",
                  __FUNCTION__);

    /* Checking FIRMWARE_VERSION size*/
    sz = pdb_channel_size(PDB_FIRMWARE_VERSION_CHANNEL, &error);
    zassert_equal(error, 0, "[%s] pdb_channel_size is setting error to %d in a valid call!\n",
                  __FUNCTION__, error);
    zassert_equal(sz, 4, "[%s] pdb_channel_size returned a wrong size value: %d\n", __FUNCTION__,
                  sz);

    /* Checking COUNT_REACH_LIMIT size*/
    sz = pdb_channel_size(PDB_COUNT_REACH_LIMIT_CHANNEL, &error);
    zassert_equal(error, 0, "[%s] pdb_channel_size is setting error to %d in a valid call!\n",
                  __FUNCTION__, error);
    zassert_equal(sz, 1, "[%s] pdb_channel_size returned a wrong size value: %d\n", __FUNCTION__,
                  sz);

    /* Checking POWER_VAL size*/
    sz = pdb_channel_size(PDB_POWER_VAL_CHANNEL, &error);
    zassert_equal(error, 0, "[%s] pdb_channel_size is setting error to %d in a valid call!\n",
                  __FUNCTION__, error);
    zassert_equal(sz, 2, "[%s] pdb_channel_size returned a wrong size value: %d\n", __FUNCTION__,
                  sz);

    /* Checking PMS_SENSOR_VAL size*/
    sz = pdb_channel_size(PDB_PMS_SENSOR_VAL_CHANNEL, &error);
    zassert_equal(error, 0, "[%s] pdb_channel_size is setting error to %d in a valid call!\n",
                  __FUNCTION__, error);
    zassert_equal(sz, 1, "[%s] pdb_channel_size returned a wrong size value: %d\n", __FUNCTION__,
                  sz);
}

int pre_get_pms_wakeup(pdb_channel_e id, u8_t *channel_value, size_t size)
{
    return 0;
}

int pos_get_pms_sleep(pdb_channel_e id, u8_t *channel_value, size_t size)
{
    return 0;
}

void test_channels_get(void)
{
    u8_t data = 0;
    pdb_channel_get(PDB_PMS_SENSOR_VAL_CHANNEL, &data, sizeof(data));
}

void test_main(void)
{
    ztest_test_suite(PDB_CHECK_CHANNELS_GENERATION, ztest_unit_test(test_channels_name),
                     ztest_unit_test(test_channels_size), ztest_unit_test(test_channels_get));

    ztest_run_test_suite(PDB_CHECK_CHANNELS_GENERATION);
}
