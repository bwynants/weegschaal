#include "esphome.h"
#ifdef USE_TIME
#include "esphome/core/time.h"
#endif
#include "Scale.h"

namespace esphome
{
namespace medisana_bs444
{

  esp32_ble::ESPBTUUID Serv_SCALE = esp32_ble::ESPBTUUID::from_raw("000078b2-0000-1000-8000-00805f9b34fb"); // the service

  esp32_ble::ESPBTUUID Char_person = esp32_ble::ESPBTUUID::from_raw("00008a82-0000-1000-8000-00805f9b34fb"); // person data handle 22
  esp32_ble::ESPBTUUID Char_weight = esp32_ble::ESPBTUUID::from_raw("00008a21-0000-1000-8000-00805f9b34fb"); // weight data handle 25
  esp32_ble::ESPBTUUID Char_body = esp32_ble::ESPBTUUID::from_raw("00008a22-0000-1000-8000-00805f9b34fb");   // body data handle 28

  esp32_ble::ESPBTUUID Char_command = esp32_ble::ESPBTUUID::from_raw("00008a81-0000-1000-8000-00805f9b34fb"); // command register handle 31

  time_t time_offset = 1262304000;

  /*******************************************************************************/
  std::string timeAsString(time_t time)
  {
    return ESPTime::from_epoch_local(time).strftime("%Y-%m-%dT%H:%M:%S");
  }

  time_t sanitize_timestamp(time_t timestamp)
  {
    time_t retTS = 0;

    // Fail-safe: The timestamp will only be sanitized if it will be
    // below below the maximum unix timestamp (2147483647).
    if (timestamp + time_offset < std::numeric_limits<time_t>::max())
      retTS = timestamp + time_offset;
    else
      retTS = timestamp;

    // If the non-sanitized timestamp is already above the maximum unix timestamp,
    // 0 will be taken instead.
    if (timestamp >= std::numeric_limits<time_t>::max())
      retTS = 0;

    return retTS;
  }

  void convertTimestampToLittleEndian(time_t timestamp, uint8_t *byteArray)
  {
    // Convert timestamp to little-endian order
    byteArray[0] = static_cast<uint8_t>(timestamp & 0xFF);
    byteArray[1] = static_cast<uint8_t>((timestamp >> 8) & 0xFF);
    byteArray[2] = static_cast<uint8_t>((timestamp >> 16) & 0xFF);
    byteArray[3] = static_cast<uint8_t>((timestamp >> 24) & 0xFF);
  }

  std::string Person::toString() const
  {
    std::stringstream str;
    if (valid)
    {
      // Print the date and time in a user-readable format
      str << "Person: " << person;
      str << "; gender: " << (male ? "male" : "female");
      str << "; age: " << age;
      str << "; size: " << size;
      str << "; activity: " << (highActivity ? "high" : "normal");
    }
    else
      str << "invalid";
    return str.str();
  }

  Person Person::decode(const uint8_t *values)
  {
    /*
      decodePerson
      Handle: 0x25 (Person)
      Value:
          Byte  Data                         Value/Return   Interpretation pattern
          0     fixed byte (validity check)  [0x84]         B (integer, lenght 1)
          1     -pad byte-                                  x (pad byte)
          2     person                       [1..8]         B ((integer, lenght 1)
          3     -pad byte-                                  x (pad byte)
          4     gender (1=male, 2=female)    [1|2]          B (integer, lenght 1)
          5     age                          [0..255 years] B (integer, lenght 1)
          6     size                         [0..255 cm]    B (integer, lenght 1)
          7     -pad byte-                                  x (pad byte)
          8     activity (0=normal, 3=high)  [0|3]          B (integer, lenght 1)
          --> Interpretation pattern:                       BxBxBBBxB
    */
    Person result;

    result.valid = (values[0] == 0x84);
    result.person = values[2];
    result.male = (values[4] == 1);
    result.age = values[5];
    result.size = values[6] / 100.0;
    result.highActivity = (values[8] == 3);
    return result;
  }

