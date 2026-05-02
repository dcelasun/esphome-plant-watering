#pragma once

#include <array>
#include <cstdint>

#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/core/component.h"

namespace esphome {
namespace pbhub {

class PbHubSwitch;

class PbHubComponent : public Component, public i2c::I2CDevice {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override;

  void set_allow_simultaneous_pumps(bool allow) { this->allow_simultaneous_pumps_ = allow; }

  bool read_analog(uint8_t channel, uint16_t *value);
  bool write_digital(uint8_t channel, uint8_t index, bool value);
  void register_pump(PbHubSwitch *pump);
  void turn_off_other_pumps(PbHubSwitch *active_pump);

 protected:
  static uint8_t channel_register_(uint8_t channel, uint8_t offset);

  bool allow_simultaneous_pumps_{false};
  uint8_t firmware_version_{0};
  std::array<PbHubSwitch *, 6> pumps_{};
};

class PbHubSensor : public sensor::Sensor, public PollingComponent {
 public:
  void update() override;
  void dump_config() override;
  void set_parent(PbHubComponent *parent) { this->parent_ = parent; }
  void set_channel(uint8_t channel) { this->channel_ = channel; }

 protected:
  PbHubComponent *parent_{nullptr};
  uint8_t channel_{0};
};

class PbHubSwitch : public switch_::Switch, public Component {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override;

  void set_parent(PbHubComponent *parent) { this->parent_ = parent; }
  void set_channel(uint8_t channel) { this->channel_ = channel; }
  void set_pin_index(uint8_t pin_index) { this->pin_index_ = pin_index; }
  void set_max_run_time(uint32_t max_run_time) { this->max_run_time_ = max_run_time; }
  uint8_t get_channel() const { return this->channel_; }

 protected:
  void write_state(bool state) override;

  PbHubComponent *parent_{nullptr};
  uint8_t channel_{0};
  uint8_t pin_index_{1};
  uint32_t max_run_time_{10000};
};

}  // namespace pbhub
}  // namespace esphome
