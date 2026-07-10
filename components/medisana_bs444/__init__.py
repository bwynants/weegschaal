import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import (
    ble_client,
    time,
)
from esphome.const import (
    CONF_ID,
    CONF_TIME_ID,
    CONF_TRIGGER_ID,
)

CONF_TIME_OFFSET = "timeoffset"
CONF_ON_USER_METRICS_UPDATED = "on_user_metrics_updated"

AUTO_LOAD = [
    "sensor", "binary_sensor"
]

CODEOWNERS = ["@bwynants"]

DEPENDENCIES = ["esp32", "ble_client", "time"]

MULTI_CONF = True

CONF_MedisanaBS444_ID = "medisana_bs444_id"

# number of users supported by the scale; must match the [8] array sizes in medisanabs444.h
MAX_USERS = 8


medisana_bs444_ns = cg.esphome_ns.namespace("medisana_bs444")
MedisanaBS444 = medisana_bs444_ns.class_(
    "MedisanaBS444", ble_client.BLEClientNode, cg.Component
)

UserMeasurement = medisana_bs444_ns.struct("UserMeasurement")
UserMeasurementConstRef = UserMeasurement.operator("ref").operator("const")
UserMetricsUpdatedTrigger = medisana_bs444_ns.class_(
    "UserMetricsUpdatedTrigger", automation.Trigger.template(UserMeasurementConstRef)
)

CONFIG_SCHEMA = (
    ble_client.BLE_CLIENT_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(MedisanaBS444),
            cv.GenerateID(CONF_TIME_ID): cv.use_id(time.RealTimeClock),
            cv.Optional(CONF_TIME_OFFSET, default=True): cv.boolean,
            cv.Optional(CONF_ON_USER_METRICS_UPDATED): automation.validate_automation(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(UserMetricsUpdatedTrigger),
                }
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await ble_client.register_ble_node(var, config)
    if CONF_TIME_ID in config:
        time_ = await cg.get_variable(config[CONF_TIME_ID])
        cg.add(var.set_time_id(time_))
    cg.add(var.use_timeoffset(config[CONF_TIME_OFFSET]))

    for conf in config.get(CONF_ON_USER_METRICS_UPDATED, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trigger, [(UserMeasurementConstRef, "x")], conf)