  std::string Weight::toString(const Person &person)
  {
    std::stringstream str;
    if (valid)
    {
      str << "Person: " << this->person;
      str << "; Time:" << timeAsString(timestamp);
      str << "; weight: " << weight;
      if (person.valid)
      {
        // Normale BMI formule: gewicht / lengte^2
        // Nieuwe BMI formule: 1,3 * gewicht / lengte^2,5
        str << "; bmi: " << (weight / (person.size * person.size));
      }
    }
    else
      str << "invalid";
    return str.str();
  }

  Weight Weight::decode(const uint8_t *values)
  {
    /*
      decodeWeight
      Handle: 0x1b (Weight)
      Value:
          Byte  Data                         Value/Return       Interpretation pattern
           0    fixed byte (validity check)  [0x1d]             B (integer, length 1)
           1    weight                       [5,0..180,0 kg]    H (integer, length 2)
           2    weight
           3    -pad byte-                                      x (pad byte)
           4    -pad byte-                                      x (pad byte)
           5    timestamp                    Unix, date & time  I (integer, length 4)
           6    timestamp
           7    timestamp
           8    timestamp
           9    -pad byte-                                      x (pad byte)
          10    -pad byte-                                      x (pad byte)
          11    -pad byte-                                      x (pad byte)
          12    -pad byte-                                      x (pad byte)
          13    person                       [1..8]             B (integer, length 1)
          --> Interpretation pattern:                           BHxxIxxxxB
    */
    Weight result;

    result.valid = (values[0] == 0x1d);
    result.weight = ((values[2] << 8) | values[1]) / 100.0;
    result.timestamp = sanitize_timestamp((values[8] << 24) | (values[7] << 16) | (values[6] << 8) | values[5]);
    result.person = values[13];

    return result;
  }

  std::string Body::toString()
  {
    std::stringstream str;
    if (valid)
    {
      str << "Person: " << person;
      str << "; Time:" << timeAsString(timestamp);
      str << "; kcal: " << kcal;
      str << "; fat: " << fat;
      str << "; tbw: " << tbw;
      str << "; muscle: " << muscle;
      str << "; bone: " << bone;
    }
    else
      str << "invalid";
    return str.str();
  }

  Body Body::decode(const uint8_t *values)
  {
    /*
      decodeBody
      Handle: 0x1e (Body)
      Value:
          Byte  Data                          Value/Return       Interpretation pattern
           0    fixed byte (validity check)   [0x6f]             B (integer, lenght 1)
           1    timestamp                     Unix, date & time  I (integer, length 4)
           2    timestamp
           3    timestamp
           4    timestamp
           5    person                        [1..8]             B (integer, lenght 1)
           6    kcal                          [0..65025 Kcal]    H (integer, length 2)
           7    kcal
           8    fat (percentage of body fat)  [0..100,0 %]       H (integer, length 2)
           9    fat (percentage of body fat)
          10    tbw (percentage of water)     [0..100,0 %]       H (integer, length 2)
          11    tbw (percentage of water)
          12    muscle (percentage of muscle) [0..100,0 %]       H (integer, length 2)
          13    muscle (percentage of muscle)
          14    bone (bone weight)            [0..100,0 %]       H (integer, length 2)
          15    bone (bone weight)
          --> Interpretation pattern:                            BIBBHHHHH
      Notes: For kcal, fat, tbw, muscle, bone: First nibble = 0xf
    */
    Body result;

    result.valid = (values[0] == 0x6f);
    result.timestamp = sanitize_timestamp((values[4] << 24) | (values[3] << 16) | (values[2] << 8) | values[1]);
    result.person = (values[5]);
    result.kcal = (values[7] << 8 | values[6]);
    result.fat = (0x0fff & (values[9] << 8 | values[8])) / 10.0;
    result.tbw = (0x0fff & (values[11] << 8 | values[10])) / 10.0;
    result.muscle = (0x0fff & (values[13] << 8 | values[12])) / 10.0;
    result.bone = (0x0fff & (values[15] << 8 | values[14])) / 10.0;

    return result;
  }
} // namespace medisana_bs444
} // namespace esphome
