import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, binary_sensor, ble_client, time

from esphome.const import (
    STATE_CLASS_MEASUREMENT,
    UNIT_KILOGRAM,
    UNIT_EMPTY,
    UNIT_PERCENT,
    UNIT_CENTIMETER,
    CONF_ID,
    CONF_WEIGHT,
    CONF_SIZE,
    CONF_TIME_ID,
    ICON_SCALE_BATHROOM,
    ICON_PERCENT,
    ICON_EMPTY,
    ICON_RULER,
    ICON_TIMELAPSE,
    DEVICE_CLASS_WEIGHT,
)

UNIT_KILOCALORIERS="kcal"

CONF_BMI="bmi"
CONF_KILOCALORIERS="kcal"
CONF_FAT="fat"
CONF_TBW="tbw"
CONF_MUSCLE="muscle"
CONF_BONE="bone"
CONF_MALE="male"
CONF_FEMALE="female"
CONF_AGE="age"
CONF_HIGHACTIVITY="highactivity"
CONF_TIME_OFFSET = "timeoffset"

ICON_MALE="mdi:gender-male"
ICON_FEMALE="mdi:gender-female"

UNIT_AGE="y"

DEPENDENCIES = ["esp32", "ble_client", "time"]

medisana_bs444_ns = cg.esphome_ns.namespace("medisana_bs444")
MedisanaBS444 = medisana_bs444_ns.class_(
    "MedisanaBS444", cg.Component, ble_client.BLEClientNode
)

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
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(MedisanaBS444),
            cv.GenerateID(CONF_TIME_ID): cv.use_id(time.RealTimeClock),
            cv.Optional(CONF_TIME_OFFSET, default=True): cv.boolean,
        }
    )
    .extend(MEASUREMENTS)
    .extend(cv.COMPONENT_SCHEMA)
    .extend(ble_client.BLE_CLIENT_SCHEMA),
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await ble_client.register_ble_node(var, config)
    if CONF_TIME_ID in config:
        time_ = await cg.get_variable(config[CONF_TIME_ID])
        cg.add(var.set_time_id(time_))
    cg.add(var.use_timeoffset(config[CONF_TIME_OFFSET]))

    for x in range(1, 8):
        CONF_VAL = "%s_%s" %(CONF_WEIGHT,x)
        if CONF_VAL in config:
            sens = await sensor.new_sensor(config[CONF_VAL])
            cg.add(var.set_weight(x-1, sens))
        CONF_VAL = "%s_%s" %(CONF_BMI,x)
        if CONF_VAL in config:
            sens = await sensor.new_sensor(config[CONF_VAL])
            cg.add(var.set_bmi(x-1, sens))
        CONF_VAL = "%s_%s" %(CONF_KILOCALORIERS,x)
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
