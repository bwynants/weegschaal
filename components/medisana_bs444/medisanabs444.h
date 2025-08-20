#pragma once

#ifdef USE_ESP32

#include "esphome/core/component.h"

#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"

#ifdef USE_TIME
#include "esphome/components/time/real_time_clock.h"
#include "esphome/core/time.h"
#endif

#include "Scale.h"

/******************************* BS444 Scale *******************************************/
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 *
 * The scale submits relevant data by use of Indications.
 * Indications are messages conveying the data for certain characteristics.
 * Characteristics have a two-byte shortcut for it (the handle) an a 16 byte
 * (globally unique) identifier (the UUID). *

 * Relevant characteristics:
 * Description Handle      UUID                                  Data
 *             (2 byte)    (16 byte, globally unique)
 * Person      0x26        00008a82-0000-1000-8000-00805f9b34fb  person, gender, age, size, activity
 * Weight      0x1c        00008a21-0000-1000-8000-00805f9b34fb  weight, time, person
 * Body        0x1f        00008a22-0000-1000-8000-00805f9b34fb  time, person, kcal, fat, tbw, muscle, bone *
 *
 * Writing the command 0200 to a handle tells the device that you register to the
 * indications of this characteristic. Writing 0000 will stop it. *
 *
 * A data packet of a characteristic (hex data string) will report with a
 * handle 1 less than the handle of the characteristic. E.g. writing 0200
 * to 0x26 (Person) will report back with the handle 0x25.*
 *
 * The last 30 measurements per person will be stored in the scale and upon
 * communication, the history for this user will be dumped. So you will
 * receive 30 values like this:
 * handle=0x25, value=0x845302800134b6e0000000000000000000000000
 * handle=0x1b, value=0x1d8c1e00fe6e0aa056451100ff020900000000
 * handle=0x1e, value=0x6f6e0aa05602440ab8f07ff26bf11ef0000000
 *
 */

namespace esphome
{
  namespace medisana_bs444
  {
    class MedisanaBS444 : public Component, public esphome::ble_client::BLEClientNode
    {

    private:
      // The service(es) we are interested in
      esp32_ble::ESPBTUUID mServiceUUID = Serv_SCALE;
      // The characteristic of the remote service we are interested in.
      esp32_ble::ESPBTUUID mCharacteristics[3] = {Char_person, Char_weight, Char_body};
      uint16_t mCharacteristicHandles[3] = {0, 0, 0};
      // last read values
      Person mPerson;
      Weight mWeight;
      Body mBody;

    public:
      MedisanaBS444() = default;

      void dump_config() override;

    protected:
      time_t now();

      void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);

    public:
      void set_weight(uint8_t i, sensor::Sensor *sensor) { weight_sensor_[i] = sensor; }
      void set_bmi(uint8_t i, sensor::Sensor *sensor) { bmi_sensor_[i] = sensor; }
      void set_kcal(uint8_t i, sensor::Sensor *sensor) { kcal_sensor_[i] = sensor; }
      void set_fat(uint8_t i, sensor::Sensor *sensor) { fat_sensor_[i] = sensor; }
      void set_tbw(uint8_t i, sensor::Sensor *sensor) { tbw_sensor_[i] = sensor; }
      void set_muscle(uint8_t i, sensor::Sensor *sensor) { muscle_sensor_[i] = sensor; }
      void set_bone(uint8_t i, sensor::Sensor *sensor) { bone_sensor_[i] = sensor; }
      void set_age(uint8_t i, sensor::Sensor *sensor) { age_sensor_[i] = sensor; }
      void set_size(uint8_t i, sensor::Sensor *sensor) { size_sensor_[i] = sensor; }
      void set_male(uint8_t i, binary_sensor::BinarySensor *sensor) { male_sensor_[i] = sensor; }
      void set_female(uint8_t i, binary_sensor::BinarySensor *sensor) { female_sensor_[i] = sensor; }
      void set_high_activity(uint8_t i, binary_sensor::BinarySensor *sensor) { high_activity_sensor_[i] = sensor; }

    protected:
      sensor::Sensor *weight_sensor_[8]{nullptr};
      sensor::Sensor *bmi_sensor_[8]{nullptr};
      sensor::Sensor *kcal_sensor_[8]{nullptr};
      sensor::Sensor *fat_sensor_[8]{nullptr};
      sensor::Sensor *tbw_sensor_[8]{nullptr};
      sensor::Sensor *muscle_sensor_[8]{nullptr};
      sensor::Sensor *bone_sensor_[8]{nullptr};
      sensor::Sensor *age_sensor_[8]{nullptr};
      sensor::Sensor *size_sensor_[8]{nullptr};
      binary_sensor::BinarySensor *male_sensor_[8]{nullptr};
      binary_sensor::BinarySensor *female_sensor_[8]{nullptr};
      binary_sensor::BinarySensor *high_activity_sensor_[8]{nullptr};

    public:
      void use_timeoffset(bool use_timeoffset) { use_timeoffset_ = use_timeoffset; }

    protected:
      bool use_timeoffset_ = false;

#ifdef USE_TIME
    public:
      void set_time_id(time::RealTimeClock *time_id);

    protected:
      time::RealTimeClock *time_id_ = nullptr;
#endif

    private:
      u_int32_t registered_notifications_ = 0;
    };
  } // namespace medisana_bs444
} // namespace esphome
#endif // USE_ESP32
