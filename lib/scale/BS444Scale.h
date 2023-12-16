#include "esphome.h"
#include "BLEDevice.h"
#include "Scale.h"
#include <vector>

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
static const uint8_t kMaxUsers = 2;
static const uint8_t kSleepTimeBetweenScans = 30;


// https://www.leef.nl/kennisbank/weegschalen-en-vetmeters

class BS444Scale : public Component {

  public:
    std::vector<Sensor*> sensor_weights;
    std::vector<Sensor*> sensor_kcal; //                           [0..65025 Kcal] 
    std::vector<Sensor*> sensor_fat; // percentage of body fat     [0..100 %]
    std::vector<Sensor*> sensor_tbw; // percentage of water        [0..100 %]
    std::vector<Sensor*> sensor_muscle; // percentage of muscle    [0..100 %]
    std::vector<Sensor*> sensor_bone; // percentage of bone weight [0..100 %]
    std::vector<Sensor*> sensor_bmi; // BMI

    // Connection and device discovery state variables
    boolean mConnected = false; // we have a valid connection
    boolean mDoConnect = false; // we will try to connect
    boolean mDoScan = false; // will schedule a new scan in 30 seconds
    boolean mScan = false; // will do a new scan

    // The service(es) we are interested in 
    BLEUUID mServiceUUID = Serv_SCALE;
    // The characteristic of the remote service we are interested in.
    BLEUUID mCharacteristics[3] = {Char_person, Char_weight, Char_body};

    BLEAdvertisedDevice *scaleDevice;

    // last read values
    Person mPerson;
    Weight mWeight;
    Body mBody;
    
    // timer
    uint32_t bletime  = millis() + 1000;

  private:
      BS444Scale() {

          for (uint8_t i = 0; i < kMaxUsers; i++){
            this->sensor_weights.push_back(new Sensor());
            this->sensor_kcal.push_back(new Sensor());
            this->sensor_fat.push_back(new Sensor());
            this->sensor_tbw.push_back(new Sensor());
            this->sensor_muscle.push_back(new Sensor());
            this->sensor_bone.push_back(new Sensor());
            this->sensor_bmi.push_back(new Sensor());
          }
      }
  public:
      time_t now(){
        return id(homeassistant_time).now().timestamp;
      }

      static BS444Scale* instance()  {
          static BS444Scale* INSTANCE = new BS444Scale();
          return INSTANCE;
      }
      
