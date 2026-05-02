import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_EMPTY,
    STATE_CLASS_MEASUREMENT,
)

from . import CONF_PBHUB_ID, PbHubComponent, pbhub_ns

CONF_CHANNEL = "channel"

DEPENDENCIES = ["pbhub"]

PbHubSensor = pbhub_ns.class_("PbHubSensor", sensor.Sensor, cg.PollingComponent)

CONFIG_SCHEMA = (
    sensor.sensor_schema(
        PbHubSensor,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_EMPTY,
        state_class=STATE_CLASS_MEASUREMENT,
    )
    .extend(cv.polling_component_schema("30s"))
    .extend(
        {
            cv.GenerateID(CONF_PBHUB_ID): cv.use_id(PbHubComponent),
            cv.Required(CONF_CHANNEL): cv.int_range(min=0, max=5),
        }
    )
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await sensor.register_sensor(var, config)

    parent = await cg.get_variable(config[CONF_PBHUB_ID])
    cg.add(var.set_parent(parent))
    cg.add(var.set_channel(config[CONF_CHANNEL]))
