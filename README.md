# 🏋️ Medisana BS444 Scale Integration with ESPHome on ESP32  
This component integrates the **Medisana BS444 Bluetooth scale** directly with **ESPHome**, so you can track weight, body composition, and more in **Home Assistant**.  

👉 BS430 users should check: [ESP32-BLE-Arduino branch](https://github.com/bwynants/weegschaal/tree/ESP32-BLE-Arduino)

## 🔧 Setup Guide  

### 1. Add External Component  
#### Option A: Install from GitHub  
```yaml
external_components:
  - source:
      type: git
      url: https://github.com/bwynants/weegschaal
      ref: main
    components: [ medisana_bs444 ]

#### Option B: From Local Directory

```yaml
external_components:
  - source: 
      type: local
      path: components
```

### 2. Enable BLE Tracker

```yaml
esp32_ble_tracker:
  scan_parameters:
    interval: 1100ms
    window: 1100ms
    active: false
```

### 3. Set Up BLE Client

Replace with your scale’s MAC address:


```yaml
ble_client:
  - mac_address: "00:00:00:00:00:00"
    id: medisanabs44_ble_id
```

### 4. Configure the Scale

Supports multiple scales (just add more MAC addresses).
Up to 8 users can be defined.
For BS410 & BS444 you need timeoffset: true.

#### Core Device

## 📦 Component Configuration

```yaml
medisana_bs444:
  - id: myscale
    timeoffset: true 
```

### Sensors

```yaml
sensor:
  - platform: medisana_bs444
    medisana_bs444_id: myscale
      - platform: medisana_bs444
        ble_client_id: medisanabs44_ble_id
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
        age_1:
          name: "Age user 1"
        size_1:
          name: "Size user 1"

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
        age_2:
          name: "Age user 2"
        size_2:
          name: "Size user 2"
```

### Binary Sensors

Track gender and activity level

```yaml
binary_sensor:
  - platform: medisana_bs444
    medisana_bs444_id: myscale

    male_1:
      name: "Male user 1"
    female_1:
      name: "Female user 1"
    highactivity_1:
      name: "High Activity user 1"
    
    male_2:
      name: "Male user 2"
    female_2:
      name: "Female user 2"
    highactivity_2:
      name: "High Activity user 2"
```


## add homeassistant time

```yaml
time:
  - platform: homeassistant
    id: homeassistant_time
```

## support

Confirmed: BS444
Likely compatible: BS410 / BS430 / BS440
(only BS444 was tested)

## credits
based on reverse engineering from https://github.com/keptenkurk/BS440
