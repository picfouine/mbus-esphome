import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.components import binary_sensor
from esphome.components import uart
from esphome.const import  (
    CONF_ID,
    DEVICE_CLASS_DURATION,
    DEVICE_CLASS_ENERGY,
    DEVICE_CLASS_HEAT,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_VOLUME,
    ICON_THERMOMETER,
    ICON_WATER,
    UNIT_CELSIUS, 
    UNIT_KILOWATT_HOURS,
    STATE_CLASS_MEASUREMENT,
)

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["binary_sensor"]

heatmetermbus_ns = cg.esphome_ns.namespace("warmtemetermbus")
heatmetermbus = heatmetermbus_ns.class_("HeatMeterMbus", cg.PollingComponent, uart.UARTDevice)
binary_sensor_ns = cg.esphome_ns.namespace("binary_sensor")

CONF_HEAT_ENERGY_E1 = "heat_energy_e1"
CONF_VOLUME_V1 = "volume_v1"
CONF_ENERGY_E8_INLET = "energy_e8_inlet"
CONF_ENERGY_E9_OUTLET = "energy_e9_outlet"
CONF_OPERATING_HOURS = "operating_hours"
CONF_ERROR_HOUR_COUNTER = "error_hour_counter"
CONF_T1_ACTUAL = "t1_actual"
CONF_INFO_V1_AIR = "info_v1_air"

ICON_HEAT_WAVE = "mdi:heat-wave"
ICON_CLOCK_ALERT_OUTLINE = "mdi:clock-alert-outline"
ICON_CLOCK_OUTLINE = "mdi:clock-outline"

UNIT_CUBIC_METER_TIMES_DEG_CELCIUS = "m³ * °C"
UNIT_DAYS = "days"
UNIT_LITER = "l"

SENSORS = {
    CONF_HEAT_ENERGY_E1: sensor.sensor_schema(
        unit_of_measurement=UNIT_KILOWATT_HOURS,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_ENERGY,
        state_class=STATE_CLASS_MEASUREMENT,
        icon=ICON_HEAT_WAVE
    ),
    CONF_VOLUME_V1: sensor.sensor_schema(
        unit_of_measurement=UNIT_LITER,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_VOLUME,
        state_class=STATE_CLASS_MEASUREMENT,
        icon=ICON_WATER
    ),
    CONF_ENERGY_E8_INLET: sensor.sensor_schema(
        unit_of_measurement=UNIT_CUBIC_METER_TIMES_DEG_CELCIUS,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_ENERGY,#DEVICE_CLASS_HEAT,
        state_class=STATE_CLASS_MEASUREMENT,
        icon=ICON_HEAT_WAVE
    ),
    CONF_ENERGY_E9_OUTLET: sensor.sensor_schema(
        unit_of_measurement=UNIT_CUBIC_METER_TIMES_DEG_CELCIUS,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_ENERGY,#DEVICE_CLASS_HEAT,
        state_class=STATE_CLASS_MEASUREMENT,
        icon=ICON_HEAT_WAVE
    ), 
    CONF_OPERATING_HOURS: sensor.sensor_schema(
        unit_of_measurement=UNIT_DAYS,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_DURATION,
        state_class=STATE_CLASS_MEASUREMENT,
        icon=ICON_CLOCK_OUTLINE
    ), 
    CONF_ERROR_HOUR_COUNTER: sensor.sensor_schema(
        unit_of_measurement=UNIT_DAYS,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_DURATION,
        state_class=STATE_CLASS_MEASUREMENT,
        icon=ICON_CLOCK_ALERT_OUTLINE
    ), 
    CONF_T1_ACTUAL: sensor.sensor_schema(
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
