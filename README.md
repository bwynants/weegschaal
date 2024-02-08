# weegschaal

Use Medisana scale with ESPHome in HomeAssistant

We should not use "ESP32 BLE Arduino" as it does not scan asyncrounous and uses a lot of memory, but is seems like ble_client is not working for BS430 and this version is....

## Use Medisana BS444 scale with ESPHome on ESP32 in Home Assistant

### how to setup

#### add the code to the yaml

add a reference to the code on github

    external_components:
      - source:
          type: git
          url: https://github.com/bwynants/weegschaal
          ref: ESP-BLE-Arduino
        components: [ medisana_bs444 ]

or local on your esphome directory

    external_components:
      - source: 
          type: local
          path: components

#### define the component

	medisana_bs444:
	    timeoffset: false # BS410 and BS444 needs timeoffset, set to false for other scales

#### define de sensors

    sensor:
      - platform: medisana_bs444
        weight_1:
          name: "Weight User 1"
        kcal_1:
          name: "kcal User 1"
        fat_1:
          name: "Fat User 1"
        tbw_1:
          name: "Water User 1"
        muscle_1:
          name: "Muscle User 1"
        bone_1:
          name: "Bone User 1"
        bmi_1:
          name: "BMI User 1"

        weight_2:
          name: "Weight User 2"
        kcal_2:
          name: "kcal User 2"
        fat_2:
          name: "Fat User 2"
        tbw_2:
          name: "Water User 2"
        muscle_2:
          name: "Muscle User 2"
        bone_2:
          name: "Bone User 2"
        bmi_2:
          name: "BMI User 2"

up till 8 users can be specified....

#### add homeassistant time

    time:
      - platform: homeassistant
        id: homeassistant_time

#### define, an optional, switch to disable the scanning

    switch:
      - platform: medisana_bs444
        scan: 
          restore_mode: RESTORE_DEFAULT_ON
          name: "Scan"

## support

propably more medisana scales work BS410/BS430/BS440, but I only have the BS444 to test

## credits
   
based on reverse engineering from https://github.com/keptenkurk/BS440
