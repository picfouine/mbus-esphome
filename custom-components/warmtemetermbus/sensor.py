import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.components import uart
from esphome.const import  (
    CONF_ID, 
    DEVICE_CLASS_TEMPERATURE,
    ICON_THERMOMETER,
    UNIT_CELSIUS, 
    STATE_CLASS_MEASUREMENT,
)

DEPENDENCIES = ["uart"]

heatmetermbus_ns = cg.esphome_ns.namespace("warmtemetermbus")
heatmetermbus = heatmetermbus_ns.class_("HeatMeterMbus", cg.PollingComponent, uart.UARTDevice)

CONF_TEST_TEMPERATURE = "test_temperature"
CONF_INFO_V1_AIR = "info_v1_air"

SENSORS = {
    CONF_TEST_TEMPERATURE: sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=2,
        device_class=DEVICE_CLASS_TEMPERATURE,
        state_class=STATE_CLASS_MEASUREMENT,
        icon=ICON_THERMOMETER
    )
}


CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(heatmetermbus)
        }
    )
    .extend(
        {
            cv.Optional(sensor_name): schema for sensor_name, schema in SENSORS.items()
        }
    ).extend(uart.UART_DEVICE_SCHEMA)
)

async def to_code(config):
    cg.add_global(heatmetermbus_ns.using)
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    for name in SENSORS:
        if name in config:
            sens = await sensor.new_sensor(config[name])
            cg.add(getattr(var, f"set_{name}_sensor")(sens))
