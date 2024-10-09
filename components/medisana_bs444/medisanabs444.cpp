#include "medisanabs444.h"
#ifdef USE_ESP32

namespace esphome
{
namespace medisana_bs444
{

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

  /*
   * The numers in use on the scale
   */
  static const char *TAG = "MedisanaBS444";

  MedisanaBS444::MedisanaBS444() : Component()
  {
    ESP_LOGE(TAG, "MedisanaBS444 constructed");
  }

  void MedisanaBS444::dump_config()
  {
    ESP_LOGCONFIG(TAG, "MedisanaBS444:");
    ESP_LOGCONFIG(TAG, "  MAC address        : %s", this->parent()->address_str().c_str());
    for (uint8_t i = 0; i < 8; i++)
    {
      ESP_LOGCONFIG(TAG, "User_%d:", i);
      if (this->weight_sensor_[i])
        LOG_SENSOR(TAG, " weight", this->weight_sensor_[i]);
      if (this->bmi_sensor_[i])
        LOG_SENSOR(TAG, " BMI", this->bmi_sensor_[i]);
      if (this->kcal_sensor_[i])
        LOG_SENSOR(TAG, " kcal", this->kcal_sensor_[i]);
      if (this->fat_sensor_[i])
        LOG_SENSOR(TAG, " fat", this->fat_sensor_[i]);
      if (this->tbw_sensor_[i])
        LOG_SENSOR(TAG, " tbw", this->tbw_sensor_[i]);
      if (this->muscle_sensor_[i])
        LOG_SENSOR(TAG, " muscle", this->muscle_sensor_[i]);
      if (this->bone_sensor_[i])
        LOG_SENSOR(TAG, " bone", this->bone_sensor_[i]);
    }
  }

#ifdef USE_TIME
  void MedisanaBS444::set_time_id(time::RealTimeClock *time_id)
  {
    ESPTime now = time_id->now();
    ESP_LOGI(TAG, "setting time %ld!", time_id->now().timestamp);
    this->time_id_ = time_id;
  }
#endif

  time_t MedisanaBS444::now()
  {
#ifdef USE_TIME
    if (this->time_id_.has_value())
    {
      auto *time_id = *this->time_id_;
      ESPTime now = time_id->now();
      return time_id->now().timestamp;
    }
    else
#endif
    {
      ESP_LOGI(TAG, "Time unknown!");
      return millis() / 1000; // some stupid value.....
    }
  }

