#include "pbhub.h"

#include "esphome/core/log.h"

namespace esphome {
namespace pbhub {

static const char *const TAG = "pbhub";

static constexpr uint8_t REGISTER_FIRMWARE_VERSION = 0xFE;
static constexpr uint8_t REGISTER_DIGITAL_WRITE = 0x00;
static constexpr uint8_t REGISTER_ANALOG_READ = 0x06;

float PbHubComponent::get_setup_priority() const { return setup_priority::HARDWARE; }

void PbHubComponent::setup() {
  auto version = this->read_byte(REGISTER_FIRMWARE_VERSION);
  if (!version.has_value()) {
    ESP_LOGE(TAG, "PbHub not found at I2C address 0x%02X", this->get_i2c_address());
    this->mark_failed();
    return;
  }
  this->firmware_version_ = *version;
}

void PbHubComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "PbHub:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, ESP_LOG_MSG_COMM_FAIL);
    return;
  }
  ESP_LOGCONFIG(TAG, "  Firmware Version: %u", this->firmware_version_);
  ESP_LOGCONFIG(TAG, "  Allow Simultaneous Pumps: %s", YESNO(this->allow_simultaneous_pumps_));
}

uint8_t PbHubComponent::channel_register_(uint8_t channel, uint8_t offset) {
  if (channel == 5) {
    channel++;
  }
  return static_cast<uint8_t>(((channel + 4) << 4) | offset);
}

bool PbHubComponent::read_analog(uint8_t channel, uint16_t *value) {
  if (channel > 5 || value == nullptr) {
    return false;
  }

  uint8_t data[2]{0, 0};
  const uint8_t reg = channel_register_(channel, REGISTER_ANALOG_READ);
  if (this->read_register(reg, data, sizeof(data)) != i2c::ERROR_OK) {
    this->status_set_warning(LOG_STR("analog read failed"));
    return false;
  }

  *value = static_cast<uint16_t>((static_cast<uint16_t>(data[1]) << 8) | data[0]);
  this->status_clear_warning();
  return true;
}

bool PbHubComponent::write_digital(uint8_t channel, uint8_t index, bool value) {
  if (channel > 5 || index > 1) {
    return false;
  }

  const uint8_t reg = channel_register_(channel, REGISTER_DIGITAL_WRITE + index);
  const uint8_t data = value ? 1 : 0;
  if (!this->write_byte(reg, data)) {
    this->status_set_warning(LOG_STR("digital write failed"));
    return false;
  }

  this->status_clear_warning();
  return true;
}

void PbHubComponent::register_pump(PbHubSwitch *pump) {
  if (pump == nullptr || pump->get_channel() > 5) {
    return;
  }
  this->pumps_[pump->get_channel()] = pump;
}

void PbHubComponent::turn_off_other_pumps(PbHubSwitch *active_pump) {
  if (this->allow_simultaneous_pumps_) {
    return;
  }
  for (auto *pump : this->pumps_) {
    if (pump != nullptr && pump != active_pump && pump->state) {
      pump->turn_off();
    }
  }
}

void PbHubSensor::update() {
  if (this->parent_ == nullptr) {
    ESP_LOGW(TAG, "No PbHub parent configured for channel %u", this->channel_);
    this->status_set_warning();
    return;
  }

  uint16_t value = 0;
  if (!this->parent_->read_analog(this->channel_, &value)) {
    ESP_LOGW(TAG, "Failed to read PbHub channel %u analog value", this->channel_);
    this->status_set_warning();
    return;
  }

  this->status_clear_warning();
  this->publish_state(value);
}

void PbHubSensor::dump_config() {
  LOG_SENSOR("", "PbHub Moisture Raw", this);
  ESP_LOGCONFIG(TAG, "  Channel: %u", this->channel_);
  LOG_UPDATE_INTERVAL(this);
}

float PbHubSwitch::get_setup_priority() const { return setup_priority::HARDWARE; }

void PbHubSwitch::setup() {
  if (this->parent_ != nullptr) {
    this->parent_->register_pump(this);
  }

  this->cancel_timeout("max_run");
  if (this->parent_ == nullptr || !this->parent_->write_digital(this->channel_, this->pin_index_, false)) {
    ESP_LOGW(TAG, "Failed to force PbHub channel %u pump off during setup", this->channel_);
    this->publish_state(false);
    return;
  }
  this->publish_state(false);
}

void PbHubSwitch::dump_config() {
  LOG_SWITCH("", "PbHub Pump", this);
  ESP_LOGCONFIG(TAG, "  Channel: %u", this->channel_);
  ESP_LOGCONFIG(TAG, "  Pin Index: %u", this->pin_index_);
  ESP_LOGCONFIG(TAG, "  Max Run Time: %ums", this->max_run_time_);
}

void PbHubSwitch::write_state(bool state) {
  if (this->parent_ == nullptr) {
    ESP_LOGW(TAG, "No PbHub parent configured for channel %u", this->channel_);
    this->publish_state(false);
    return;
  }

  if (state) {
    this->parent_->turn_off_other_pumps(this);
  } else {
    this->cancel_timeout("max_run");
  }

  if (!this->parent_->write_digital(this->channel_, this->pin_index_, state)) {
    ESP_LOGW(TAG, "Failed to write PbHub channel %u pump state", this->channel_);
    this->publish_state(false);
    return;
  }

  this->publish_state(state);

  if (state) {
    this->set_timeout("max_run", this->max_run_time_, [this]() {
      ESP_LOGW(TAG, "PbHub channel %u pump max run time reached", this->channel_);
      this->turn_off();
    });
  }
}

}  // namespace pbhub
}  // namespace esphome
