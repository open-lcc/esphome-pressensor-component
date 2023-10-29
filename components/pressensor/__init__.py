import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import ble_client
from esphome.const import (
    CONF_ID,
    CONF_TRIGGER_ID,
)

CODEOWNERS = ["@magnusnordlander"]
pressensor_ns = cg.esphome_ns.namespace("esphome::pressensor")

Pressensor = pressensor_ns.class_("Pressensor", cg.Component, ble_client.BLEClientNode)

CONF_ACAIA_ID = "pressensor_id"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(Pressensor),
    }
).extend(ble_client.BLE_CLIENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await ble_client.register_ble_node(var, config)