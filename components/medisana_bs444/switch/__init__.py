import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.components.switch import SwitchRestoreMode

from esphome.const import (
    ENTITY_CATEGORY_CONFIG,
    DEVICE_CLASS_SWITCH,
)

CONF_SCAN = "scan"

from .. import MedisanaBS444, medisana_bs444_ns, CONF_MedisanaBS444_ID

ScanSwitch = medisana_bs444_ns.class_("ScanSwitch", switch.Switch, cg.Component)

ScanSwitchRestoreMode = medisana_bs444_ns.enum("ScanSwitchRestoreMode")

CONFIG_SCHEMA = {
    cv.GenerateID(CONF_MedisanaBS444_ID): cv.use_id(MedisanaBS444),
    cv.Optional(CONF_SCAN): switch.switch_schema(
        ScanSwitch,
        device_class=DEVICE_CLASS_SWITCH,
        entity_category=ENTITY_CATEGORY_CONFIG,
    )
    .extend(cv.COMPONENT_SCHEMA),
}


async def to_code(config):
    if scan_config := config.get(CONF_SCAN):
        s = await switch.new_switch(scan_config)
        await cg.register_component(s, scan_config)

        hub = await cg.get_variable(config[CONF_MedisanaBS444_ID])
        cg.add(hub.set_scan_switch(s))
