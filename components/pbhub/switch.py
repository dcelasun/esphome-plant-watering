import esphome.codegen as cg
from esphome.components import switch
import esphome.config_validation as cv
from esphome.const import CONF_ID

from . import CONF_PBHUB_ID, PbHubComponent, pbhub_ns

CONF_CHANNEL = "channel"
CONF_MAX_RUN_TIME = "max_run_time"
CONF_PIN_INDEX = "pin_index"

DEPENDENCIES = ["pbhub"]

PbHubSwitch = pbhub_ns.class_("PbHubSwitch", switch.Switch, cg.Component)

CONFIG_SCHEMA = (
    switch.switch_schema(
        PbHubSwitch,
        default_restore_mode="RESTORE_DEFAULT_OFF",
        icon="mdi:water-pump",
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(
        {
            cv.GenerateID(CONF_PBHUB_ID): cv.use_id(PbHubComponent),
            cv.Required(CONF_CHANNEL): cv.int_range(min=0, max=5),
            cv.Optional(CONF_PIN_INDEX, default=1): cv.int_range(min=0, max=1),
            cv.Optional(CONF_MAX_RUN_TIME, default="10s"): cv.positive_time_period_milliseconds,
        }
    )
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await switch.register_switch(var, config)

    parent = await cg.get_variable(config[CONF_PBHUB_ID])
    cg.add(var.set_parent(parent))
    cg.add(var.set_channel(config[CONF_CHANNEL]))
    cg.add(var.set_pin_index(config[CONF_PIN_INDEX]))
    cg.add(var.set_max_run_time(config[CONF_MAX_RUN_TIME]))
