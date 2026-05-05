#pragma once

#include <iostream>
#include <sstream>

#include "esphome/components/esp32_ble/ble_uuid.h"

namespace esphome
{
  namespace medisana_bs444
  {

    extern const esp32_ble::ESPBTUUID Serv_SCALE; // the service

    extern const esp32_ble::ESPBTUUID Char_person; // person data handle 22
    extern const esp32_ble::ESPBTUUID Char_weight; // weight data handle 25
    extern const esp32_ble::ESPBTUUID Char_body;   // body data handle 28

    extern const esp32_ble::ESPBTUUID Char_command; // command register handle 31
    
    constexpr double LB_TO_KG = 0.45359237;

    //   On some scales (e.g. BS410 and BS444, maybe others as well), time=0
    //   equals 1/1/2010. However, goal is to have unix-timestamps. Thus, the
    //   function converts the "scale-timestamp" to unix-timestamp by adding
    //   the time-offset (most scales: 1262304000 = 01.01.2010) to the timestamp.

    // Assuming time_offset is a constant defined somewhere in the code
    extern const time_t time_offset;

    time_t sanitize_timestamp(time_t timestamp, bool useTimeoffset);

    void convertTimestampToLittleEndian(time_t timestamp, uint8_t *byteArray);

    std::string timeAsString(time_t time);

    class Person
    {
    public:
      auto operator<=>(const Person &) const = default;

    public:
      bool valid = false;
      u_int32_t person = 255;
      bool male;
      u_int32_t age;
      double size;
      bool highActivity;

      std::string toString() const;
      static Person decode(const uint8_t *values);
    };

    class Weight
    {
    public:
      auto operator<=>(const Weight &) const = default;

    public:
      bool valid = false;
      time_t timestamp = 0;
      u_int32_t person;
      double weight;

      std::string toString(const Person &person = Person()) const;
      static Weight decode(const uint8_t *values, bool useTimeoffset);
    };

    class Body
    {
    public:
      auto operator<=>(const Body &) const = default;

    public:
      bool valid = false;
      time_t timestamp = 0;
      u_int32_t person;
      u_int32_t kcal;
      double fat;
      double tbw;
      double muscle;
      double bone;
      std::string toString() const;
      static Body decode(const uint8_t *values, bool useTimeoffset);
    };

    /// Struct containing all measurement data for a user, passed to on_user_metrics_updated trigger
    struct UserMeasurement
    {
      uint8_t user_id{0}; // 1-8

      // Person data
      uint8_t age{0};
      float size{0}; // in meters
      bool is_male{false};
      bool high_activity{false};

      // Weight data
      float weight{0};
      float bmi{0};
      bool has_weight{false};

      // Body composition
      uint32_t kcal{0};
      float fat{0};
      float tbw{0};
      float muscle{0};
      float bone{0};
      bool has_body{false};

      time_t timestamp{0};
    };
  } // namespace medisana_bs444
} // namespace esphome
