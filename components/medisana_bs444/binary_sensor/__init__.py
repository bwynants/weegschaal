import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor

from esphome.const import (
    ICON_EMPTY,
)

CONF_MALE="male"
CONF_FEMALE="female"
CONF_HIGHACTIVITY="highactivity"

ICON_MALE="mdi:gender-male"
ICON_FEMALE="mdi:gender-female"

from .. import MedisanaBS444, medisana_bs444_ns, CONF_MedisanaBS444_ID


MEASUREMENTS = cv.Schema({
    });


# Generate schema for 8 persons
for x in range(1, 8):
    MEASUREMENTS = MEASUREMENTS.extend(
        cv.Schema(
        {
            cv.Optional("%s_%s" %(CONF_MALE,x)): binary_sensor.binary_sensor_schema(
                icon=ICON_MALE,
            ),
            cv.Optional("%s_%s" %(CONF_FEMALE,x)): binary_sensor.binary_sensor_schema(
                icon=ICON_FEMALE,
            ),
            cv.Optional("%s_%s" %(CONF_HIGHACTIVITY,x)): binary_sensor.binary_sensor_schema(
                icon=ICON_EMPTY,
            ),
        }
        )
    )

CONFIG_SCHEMA = cv.All(
        cv.Schema({
            cv.GenerateID(CONF_MedisanaBS444_ID): cv.use_id(MedisanaBS444),
        }
    )
    .extend(MEASUREMENTS)
    .extend(cv.COMPONENT_SCHEMA).extend()
)

async def to_code(config):
    var = await cg.get_variable(config[CONF_MedisanaBS444_ID])
    for x in range(1, 8):
        CONF_VAL = "%s_%s" %(CONF_MALE,x)
        if CONF_VAL in config:
            sens = await binary_sensor.new_binary_sensor(config[CONF_VAL])
            cg.add(var.set_male(x-1, sens))
        CONF_VAL = "%s_%s" %(CONF_FEMALE,x)
        if CONF_VAL in config:
            sens = await binary_sensor.new_binary_sensor(config[CONF_VAL])
            cg.add(var.set_female(x-1, sens))
        CONF_VAL = "%s_%s" %(CONF_HIGHACTIVITY,x)
        if CONF_VAL in config:
            sens = await binary_sensor.new_binary_sensor(config[CONF_VAL])
            cg.add(var.set_high_activity(x-1, sens))
