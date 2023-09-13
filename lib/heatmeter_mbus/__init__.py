import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome.components import uart

DEPENDENCIES = ["uart"]
MULTI_CONF = True
warmtemetermbus_ns = cg.esphome_ns.namespace('warmtemetermbus')

HeatMeterMbus = warmtemetermbus_ns.class_('HeatMeterMbus', cg.Component, uart.UARTDevice)

CONFIG_SCHEMA = cv.Schema({
  cv.GenerateID(): cv.declare_id(HeatMeterMbus)
}).extend(cv.COMPONENT_SCHEMA).extend(uart.UART_DEVICE_SCHEMA)

async def to_code(config):
  rhs = HeatMeterMbus.new()
  var = cg.Pvariable(config[CONF_ID], rhs)
  await cg.register_component(var, config)
  await uart.register_uart_device(var, config)