#pragma once

#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"
#include "../medisanabs444.h"

#ifdef USE_ESP32

namespace esphome {
namespace medisana_bs444 {

class ScanSwitch : public switch_::Switch, public Component {
 public:
  ScanSwitch() = default;

 protected:
  void write_state(bool state) override;
  void setup() override;
};
#endif

}  // namespace medisana_bs444
}  // namespace esphome