  void MedisanaBS444::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                          esp_ble_gattc_cb_param_t *param)
  {
    switch (event)
    {
    case ESP_GATTC_OPEN_EVT:
    {
      ESP_LOGD(TAG, "ESP_GATTC_OPEN_EVT!");
      if (param->open.status == ESP_GATT_OK)
      {
        ESP_LOGI(TAG, "Connected successfully!");
      }
      break;
    }

    case ESP_GATTC_DISCONNECT_EVT:
    {
      ESP_LOGD(TAG, "ESP_GATTC_DISCONNECT_EVT!");
      this->node_state = esp32_ble_tracker::ClientState::IDLE;
      if (mPerson.valid)
      {
        // this is a measurement
        ESP_LOGI(TAG, "Person %s:", mPerson.toString().c_str());
        if ((mPerson.person >= 1) && (mPerson.person <= 8))
        {
          uint8_t index = mPerson.person - 1;

          if (mWeight.valid && (mWeight.person == mPerson.person))
          {
            ESP_LOGI(TAG, "Weight %s:", mWeight.toString(mPerson).c_str());
            if (this->weight_sensor_[index])
              this->weight_sensor_[index]->publish_state(mWeight.weight);
            if (this->bmi_sensor_[index] && mPerson.size)
              this->bmi_sensor_[index]->publish_state(mWeight.weight / (mPerson.size * mPerson.size));
          }
          if (mBody.valid && (mBody.person == mPerson.person))
          {
            ESP_LOGI(TAG, "Body %s:", mBody.toString().c_str());
            if (this->kcal_sensor_[index])
              this->kcal_sensor_[index]->publish_state(mBody.kcal);
            if (this->fat_sensor_[index])
              this->fat_sensor_[index]->publish_state(mBody.fat);
            if (this->tbw_sensor_[index])
              this->tbw_sensor_[index]->publish_state(mBody.tbw);
            if (this->muscle_sensor_[index])
              this->muscle_sensor_[index]->publish_state(mBody.muscle);
            if (this->bone_sensor_[index])
              this->bone_sensor_[index]->publish_state(mBody.bone);
          }
        }
      }

      break;
    }

    case ESP_GATTC_SEARCH_CMPL_EVT:
    {
      ESP_LOGD(TAG, "ESP_GATTC_SEARCH_CMPL_EVT!");
      // reset
      mPerson = Person();
      mBody = Body();
      mWeight = Weight();
      registered_notifications_ = 0;
      for (const auto &characteristic: mCharacteristics)
      {
        auto *chr = this->parent()->get_characteristic(mServiceUUID, characteristic);
        if (chr == nullptr)
        {
          ESP_LOGE(TAG, "No sensor read characteristic found at service %s char %s", mServiceUUID.to_string().c_str(),
                   characteristic.to_string().c_str());
          break;
        }

        auto status_notify = esp_ble_gattc_register_for_notify(this->parent()->get_gattc_if(), this->parent()->get_remote_bda(), chr->handle);
        if (status_notify)
        {
          ESP_LOGE(TAG, "esp_ble_gattc_register_for_notify failed, status=%d", status_notify);
        }
        else
        {
          mCharacteristicHandles[registered_notifications_] = chr->handle;
          registered_notifications_++;
        }
      }

      ESP_LOGD(TAG, "All characteristic found at service %s", mServiceUUID.to_string().c_str());
      break;
    }

    case ESP_GATTC_READ_CHAR_EVT:
    {
      ESP_LOGD(TAG, "ESP_GATTC_READ_CHAR_EVT!");
      if (param->read.conn_id != this->parent()->get_conn_id())
        break;
      if (param->read.status != ESP_GATT_OK)
      {
        ESP_LOGW(TAG, "Error reading char at handle %d, status=%d", param->read.handle, param->read.status);
        break;
      }
      break;
    }

    case ESP_GATTC_REG_FOR_NOTIFY_EVT:
    {
      ESP_LOGD(TAG, "ESP_GATTC_REG_FOR_NOTIFY_EVT!");
      if (--registered_notifications_ == 0)
      { 
        // all notify requests are handled
        this->node_state = esp32_ble_tracker::ClientState::ESTABLISHED;

        const uint8_t indicationOn[] = {0x2, 0x0};
        //for (uint8_t i = 0; i < 3; i++)
        for(const auto&handle :mCharacteristicHandles)
        {
          // send indicate for these handles
          auto status = esp_ble_gattc_write_char_descr(this->parent()->get_gattc_if(), this->parent()->get_conn_id(),
                                                       handle + 1, sizeof(indicationOn), (uint8_t *)indicationOn,
                                                       ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
          if (status)
          {
            ESP_LOGE(TAG, "Error sending write request for characteristic handle, status=%d", status);
          }
          else
          {
            ESP_LOGD(TAG, "notification on for characteristic handle 0x%x", handle);
          }
        }

        auto *write_chr = this->parent()->get_characteristic(mServiceUUID, Char_command);
        if (write_chr == nullptr)
        {
          ESP_LOGE(TAG, "No write characteristic found at service %s char %s", mServiceUUID.to_string().c_str(),
                   Char_command.to_string().c_str());
          break;
        }

        uint8_t byteArray[5] = {2, 0, 0, 0, 0};
        convertTimestampToLittleEndian(now() - (use_timeoffset_ ? time_offset : 0), &byteArray[1]);

        auto status = esp_ble_gattc_write_char_descr(this->parent()->get_gattc_if(), this->parent()->get_conn_id(),
                                                     write_chr->handle, sizeof(byteArray), (uint8_t *)byteArray,
                                                     ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
        if (status)
        {
          ESP_LOGE(TAG, "Error sending datetimestap, status=%d", status);
        }
        ESP_LOGD(TAG, "request to send data sent");
      }
      break;
    }

    case ESP_GATTC_NOTIFY_EVT:
    {
      ESP_LOGD(TAG, "ESP_GATTC_NOTIFY_EVT! 0x%x, %d", param->notify.handle, param->notify.value_len);
      if (mCharacteristicHandles[0] == param->notify.handle)
      {
        mPerson = Person::decode(param->notify.value);
        ESP_LOGD(TAG, "data %s:", mPerson.toString().c_str());
      }
      else if (mCharacteristicHandles[1] == param->notify.handle)
      {
        auto data = Weight::decode(param->notify.value, use_timeoffset_);
        if (data.timestamp <= now())
        {
          ESP_LOGD(TAG, "data %s:", data.toString().c_str());
          if (!mWeight.valid || (mWeight < data))
            mWeight = data;
        }
        else
        {
          ESP_LOGD(TAG, "Skipped future event!");
        }
      }
      else if (mCharacteristicHandles[2] == param->notify.handle)
      {
        auto data = Body::decode(param->notify.value, use_timeoffset_);
        if (data.timestamp <= now())
        {
          ESP_LOGD(TAG, "data %s:", data.toString().c_str());
          if (!mBody.valid || (mBody < data))
            mBody = data;
        }
        else
        {
          ESP_LOGD(TAG, "Skipped future event!");
        }
      }
      break;
    }

    default:
      break;
    }
  }

} // namespace medisana_bs444
} // namespace esphome

#endif // USE_ESP32
