import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome.components import uart

DEPENDENCIES = ["uart"]
MULTI_CONF = True
CODEOWNERS = ["@pdjong"]

mbus_controller_ns = cg.esphome_ns.namespace('mbus_controller')

MbusController = mbus_controller_ns.class_('MbusController', cg.Component, uart.UARTDevice)
CONF_ADDRESS = 'address'

CONFIG_SCHEMA = cv.Schema({
  cv.GenerateID(): cv.declare_id(MbusController),
  cv.Required(CONF_ADDRESS): int
}).extend(cv.COMPONENT_SCHEMA).extend(uart.UART_DEVICE_SCHEMA)

async def to_code(config):
  rhs = MbusController.new(config[CONF_ADDRESS])
  var = cg.Pvariable(config[CONF_ID], rhs)
  await cg.register_component(var, config)
  await uart.register_uart_device(var, config)