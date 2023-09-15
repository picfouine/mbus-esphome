import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID
from . import MbusController, mbus_controller_ns

DEPENDENCIES = ["mbus_controller"]

MbusSensor = mbus_controller_ns.class_('MbusSensor', sensor.Sensor)
CONF_MBUS_CONTROLLER_ID = 'mbus_controller_id'
CONF_INDEX = 'index'

CONFIG_SCHEMA = sensor.sensor_schema(MbusSensor).extend({
    cv.GenerateID(CONF_MBUS_CONTROLLER_ID): cv.use_id(MbusController),
    cv.Required(CONF_INDEX): int
})

async def to_code(config):
    paren = await cg.get_variable(config[CONF_MBUS_CONTROLLER_ID])
    rhs = paren.create_sensor(config[CONF_INDEX])
    var = cg.Pvariable(config[CONF_ID], rhs)
    await sensor.register_sensor(var, config)