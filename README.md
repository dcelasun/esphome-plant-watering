# ESPHome Plant Watering

ESPHome firmware for automated watering of up to 6 plants with an M5Stack AtomS3-Lite or Atom Lite, M5Stack PbHub v1.1, and M5Stack Unit Watering modules.

<p align="center">
    <img src="/ha-device.png" alt="Device in Home Assistant">
</p>

<!-- TOC -->
* [ESPHome Plant Watering](#esphome-plant-watering)
  * [Hardware](#hardware)
  * [Board Selection](#board-selection)
  * [Configuration](#configuration)
  * [Installation](#installation)
    * [ESPHome](#esphome)
    * [Home Assistant](#home-assistant)
  * [Calibration](#calibration)
  * [Home Assistant Automation Examples](#home-assistant-automation-examples)
<!-- TOC -->

<p align="center">
    <img src="/diagram.png" alt="Hardware Diagram">
</p>

## Hardware

- One of:
  - M5Stack [AtomS3-Lite](https://docs.m5stack.com/en/core/AtomS3%20Lite) (ESP32-S3FN8), or
  - M5Stack [Atom Lite](https://docs.m5stack.com/en/core/atom_lite) (ESP32-PICO-D4).
- M5Stack [PbHub v1.1](https://docs.m5stack.com/en/unit/pbhub_1.1).
- 1 to 6 M5Stack [Unit Watering](https://docs.m5stack.com/en/unit/watering) modules.
- USB-C power supply for the controller.
- Optional [Unit TypeC to Grove](https://docs.m5stack.com/en/unit/typec2grove) or equivalent external 5V supply if you want to turn on multiple pumps at once.

Expected wiring:

- Controller Grove port to PbHub input.
- PbHub port `0` to Plant 1 Unit Watering.
- PbHub port `1` to Plant 2 Unit Watering.
- PbHub ports `2..5` are configured but disabled by default in Home Assistant.

The Grove I2C pins differ between the two boards, which is why each has its own configuration:

| Board        | SDA   | SCL   | Serial device  |
|--------------|-------|-------|----------------|
| AtomS3-Lite  | `G2`  | `G1`  | `/dev/ttyACM0` |
| Atom Lite    | `G26` | `G32` | `/dev/ttyUSB0` |

The AtomS3-Lite exposes native USB, while the Atom Lite uses a CH9102/CP210x USB-serial bridge, hence the different device nodes.

Either board alone can only supply one pump at a time, so the firmware defaults to `allow_simultaneous_pumps: "false"`.

## Board Selection

Each board has a `plant-watering-<board-name>.yaml` in the repository root, and per-board overrides in `boards/<board-name>.yaml`:

```
plant-watering-atoms3-lite.yaml   # AtomS3-Lite
plant-watering-atom-lite.yaml     # Atom Lite
boards/                           # per-board pins, chip and build flags
common/plant-watering.yaml        # shared configuration
```

Every `make` target takes a `BOARD` variable and default to `atoms3-lite`:

```sh
make compile BOARD=atom-lite
```

`BOARD` also picks the right default serial device, so `make upload BOARD=atom-lite` targets `/dev/ttyUSB0`.

## Configuration

Copy the example secrets file and edit the values:
```sh
cp secrets.example.yaml secrets.yaml
```

Customise `common/plant-watering.yaml` if needed. These settings are shared by all boards:

- `pump_max_run_time`: default `10s`.
- `allow_simultaneous_pumps`: default `"false"`.
- `plant_N_dry_raw` and `plant_N_wet_raw`: calibration values for each probe.
- `device_name`: base hostname. ESPHome appends the device MAC suffix at runtime.

To allow multiple pumps at once with external pump power:

```yaml
substitutions:
  allow_simultaneous_pumps: "true"
```

## Installation

Make sure you have [uv](https://docs.astral.sh/uv/#installation) installed. On Linux, it's available in most package managers.

### ESPHome

All dependencies, including `esphome` are handled by `uv` without touching your system files.

Install ESPHome and dependencies:
```sh
make setup
```

All of the commands below default to the AtomS3-Lite. Append `BOARD=atom-lite` to any of them to target the Atom Lite instead.

Validate the configuration:
```sh
make validate
```

Compile:
```sh
make compile
```

Flash over USB (assumes `/dev/ttyACM0` on the AtomS3-Lite, `/dev/ttyUSB0` on the Atom Lite):
```sh
make upload
```

Override the serial device if needed:
```sh
make upload DEVICE=/dev/ttyACM1
```

Watch logs:
```sh
make logs
```

You can also watch logs by the device's mDNS hostname, which is the base name plus the last six MAC hex digits, for example:
```sh
make logs DEVICE=plant-watering-a1b2c3.local
```

### Home Assistant

If you have the [ESPHome integration](https://esphome.io/integrations/esphome.html) installed, your device should show up automatically in Home Assistant.

Alternatively, you can [manually add the device](https://my.home-assistant.io/redirect/config_flow_start?domain=esphome) to Home Assistant by its IP address.

It's strongly recommended to use a static IP address for the device.

## Calibration

The Unit Watering moisture output usually reads higher when dry and lower when wet.

For each connected plant:

1. Run logs and note the raw value with the probe dry.
2. Wet the soil or probe area and note the raw value after it stabilizes.
3. Put those values into `plant_N_dry_raw` and `plant_N_wet_raw`.
4. Adjust `dry_percent_threshold` if Home Assistant should water earlier or later.

## Home Assistant Automation Examples

Example: water Plant 1 at 07:30 only if moisture is below 35%.

```yaml
alias: Water Plant 1 When Dry
trigger:
  - platform: time
    at: "07:30:00"
condition:
  - condition: numeric_state
    entity_id: sensor.plant_watering_plant_1_soil_moisture_percent
    below: 35
action:
  - service: switch.turn_on
    target:
      entity_id: switch.plant_watering_plant_1_water_pump
mode: single
```

Example: prevent watering if the plant is already saturated.

```yaml
condition:
  - condition: not
    conditions:
      - condition: state
        entity_id: sensor.plant_watering_plant_1_soil_moisture_state
        state: "Saturated"
```

The firmware enforces a pump max runtime, so Home Assistant does not need to turn the switch off for basic safety. 
Of course, you can still add explicit delays and `switch.turn_off` actions.
