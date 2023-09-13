import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID
from . import HeatMeterMbus, warmtemetermbus_ns

DEPENDENCIES = ["heatmeter_mbus"]

MbusSensor = warmtemetermbus_ns.class_('MbusSensor', sensor.Sensor)
CONF_HEATMETERMBUS_ID = 'heatmeter_mbus_id'
CONF_INDEX = 'index'

CONFIG_SCHEMA = sensor.sensor_schema(MbusSensor).extend({
    cv.GenerateID(CONF_HEATMETERMBUS_ID): cv.use_id(HeatMeterMbus),
    cv.Required(CONF_INDEX): int
})

async def to_code(config):
    paren = await cg.get_variable(config[CONF_HEATMETERMBUS_ID])
    rhs = paren.create_sensor(config[CONF_INDEX])
    var = cg.Pvariable(config[CONF_ID], rhs)
    await sensor.register_sensor(var, config)