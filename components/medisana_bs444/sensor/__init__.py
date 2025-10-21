import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor

from esphome.const import (
    STATE_CLASS_MEASUREMENT,
    UNIT_KILOGRAM,
    UNIT_EMPTY,
    UNIT_PERCENT,
    UNIT_CENTIMETER,
    CONF_WEIGHT,
    CONF_SIZE,
    ICON_SCALE_BATHROOM,
    ICON_PERCENT,
    ICON_EMPTY,
    ICON_RULER,
    ICON_TIMELAPSE,
    DEVICE_CLASS_WEIGHT,
)
UNIT_KILOCALORIES="kcal"

CONF_BMI="bmi"
CONF_KILOCALORIES="kcal"
CONF_FAT="fat"
CONF_TBW="tbw"
CONF_MUSCLE="muscle"
CONF_BONE="bone"
CONF_AGE="age"

UNIT_AGE="y"

from .. import MedisanaBS444, medisana_bs444_ns, CONF_MedisanaBS444_ID

MEASUREMENTS = cv.Schema({
    });


# Generate schema for 8 persons
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
            cv.Optional("%s_%s" %(CONF_KILOCALORIES,x)): sensor.sensor_schema(
                unit_of_measurement=UNIT_KILOCALORIES,
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
                unit_of_measurement=UNIT_KILOGRAM,
                icon=ICON_SCALE_BATHROOM,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            # some data entered at the scale
            cv.Optional("%s_%s" %(CONF_AGE,x)): sensor.sensor_schema(
                unit_of_measurement=UNIT_AGE,
                icon=ICON_TIMELAPSE,
                accuracy_decimals=0,
            ),
            cv.Optional("%s_%s" %(CONF_SIZE,x)): sensor.sensor_schema(
                unit_of_measurement=UNIT_CENTIMETER,
                icon=ICON_RULER,
                accuracy_decimals=0,
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
        CONF_VAL = "%s_%s" %(CONF_WEIGHT,x)
        if CONF_VAL in config:
            sens = await sensor.new_sensor(config[CONF_VAL])
            cg.add(var.set_weight(x-1, sens))
        CONF_VAL = "%s_%s" %(CONF_BMI,x)
        if CONF_VAL in config:
            sens = await sensor.new_sensor(config[CONF_VAL])
            cg.add(var.set_bmi(x-1, sens))
        CONF_VAL = "%s_%s" %(CONF_KILOCALORIES,x)
        if CONF_VAL in config:
            sens = await sensor.new_sensor(config[CONF_VAL])
            cg.add(var.set_kcal(x-1, sens))
        CONF_VAL = "%s_%s" %(CONF_FAT,x)
        if CONF_VAL in config:
            sens = await sensor.new_sensor(config[CONF_VAL])
            cg.add(var.set_fat(x-1, sens))
        CONF_VAL = "%s_%s" %(CONF_TBW,x)
        if CONF_VAL in config:
            sens = await sensor.new_sensor(config[CONF_VAL])
            cg.add(var.set_tbw(x-1, sens))
        CONF_VAL = "%s_%s" %(CONF_MUSCLE,x)
        if CONF_VAL in config:
            sens = await sensor.new_sensor(config[CONF_VAL])
            cg.add(var.set_muscle(x-1, sens))
        CONF_VAL = "%s_%s" %(CONF_BONE,x)
        if CONF_VAL in config:
            sens = await sensor.new_sensor(config[CONF_VAL])
            cg.add(var.set_bone(x-1, sens))
        CONF_VAL = "%s_%s" %(CONF_AGE,x)
        if CONF_VAL in config:
            sens = await sensor.new_sensor(config[CONF_VAL])
            cg.add(var.set_age(x-1, sens))
        CONF_VAL = "%s_%s" %(CONF_SIZE,x)
        if CONF_VAL in config:
            sens = await sensor.new_sensor(config[CONF_VAL])
            cg.add(var.set_size(x-1, sens))
