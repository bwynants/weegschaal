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
  const uint8_t kMaxUsers = 8;
  const uint8_t kSleepTimeBetweenScans = 30;
  const char *TAG = "BS4xxScale";

  MedisanaBS444::MedisanaBS444() : Component()
  {
    ESP_LOGE(TAG, "MedisanaBS444 constructed");
  }

  // we found a device
  void MedisanaBS444::onResult(BLEAdvertisedDevice &advertisedDevice)
  {
    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(mServiceUUID))
    {
      ESP_LOGD(TAG, "BLE Advertised Device found with our service%s", advertisedDevice.toString().c_str());
      BLEDevice::getScan()->stop(); // we found our device
      scaleDevice = new BLEAdvertisedDevice(advertisedDevice);
      mDoConnect = true;
      mDoScan = true;
    }
  }

  void MedisanaBS444::setup()
  {

    BLEDevice::init("");

    // Retrieve a Scanner and set the callback we want to use to be informed when we
    // have detected a new device.  Specify that we want active scanning and start the
    // scan to run for 5 seconds.
    BLEScan *pBLEScan = BLEDevice::getScan();

    // Register a callback function to be called when the device is discovered
    //  and send the device to our MedisanaBS444 object
    class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
    {
    public:
      MyAdvertisedDeviceCallbacks(MedisanaBS444 *scale) : mScale(scale){};
      MyAdvertisedDeviceCallbacks() = delete;
      /**
       * Called for each advertising BLE server.
       */
      void onResult(BLEAdvertisedDevice advertisedDevice)
      {
        mScale->onResult(advertisedDevice);
      } // onResult

    private:
      MedisanaBS444 *mScale;
    };

    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(this));
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(false);
    mDoConnect = false;
    mDoScan = true;
  }

  void MedisanaBS444::loop()
  {
    if (millis() > (bletime + 1000))
    {
      bletime = millis();
      // executes every second
      bleClient_loop();
    }
  }

  void MedisanaBS444::notifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
  {
    ESP_LOGD(TAG, "Notify callback for characteristic %s of data length %d data:", pBLERemoteCharacteristic->getUUID().toString().c_str(), pBLERemoteCharacteristic->getHandle(), length);
    if (pBLERemoteCharacteristic->getUUID().equals(Char_person))
    {
      mPerson = Person::decode(pData);
      ESP_LOGD(TAG, "data %s:", mPerson.toString().c_str());
    }
    else if (pBLERemoteCharacteristic->getUUID().equals(Char_weight))
    {
      auto data = Weight::decode(pData, use_timeoffset_);
      if (data.timestamp <= now())
      {
        ESP_LOGD(TAG, "data %s:", data.toString().c_str());
        if (!mWeight.valid || (mWeight < data))
          mWeight = data;
      }
    }
    else if (pBLERemoteCharacteristic->getUUID().equals(Char_body))
    {
      auto data = Body::decode(pData, use_timeoffset_);
      if (data.timestamp <= now())
      {
        ESP_LOGD(TAG, "data %s:", data.toString().c_str());
        if (!mBody.valid || (mBody < data))
          mBody = data;
      }
    }
  }

  void MedisanaBS444::onDisconnect(BLEClient *pclient)
  {
    ESP_LOGD(TAG, "onDisconnect");
    if (mPerson.valid)
    {
      // this is a measurement
      ESP_LOGI(TAG, "Person %s:", mPerson.toString().c_str());

      if ((mPerson.person >= 1) && (mPerson.person <= kMaxUsers))
      {
        uint8_t index = mPerson.person - 1;

        if (mWeight.valid && (mWeight.person == mPerson.person))
        {
          ESP_LOGI(TAG, "%s:", mWeight.toString(mPerson).c_str());
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
      else
        ESP_LOGE(TAG, "User not defined %d:", mPerson.person);
    }

    mConnected = false;
    mDoScan = true;
  }

  // Function that attempts to connect to the device, here is the KEY!!!
  bool MedisanaBS444::connectToServer()
  {
    // scaleDevice is a variable containing information about the device to be connected.
    // In the device scan below, information will be entered into the corresponding variable.
    ESP_LOGD(TAG, "Creating a connection to %s", scaleDevice->getAddress().toString().c_str());

    // Create a client (Central) class to connect to the server (Pheriphral)
    BLEClient *pClient = BLEDevice::createClient();
    ESP_LOGI(TAG, "Created client");

    class MyClientCallback : public BLEClientCallbacks
    {
    public:
      MyClientCallback(MedisanaBS444 *scale) : mScale(scale){};
      MyClientCallback() = delete;

      void onConnect(BLEClient *pclient)
      {
      }

      void onDisconnect(BLEClient *pclient)
      {
        mScale->onDisconnect(pclient);
      }

    private:
      MedisanaBS444 *mScale;
    };

    // Set up a callback function to receive connection status events
    pClient->setClientCallbacks(new MyClientCallback(this));

    // Finally tried to connect to the server (Pheriphral) device!!!
    pClient->connect(scaleDevice); // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    ESP_LOGI(TAG, "Connected to server");
    // set client to request maximum MTU from server (default is 23 otherwise)
    pClient->setMTU(517);

    // Receive service information when connected
    for (auto service : *pClient->getServices())
    {
      ESP_LOGD(TAG, "service:  %s, handle %d", service.first.c_str(), service.second->getHandle());
    }

    //
    // Obtain a reference to the service we are after in the remote BLE server.
    //
    BLERemoteService *pRemoteService = pClient->getService(mServiceUUID);
    if (pRemoteService == nullptr)
    {
      ESP_LOGE(TAG, "Failed to find our service UUID: %s", mServiceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    ESP_LOGD(TAG, "service %s found", mServiceUUID.toString().c_str());

    for (auto characteristic : *pRemoteService->getCharacteristics())
    {
      ESP_LOGD(TAG, "characteristic:  %s -> handle %d", characteristic.first.c_str(), characteristic.second->getHandle());
    }
    mPerson = Person();
    mBody = Body();
    mWeight = Weight();
    //
    // subscribe to characteristics
    //
    for (auto &characteristic : mCharacteristics)
    {
      auto pRemoteCharacteristic = pRemoteService->getCharacteristic(characteristic);
      if (pRemoteCharacteristic == nullptr)
      {
        ESP_LOGE(TAG, "Failed to find our characteristic UUID: %s", characteristic.toString().c_str());
        pClient->disconnect();
      }
      ESP_LOGD(TAG, "Found characteristic: Handle: %d, UUID: %s", pRemoteCharacteristic->getHandle(), characteristic.toString().c_str());

      // Read the value of the characteristic.
      if (pRemoteCharacteristic->canRead())
      {
        ESP_LOGD(TAG, "The characteristic canRead");
        std::string value = pRemoteCharacteristic->readValue();
        ESP_LOGD(TAG, "The characteristic value was: %s", value.c_str());
      }

      if (pRemoteCharacteristic->canNotify())
      {
        ESP_LOGD(TAG, "The characteristic canNotify");
        pRemoteCharacteristic->registerForNotify(std::bind(&MedisanaBS444::notifyCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
        ESP_LOGD(TAG, "Registered Callback");
      }

      if (pRemoteCharacteristic->canIndicate())
      {
        ESP_LOGD(TAG, "The characteristic canIndicate");
        pRemoteCharacteristic->registerForNotify(std::bind(&MedisanaBS444::notifyCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
        ESP_LOGD(TAG, "Registered Callback");

        for (auto descriptor : *pRemoteCharacteristic->getDescriptors())
        {
          ESP_LOGD(TAG, "descriptor:  %s, handle %d", descriptor.first.c_str(), descriptor.second->getHandle());
        }
        // do we find a descriptor with a handle of the pRemoteCharacteristic + 1 ?
        auto it = std::find_if(pRemoteCharacteristic->getDescriptors()->begin(), pRemoteCharacteristic->getDescriptors()->end(), [&pRemoteCharacteristic](std::map<std::string, BLERemoteDescriptor *>::value_type &descriptor)
                               { return (descriptor.second->getHandle() == pRemoteCharacteristic->getHandle() + 1); });
        if (it != pRemoteCharacteristic->getDescriptors()->end())
        {
          ESP_LOGD(TAG, "descriptor found:  %s, handle %d", it->first.c_str(), it->second->getHandle());
          const uint8_t indicationOn[] = {0x2, 0x0};
          it->second->writeValue((uint8_t *)indicationOn, 2, true);
          ESP_LOGD(TAG, "Indication Enabled");
        }
      }
    }
    auto pRemoteCharacteristic = pRemoteService->getCharacteristic(Char_command);
    if (pRemoteCharacteristic != nullptr)
    {
      ESP_LOGI(TAG, "Found characteristic: Handle: %d, UUID: %s", pRemoteCharacteristic->getHandle(), Char_command.toString().c_str());
      if (pRemoteCharacteristic->canWrite())
      {
        ESP_LOGD(TAG, "The characteristic canWrite");

        /*
        Send the unix timestamp in little endian order preceded by 02 as
        bytearray to Char_command. This will resync the scale's RTC.
        While waiting for a response notification, which will never
        arrive, the scale will emit 30 Indications on 0x1b (weight)
        and 0x1e (body) each.
        */
        uint8_t byteArray[5] = {2, 0, 0, 0, 0};
        convertTimestampToLittleEndian(now() - (use_timeoffset_ ? time_offset : 0), &byteArray[1]);
        pRemoteCharacteristic->writeValue((uint8_t *)byteArray, sizeof(byteArray), true);
      }
      else
      {
        ESP_LOGE(TAG, "Can not write to characteristic UUID: %s", Char_command.toString().c_str());
        pClient->disconnect();
      }
    }

    // If you have reached this point, set a variable to indicate that the connection was successful.
    mConnected = true;
    return true;
  }

  void MedisanaBS444::bleClient_loop()
  {
    // If the flag "mDoConnect" is true then we have scanned for and found the desired
    // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
    // connected we set the mConnected flag to be true.
    if (mDoConnect)
    {
      if (connectToServer())
        ESP_LOGI(TAG, "We are now connected to the BLE Server.");
      else
        ESP_LOGE(TAG, "We have failed to connect to the server; there is nothing more we will do.");
      mDoConnect = false;
    }

    // If we are connected to a peer BLE Server, update the characteristic each time we are reached
    // with the current time since boot.
    if (mConnected)
    {
      // we do nothing with the connection, we only wait for messages in the callback
    }
    else if (mDoScan)
    {
      ESP_LOGI(TAG, "starting scan in 30 seconds.");
      // start a next scan in 30 time
      mDoScan = false;
      mScan = true;

      bletime = millis() + kSleepTimeBetweenScans * 1000; // delay next itertation
    }
    else if (mScan)
    {
      mScan = false;
      mDoScan = true; // restarting a delayed scan whenever possible
                      // If the connection is released and mDoScan is true, start scanning
      ESP_LOGI(TAG, "Restarting scan.");
      BLEDevice::getScan()->start(1, false);
    }
  }

  void MedisanaBS444::dump_config()
  {
    ESP_LOGCONFIG(TAG, "MedisanaBS444:");
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

} // namespace medisana_bs444
} // namespace esphome

#endif // USE_ESP32
