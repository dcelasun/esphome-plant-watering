import esphome.codegen as cg
from esphome.components import i2c
import esphome.config_validation as cv
from esphome.const import CONF_ID

CODEOWNERS = ["@dcelasun"]
DEPENDENCIES = ["i2c"]

CONF_ALLOW_SIMULTANEOUS_PUMPS = "allow_simultaneous_pumps"
CONF_PBHUB_ID = "pbhub_id"

pbhub_ns = cg.esphome_ns.namespace("pbhub")
PbHubComponent = pbhub_ns.class_("PbHubComponent", cg.Component, i2c.I2CDevice)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(PbHubComponent),
            cv.Optional(CONF_ALLOW_SIMULTANEOUS_PUMPS, default=False): cv.boolean,
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(i2c.i2c_device_schema(0x61))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_allow_simultaneous_pumps(config[CONF_ALLOW_SIMULTANEOUS_PUMPS]))
