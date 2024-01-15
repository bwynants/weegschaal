# weegschaal

Use Medisana scale with ESPHome in HomeAssistant

We should not use "ESP32 BLE Arduino" as it does not scan asyncrounous and uses a lot of memory, but is seems like ble_client is not working for BS430 and this version is....

Have a look in the "weegschaal.yaml" for how to uses it. It demonstrate uage for 2 people, up till 8 users can be specified

make sure you copy lib/scale in your esphome directory

