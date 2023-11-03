import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID, UNIT_PASCAL, UNIT_PERCENT, ICON_RADIOACTIVE, STATE_CLASS_MEASUREMENT, STATE_CLASS_NONE

from .. import pressensor_ns, Pressensor

CONF_PRESSENSOR_ID = "pressensor_id"

CONF_PRESSURE = "pressure"
CONF_BATTERY_LEVEL = "battery_level"


PressensorSensor = pressensor_ns.class_(
    "PressensorSensor",
    cg.Component,
    cg.Parented.template(Pressensor),
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ID): cv.declare_id(PressensorSensor),
        cv.GenerateID(CONF_PRESSENSOR_ID): cv.use_id(Pressensor),
        cv.Optional(CONF_PRESSURE): sensor.sensor_schema(
            unit_of_measurement=UNIT_PASCAL,
            icon=ICON_RADIOACTIVE,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_BATTERY_LEVEL): sensor.sensor_schema(
            unit_of_measurement=UNIT_PERCENT,
            icon=ICON_RADIOACTIVE,
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
        ),

    }
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await cg.register_parented(var, config[CONF_PRESSENSOR_ID])

    if pressure_config := config.get(CONF_PRESSURE):
        sens = await sensor.new_sensor(pressure_config)
        cg.add(var.set_pressure(sens))

    if batt_config := config.get(CONF_BATTERY_LEVEL):
        sens = await sensor.new_sensor(batt_config)
        cg.add(var.set_battery_level(sens))