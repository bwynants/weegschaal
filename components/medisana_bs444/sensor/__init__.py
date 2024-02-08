import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor

from esphome.const import (
    STATE_CLASS_MEASUREMENT,
    UNIT_KILOGRAM,
    UNIT_EMPTY,
    UNIT_PERCENT,
    CONF_ID,
    CONF_WEIGHT,
    ICON_SCALE_BATHROOM,
    ICON_PERCENT,
    ICON_EMPTY,
    DEVICE_CLASS_WEIGHT,
)

from .. import MedisanaBS444, medisana_bs444_ns, CONF_MedisanaBS444_ID

UNIT_KILOCALORIERS="kcal"

CONF_BMI="bmi"
CONF_KILOCALORIERS="kcal"
CONF_FAT="fat"
CONF_TBW="tbw"
CONF_MUSCLE="muscle"
CONF_BONE="bone"
CONF_TIME_OFFSET = "timeoffset"

# Generate schema for 8 persons
MEASUREMENTS = cv.Schema({

    });

for x in range(1, 8):
    MEASUREMENTS = MEASUREMENTS.extend(
       cv.Schema(
        {
            cv.Optional("%s_%s" %(CONF_WEIGHT,x)): sensor.sensor_schema(
                unit_of_measurement=UNIT_KILOGRAM,
                icon=ICON_SCALE_BATHROOM,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_WEIGHT,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional("%s_%s" %(CONF_BMI,x)): sensor.sensor_schema(
                unit_of_measurement=UNIT_EMPTY,
                icon=ICON_SCALE_BATHROOM,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional("%s_%s" %(CONF_BMI,x)): sensor.sensor_schema(
                unit_of_measurement=UNIT_EMPTY,
                icon=ICON_SCALE_BATHROOM,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional("%s_%s" %(CONF_KILOCALORIERS,x)): sensor.sensor_schema(
                unit_of_measurement=UNIT_KILOCALORIERS,
                icon=ICON_EMPTY,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional("%s_%s" %(CONF_FAT,x)): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                icon=ICON_PERCENT,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional("%s_%s" %(CONF_TBW,x)): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                icon=ICON_PERCENT,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional("%s_%s" %(CONF_MUSCLE,x)): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                icon=ICON_PERCENT,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional("%s_%s" %(CONF_BONE,x)): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                icon=ICON_PERCENT,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
        )
    )

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(CONF_MedisanaBS444_ID): cv.use_id(MedisanaBS444),
        }
    )
    .extend(MEASUREMENTS)
    .extend(cv.COMPONENT_SCHEMA)
)

async def to_code(config):
    hub = await cg.get_variable(config[CONF_MedisanaBS444_ID])

    for x in range(1, 8):
        CONF_VAL = "%s_%s" %(CONF_WEIGHT,x)
        if CONF_VAL in config:
            sens = await sensor.new_sensor(config[CONF_VAL])
            cg.add(hub.set_weight(x-1, sens))
        CONF_VAL = "%s_%s" %(CONF_BMI,x)
        if CONF_VAL in config:
            sens = await sensor.new_sensor(config[CONF_VAL])
            cg.add(hub.set_bmi(x-1, sens))
        CONF_VAL = "%s_%s" %(CONF_KILOCALORIERS,x)
        if CONF_VAL in config:
            sens = await sensor.new_sensor(config[CONF_VAL])
            cg.add(hub.set_kcal(x-1, sens))
        CONF_VAL = "%s_%s" %(CONF_FAT,x)
        if CONF_VAL in config:
            sens = await sensor.new_sensor(config[CONF_VAL])
            cg.add(hub.set_fat(x-1, sens))
        CONF_VAL = "%s_%s" %(CONF_TBW,x)
        if CONF_VAL in config:
            sens = await sensor.new_sensor(config[CONF_VAL])
            cg.add(hub.set_tbw(x-1, sens))
        CONF_VAL = "%s_%s" %(CONF_MUSCLE,x)
        if CONF_VAL in config:
            sens = await sensor.new_sensor(config[CONF_VAL])
            cg.add(hub.set_muscle(x-1, sens))
        CONF_VAL = "%s_%s" %(CONF_BONE,x)
        if CONF_VAL in config:
            sens = await sensor.new_sensor(config[CONF_VAL])
            cg.add(hub.set_bone(x-1, sens))
