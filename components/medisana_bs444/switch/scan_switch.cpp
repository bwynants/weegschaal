#include "scan_switch.h"
#include "esphome/core/log.h"

namespace esphome {
namespace medisana_bs444 {

#ifdef USE_ESP32

void ScanSwitch::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Scan Switch '%s'...", this->name_.c_str());

  bool initial_state = this->get_initial_state_with_restore_mode().value_or(false);

  if (initial_state) {
    this->turn_on();
  } else {
    this->turn_off();
  }
}

void ScanSwitch::write_state(bool state) {
  this->publish_state(state);
}

#endif

}  // namespace medisana_bs444
}  // namespace esphome