      // we found a device
      void onResult(BLEAdvertisedDevice &advertisedDevice)
      {
        // We have found a device, let us now see if it contains the service we are looking for.
        if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(mServiceUUID))
        {
          ESP_LOGD("BS444","BLE Advertised Device found with our service%s", advertisedDevice.toString().c_str());
          BLEDevice::getScan()->stop(); //we found our device
          scaleDevice = new BLEAdvertisedDevice(advertisedDevice);
          mDoConnect = true;
          mDoScan = true;
        }
      }

      void setup() override {

        BLEDevice::init("");

        // Retrieve a Scanner and set the callback we want to use to be informed when we
        // have detected a new device.  Specify that we want active scanning and start the
        // scan to run for 5 seconds.
        BLEScan *pBLEScan = BLEDevice::getScan();

        //Register a callback function to be called when the device is discovered
        // and send the device to our BS444Scale object
        class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
        {
          public:
            MyAdvertisedDeviceCallbacks(BS444Scale *scale): mScale(scale){};
            MyAdvertisedDeviceCallbacks() = delete;
            /**
             * Called for each advertising BLE server.
             */
            void onResult(BLEAdvertisedDevice advertisedDevice)
            {
                mScale->onResult(advertisedDevice);
            }   // onResult

          private:
             BS444Scale *mScale;
          };

        pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(this));
        pBLEScan->setInterval(1349);
        pBLEScan->setWindow(449);
        pBLEScan->setActiveScan(false);
        mDoConnect = false;
        mDoScan = true;
      }

      void loop() override {

        if (millis() > (bletime + 1000)) {
          bletime = millis();
          // executes every second
           bleClient_loop();
        }
      }

      void notifyCallback(
          BLERemoteCharacteristic *pBLERemoteCharacteristic,
          uint8_t *pData,
          size_t length,
          bool isNotify)
      {
        ESP_LOGD("BS444","Notify callback for characteristic %s of data length %d data:", pBLERemoteCharacteristic->getUUID().toString().c_str(), pBLERemoteCharacteristic->getHandle(), length);
        if (pBLERemoteCharacteristic->getUUID().equals(Char_person))
        {
          mPerson = Person::decode(pData);
          ESP_LOGD("BS444","data %s:", mPerson.toString().c_str());
        }
        else if (pBLERemoteCharacteristic->getUUID().equals(Char_weight))
        {
          auto data = Weight::decode(pData);
          if (data.timestamp < now())
          {
            ESP_LOGD("BS444","data %s:", data.toString().c_str());
            if (!mWeight.valid || (mWeight < data))
              mWeight = data;
          }
        }
        else if (pBLERemoteCharacteristic->getUUID().equals(Char_body))
        {
          auto data = Body::decode(pData);
          if (data.timestamp < now())
          {
            ESP_LOGD("BS444","data %s:", data.toString().c_str());
            if (!mBody.valid || (mBody < data))
              mBody = data;
          }
        }

      }
  
      void onDisconnect(BLEClient *pclient)
      {
        ESP_LOGD("BS444","onDisconnect");
        if (mPerson.valid)
        {
          ESP_LOGD("BS444","person %s:", mPerson.toString().c_str());
          if (mWeight.valid && (mWeight.person == mPerson.person)){
            ESP_LOGD("BS444","%s:", mWeight.toString(mPerson).c_str());
            this->sensor_weights[mWeight.person-1]->publish_state(mWeight.weight);
            this->sensor_bmi[mWeight.person-1]->publish_state(mWeight.weight / (mPerson.size * mPerson.size));
          }
          if (mBody.valid && (mBody.person == mPerson.person)){
            ESP_LOGD("BS444","%s:", mBody.toString().c_str());
            this->sensor_kcal[mBody.person-1]->publish_state(mBody.kcal);
            this->sensor_fat[mBody.person-1]->publish_state(mBody.fat);
            this->sensor_tbw[mBody.person-1]->publish_state(mBody.tbw);
            this->sensor_muscle[mBody.person-1]->publish_state(mBody.muscle);
            this->sensor_bone[mBody.person-1]->publish_state(mBody.bone);
          }
        }

        mConnected = false;
        mDoScan = true;
      }

      //Function that attempts to connect to the device, here is the KEY!!!
      bool connectToServer()
      {
        //scaleDevice is a variable containing information about the device to be connected.
        //In the device scan below, information will be entered into the corresponding variable.
        ESP_LOGD("BS444","Creating a connection to %s", scaleDevice->getAddress().toString().c_str());

        //Create a client (Central) class to connect to the server (Pheriphral)
        BLEClient *pClient = BLEDevice::createClient();
        ESP_LOGI("BS444","Created client");

        class MyClientCallback : public BLEClientCallbacks
        {
          public:
            MyClientCallback(BS444Scale *scale): mScale(scale){};
            MyClientCallback() = delete;

            void onConnect(BLEClient *pclient)
            {
            }

            void onDisconnect(BLEClient *pclient)
            {
              mScale->onDisconnect(pclient);
            }
          private:
            BS444Scale *mScale;
        };

        //Set up a callback function to receive connection status events
        pClient->setClientCallbacks(new MyClientCallback(this));

        //Finally tried to connect to the server (Pheriphral) device!!!
        pClient->connect(scaleDevice); // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
        ESP_LOGI("BS444","Connected to server");
        // set client to request maximum MTU from server (default is 23 otherwise)
        pClient->setMTU(517); 

        //Receive service information when connected
        for (auto service: *pClient->getServices()){
           ESP_LOGD("BS444","service:  %s, handle %d", service.first.c_str(), service.second->getHandle());
        }

        //
        // Obtain a reference to the service we are after in the remote BLE server.
        //
        BLERemoteService *pRemoteService = pClient->getService(mServiceUUID);
        if (pRemoteService == nullptr)
        {
          ESP_LOGE("BS444","Failed to find our service UUID: %s", mServiceUUID.toString().c_str());
          pClient->disconnect();
          return false;
        }
        ESP_LOGD("BS444","service %s found", mServiceUUID.toString().c_str());

        for (auto characteristic: *pRemoteService->getCharacteristics()){
           ESP_LOGD("BS444","characteristic:  %s -> handle %d", characteristic.first.c_str(), characteristic.second->getHandle());
        }
        //
        // subscribe to characteristics
        //
        for (auto &characteristic : mCharacteristics)
        {
          auto pRemoteCharacteristic = pRemoteService->getCharacteristic(characteristic);
          if (pRemoteCharacteristic == nullptr)
          {
            ESP_LOGE("BS444","Failed to find our characteristic UUID: %s", characteristic.toString().c_str());
            pClient->disconnect();
          }
          ESP_LOGD("BS444","Found characteristic: Handle: %d, UUID: %s", pRemoteCharacteristic->getHandle(), characteristic.toString().c_str());

          // Read the value of the characteristic.
          if (pRemoteCharacteristic->canRead())
          {
            ESP_LOGD("BS444","The characteristic canRead");
            std::string value = pRemoteCharacteristic->readValue();
            ESP_LOGD("BS444","The characteristic value was: %s", value.c_str());
          }

          if (pRemoteCharacteristic->canNotify())
          {
            ESP_LOGD("BS444","The characteristic canNotify");
            pRemoteCharacteristic->registerForNotify(std::bind (&BS444Scale::notifyCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4) );
            ESP_LOGD("BS444","Registered Callback");
          }

          if (pRemoteCharacteristic->canIndicate())
          {
            ESP_LOGD("BS444","The characteristic canIndicate");
            pRemoteCharacteristic->registerForNotify(std::bind (&BS444Scale::notifyCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4) );
            ESP_LOGD("BS444","Registered Callback");

            for (auto descriptor: *pRemoteCharacteristic->getDescriptors()){
              ESP_LOGD("BS444","descriptor:  %s, handle %d", descriptor.first.c_str(), descriptor.second->getHandle());
            }
            // do we find a descriptor with a handle of the pRemoteCharacteristic + 1 ?
            auto it = std::find_if(pRemoteCharacteristic->getDescriptors()->begin(), pRemoteCharacteristic->getDescriptors()->end(), [&pRemoteCharacteristic](std::map<std::string, BLERemoteDescriptor *>::value_type &descriptor)
                                   { return (descriptor.second->getHandle() == pRemoteCharacteristic->getHandle() + 1); });
            if (it != pRemoteCharacteristic->getDescriptors()->end())
            {
              ESP_LOGD("BS444","descriptor found:  %s, handle %d", it->first.c_str(), it->second->getHandle());
              const uint8_t indicationOn[] = {0x2, 0x0};
              it->second->writeValue((uint8_t *)indicationOn, 2, true);
              ESP_LOGD("BS444","Indication Enabled");
            }
          }
        }
        auto pRemoteCharacteristic = pRemoteService->getCharacteristic(Char_command);
        if (pRemoteCharacteristic != nullptr)
        {
          ESP_LOGI("BS444","Found characteristic: Handle: %d, UUID: %s", pRemoteCharacteristic->getHandle(), Char_command.toString().c_str());
          if(pRemoteCharacteristic->canWrite()) {
            ESP_LOGD("BS444","The characteristic canWrite");

            /*
            Send the unix timestamp in little endian order preceded by 02 as
            bytearray to Char_command. This will resync the scale's RTC.
            While waiting for a response notification, which will never
            arrive, the scale will emit 30 Indications on 0x1b (weight)
            and 0x1e (body) each.
            */
            uint8_t byteArray[5] = {2, 0, 0, 0, 0};
            convertTimestampToLittleEndian(now()-time_offset, &byteArray[1]);
            pRemoteCharacteristic->writeValue((uint8_t *)byteArray, sizeof(byteArray), true);
          }
          else
          {
            ESP_LOGE("BS444","Can not write to characteristic UUID: %s", Char_command.toString().c_str());
            pClient->disconnect();
          }
        }

        //If you have reached this point, set a variable to indicate that the connection was successful.
        mConnected = true;
        return true;
      }

      void bleClient_loop()
      {
        // If the flag "mDoConnect" is true then we have scanned for and found the desired
        // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
        // connected we set the mConnected flag to be true.
        if (mDoConnect)
        {
          if (connectToServer())
            ESP_LOGI("BS444","We are now connected to the BLE Server.");
          else
            ESP_LOGE("BS444","We have failed to connect to the server; there is nothing more we will do.");
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
           ESP_LOGI("BS444","starting scan in 30 seconds.");
           //start a next scan in 30 time
           mDoScan = false;
           mScan = true;

           bletime = millis() + kSleepTimeBetweenScans*1000; // delay next itertation
        }
        else if (mScan)
        {
          mScan = false;
          mDoScan = true; // restarting a delayed scan whenever possible
           //If the connection is released and mDoScan is true, start scanning
          ESP_LOGI("BS444","Restarting scan.");
          BLEDevice::getScan()->start(1, false);
        }
      }
};