/*
 * main.c
 */

#include <inttypes.h>

#include <stdio.h>
#include <string.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/settings/settings.h>
#include <zephyr/sys/printk.h>

#include "BTN.h"
#include "LED.h"

#define SLEEP_MS 1

#define BLE_CUSTOM_SERVICE_UUID BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef0)
#define BLE_CUSTOM_CHARACTERISTIC_UUID BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef1)
#define BLE_CUSTOM_CHARACTERISTIC_UUID_2 BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef2)

#define BLE_CUSTOM_CHARACTERISTIC_MAX_DATA_LENGTH 20

static const struct bt_data ble_advertising_data[] = {
  BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
  BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME)),
};

static uint8_t ble_custom_characteristic_user_data[20] = {};
static uint8_t ble_custom_characteristic_user_data_2[20] = {};

static ssize_t ble_custom_characteristic_read_cb(struct bt_conn* conn, const struct bt_gatt_attr* attr,
                                                 void* buf, uint16_t len, uint16_t offset) {
  const char* value = attr->user_data;
  return bt_gatt_attr_read(conn, attr, buf, len, offset, value, strlen(value));
}

static ssize_t ble_custom_characteristic_write_cb(struct bt_conn* conn, const struct bt_gatt_attr* attr,
                                                  const void* buf, uint16_t len, uint16_t offset,
                                                  uint8_t flags) {
  uint8_t* value_ptr = attr->user_data;

  if (offset + len > BLE_CUSTOM_CHARACTERISTIC_MAX_DATA_LENGTH) {
    return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
  }

  memcpy(value_ptr + offset, buf, len);
  value_ptr[offset + len] = 0;

  return len;
}

static const struct bt_uuid_128 ble_custom_service_uuid = BT_UUID_INIT_128(BLE_CUSTOM_SERVICE_UUID);
static const struct bt_uuid_128 ble_custom_characteristic_uuid = BT_UUID_INIT_128(BLE_CUSTOM_CHARACTERISTIC_UUID);
static const struct bt_uuid_128 ble_custom_characteristic_uuid_2 = BT_UUID_INIT_128(BLE_CUSTOM_CHARACTERISTIC_UUID_2);

BT_GATT_SERVICE_DEFINE(
    ble_custom_service,  // Name of the struct that will store the config for this service
    BT_GATT_PRIMARY_SERVICE(&ble_custom_service_uuid),  // Setting the service UUID

    BT_GATT_CHARACTERISTIC(
        &ble_custom_characteristic_uuid.uuid,  // Setting the characteristic UUID
        BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,  // Possible operations
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,  // Permissions that connecting devices have
        ble_custom_characteristic_read_cb,     // Callback for when this characteristic is read from
        ble_custom_characteristic_write_cb,    // Callback for when this characteristic is written to
        ble_custom_characteristic_user_data    // Initial data stored in this characteristic
        ),
    BT_GATT_CHARACTERISTIC(
        &ble_custom_characteristic_uuid_2.uuid,  // Setting the characteristic UUID
        BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,  // Possible operations
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,  // Permissions that connecting devices have
        ble_custom_characteristic_read_cb,     // Callback for when this characteristic is read from
        ble_custom_characteristic_write_cb,    // Callback for when this characteristic is written to
        ble_custom_characteristic_user_data_2    // Initial data stored in this characteristic
        ),
);



int main(void) {
  int err = bt_enable(NULL);
  if (err) {
    printk("%d", err);
    return 0;
  }

  err = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, ble_advertising_data, ARRAY_SIZE(ble_advertising_data), NULL , 0);
  if (err) {
    printk("%d", err);
    return 0;
  }

  if (0 > BTN_init()) {
    return 0;
  }
  if (0 > LED_init()) {
    return 0;
  }

  while(1) {
    k_msleep(SLEEP_MS);
  }
	return 0;
}
