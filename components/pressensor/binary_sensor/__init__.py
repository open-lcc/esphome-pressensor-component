import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ID, UNIT_CELSIUS, UNIT_SECOND, ICON_RADIOACTIVE, STATE_CLASS_MEASUREMENT, STATE_CLASS_NONE

from .. import pressensor_ns, Pressensor

CONF_PRESSENSOR_ID = "pressensor_id"

CONF_CONNECTED = "connected"

PressensorBinarySensor = pressensor_ns.class_(
    "PressensorBinarySensor",
    cg.Component,
    cg.Parented.template(Pressensor),
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ID): cv.declare_id(PressensorBinarySensor),
        cv.GenerateID(CONF_PRESSENSOR_ID): cv.use_id(Pressensor),
        cv.Optional(CONF_CONNECTED): binary_sensor.binary_sensor_schema(
            icon=ICON_RADIOACTIVE,
        ),
    }
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await cg.register_parented(var, config[CONF_PRESSENSOR_ID])

    if connected_config := config.get(CONF_CONNECTED):
        sens = await binary_sensor.new_binary_sensor(connected_config)
        cg.add(var.set_connected(sens))
