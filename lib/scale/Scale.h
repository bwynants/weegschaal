#ifndef SCALE_h
#define SCALE_h

#include <iostream>
#include <sstream>

#include "BLEDevice.h"

extern BLEUUID Serv_SCALE;  // the service

extern BLEUUID Char_person;  // person data handle 22
extern BLEUUID Char_weight;  // weight data handle 25
extern BLEUUID Char_body;    // body data handle 28

extern BLEUUID Char_command; // command register handle 31

//   On some scales (e.g. BS410 and BS444, maybe others as well), time=0
//   equals 1/1/2010. However, goal is to have unix-timestamps. Thus, the
//   function converts the "scale-timestamp" to unix-timestamp by adding
//   the time-offset (most scales: 1262304000 = 01.01.2010) to the timestamp.

// Assuming time_offset is a constant defined somewhere in your code
extern time_t time_offset;

time_t sanitize_timestamp(time_t timestamp);

void convertTimestampToLittleEndian(time_t timestamp, uint8_t *byteArray);

std::string timeAsString(time_t time);

class Person
{
public:
  bool valid = false;
  u_int32_t person = 255;
  bool male;
  u_int32_t age;
  double size;
  bool highActivity;

  std::string toString() const;
  static Person decode(const uint8_t* values);

  friend bool operator<(const Person& l, const Person& r)
  { return std::tie(l.valid, l.person) <  std::tie(r.valid, r.person); }
  friend bool operator==(const Person& l, const Person& r)
  { return (l.valid == r.valid) && (l.person ==  r.person); }

};
inline bool operator> (const Person& lhs, const Person& rhs) { return rhs < lhs; }
inline bool operator<=(const Person& lhs, const Person& rhs) { return !(lhs > rhs); }
inline bool operator>=(const Person& lhs, const Person& rhs) { return !(lhs < rhs); }
inline bool operator!=(const Person& lhs, const Person& rhs) { return !(lhs == rhs); }

class Weight
{
public:
  bool valid = false;
  time_t timestamp = 0;
  u_int32_t person;
  double weight;

  std::string toString(const Person& person=Person());
  static Weight decode(const uint8_t* values);

  friend bool operator<(const Weight& l, const Weight& r)
  { return std::tie(l.valid, l.timestamp) <  std::tie(r.valid, r.timestamp); }
  friend bool operator==(const Weight& l, const Weight& r)
  { return (l.valid == r.valid) && (l.timestamp ==  r.timestamp); }
};
inline bool operator> (const Weight& lhs, const Weight& rhs) { return rhs < lhs; }
inline bool operator<=(const Weight& lhs, const Weight& rhs) { return !(lhs > rhs); }
inline bool operator>=(const Weight& lhs, const Weight& rhs) { return !(lhs < rhs); }
inline bool operator!=(const Weight& lhs, const Weight& rhs) { return !(lhs == rhs); }


class Body
{
public:
  bool valid = false;
  time_t timestamp = 0;
  u_int32_t person;
  u_int32_t kcal;
  double fat;
  double tbw;
  double muscle;
  double bone;
  std::string toString();
  static Body decode(const uint8_t *values);
 
  friend bool operator<(const Body& l, const Body& r)
  { return std::tie(l.valid, l.timestamp) <  std::tie(r.valid, r.timestamp); }

  friend bool operator==(const Body& l, const Body& r)
  { return (l.valid == r.valid) && (l.timestamp ==  r.timestamp); }

};
inline bool operator> (const Body& lhs, const Body& rhs) { return rhs < lhs; }
inline bool operator<=(const Body& lhs, const Body& rhs) { return !(lhs > rhs); }
inline bool operator>=(const Body& lhs, const Body& rhs) { return !(lhs < rhs); }
inline bool operator!=(const Body& lhs, const Body& rhs) { return !(lhs == rhs); }

#endif