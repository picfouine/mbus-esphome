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
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_VOLTAGE,
    DEVICE_CLASS_VOLUME,
    DEVICE_CLASS_WATER,
    ICON_THERMOMETER,
    ICON_WATER,
    UNIT_CELSIUS, 
    UNIT_KILOWATT,
    UNIT_KILOWATT_HOURS,
    UNIT_VOLT,
    STATE_CLASS_MEASUREMENT,
)

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["binary_sensor"]

heatmetermbus_ns = cg.esphome_ns.namespace("warmtemetermbus")
heatmetermbus = heatmetermbus_ns.class_("HeatMeterMbus", cg.Component, uart.UARTDevice)
binary_sensor_ns = cg.esphome_ns.namespace("binary_sensor")

CONF_HEAT_ENERGY_E1 = "heat_energy_e1"
CONF_VOLUME_V1 = "volume_v1"
CONF_ENERGY_E8_INLET = "energy_e8_inlet"
CONF_ENERGY_E9_OUTLET = "energy_e9_outlet"
CONF_OPERATING_HOURS = "operating_hours"
CONF_ERROR_HOUR_COUNTER = "error_hour_counter"
CONF_T1_ACTUAL = "t1_actual"
CONF_T2_ACTUAL = "t2_actual"
CONF_T1_MINUS_T2 = "t1_minus_t2"
CONF_POWER_E1_OVER_E3 = "power_e1_over_e3"
CONF_POWER_MAX_MONTH = "power_max_month"
CONF_FLOW_V1_ACTUAL = "flow_v1_actual"
CONF_FLOW_V1_MAX_MONTH = "flow_v1_max_month"
CONF_HEAT_ENERGY_E1_OLD = "heat_energy_e1_old"
CONF_VOLUME_V1_OLD = "volume_v1_old"
CONF_ENERGY_E8_INLET_OLD = "energy_e8_inlet_old"
CONF_ENERGY_E9_OUTLET_OLD = "energy_e9_outlet_old"
CONF_POWER_MAX_YEAR_OLD = "power_max_year_old"
CONF_FLOW_V1_MAX_YEAR_OLD = "flow_v1_max_year_old"
CONF_LOG_YEAR = "log_year"
CONF_LOG_MONTH = "log_month"
CONF_LOG_DAY = "log_day"
CONF_BUS_VOLTAGE = "bus_voltage"

CONF_INFO_NO_VOLTAGE_SUPPLY = "info_no_voltage_supply"
CONF_INFO_T1_ABOVE_RANGE_OR_DISCONNECTED = "info_t1_above_range_or_disconnected"
CONF_INFO_T2_ABOVE_RANGE_OR_DISCONNECTED = "info_t2_above_range_or_disconnected"
CONF_INFO_T1_BELOW_RANGE_OR_SHORTED = "info_t1_below_range_or_shorted"
CONF_INFO_T2_BELOW_RANGE_OR_SHORTED = "info_t2_below_range_or_shorted"
CONF_INFO_INVALID_TEMP_DIFFERENCE = "info_invalid_temp_difference"
CONF_INFO_V1_AIR = "info_v1_air"
CONF_INFO_V1_WRONG_FLOW_DIRECTION = "info_v1_wrong_flow_direction"
CONF_INFO_V1_GREATER_THAN_QS_MORE_THAN_HOUR = "info_v1_greater_than_qs_more_than_hour"

ICON_CLOCK_ALERT_OUTLINE = "mdi:clock-alert-outline"
ICON_CLOCK_OUTLINE = "mdi:clock-outline"
ICON_HEAT_WAVE = "mdi:heat-wave"
ICON_LIGHTNING_BOLT = "mdi:lightning-bolt"
ICON_THERMOMETER_MINUS = "mdi:thermometer-minus"
ICON_WATER_THERMOMETER_OUTLINE = "mdi:water-thermometer-outline"
ICON_WAVES_ARROW_RIGHT = "mdi:waves-arrow-right"

UNIT_CUBIC_METER_TIMES_DEG_CELCIUS = "m³ * °C"
UNIT_DAYS = "days"
UNIT_LITER = "l"
UNIT_LITER_PER_HOUR = "l/h"

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
        icon=ICON_WATER_THERMOMETER_OUTLINE
    ),
    CONF_T2_ACTUAL: sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=2,
        device_class=DEVICE_CLASS_TEMPERATURE,
        state_class=STATE_CLASS_MEASUREMENT,
        icon=ICON_WATER_THERMOMETER_OUTLINE
    ),
    CONF_T1_MINUS_T2: sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=2,
        device_class=DEVICE_CLASS_TEMPERATURE,
        state_class=STATE_CLASS_MEASUREMENT,
        icon=ICON_THERMOMETER_MINUS
    ),
    CONF_POWER_E1_OVER_E3: sensor.sensor_schema(
        unit_of_measurement=UNIT_KILOWATT,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_POWER,
        state_class=STATE_CLASS_MEASUREMENT,
        icon=ICON_HEAT_WAVE
    ),
    CONF_POWER_MAX_MONTH: sensor.sensor_schema(
        unit_of_measurement=UNIT_KILOWATT,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_POWER,
        state_class=STATE_CLASS_MEASUREMENT,
        icon=ICON_HEAT_WAVE
    ),
    CONF_FLOW_V1_ACTUAL: sensor.sensor_schema(
        unit_of_measurement=UNIT_LITER_PER_HOUR,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_WATER,
        state_class=STATE_CLASS_MEASUREMENT,
        icon=ICON_WAVES_ARROW_RIGHT
    ),
    CONF_FLOW_V1_MAX_MONTH: sensor.sensor_schema(
        unit_of_measurement=UNIT_LITER_PER_HOUR,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_WATER,
        state_class=STATE_CLASS_MEASUREMENT,
        icon=ICON_WAVES_ARROW_RIGHT
    ),
    CONF_HEAT_ENERGY_E1_OLD: sensor.sensor_schema(
        unit_of_measurement=UNIT_KILOWATT_HOURS,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_ENERGY,
        state_class=STATE_CLASS_MEASUREMENT,
        icon=ICON_HEAT_WAVE
    ),
    CONF_VOLUME_V1_OLD: sensor.sensor_schema(
        unit_of_measurement=UNIT_LITER,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_VOLUME,
        state_class=STATE_CLASS_MEASUREMENT,
        icon=ICON_WATER
    ),
    CONF_ENERGY_E8_INLET_OLD: sensor.sensor_schema(
        unit_of_measurement=UNIT_CUBIC_METER_TIMES_DEG_CELCIUS,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_ENERGY,#DEVICE_CLASS_HEAT,
        state_class=STATE_CLASS_MEASUREMENT,
        icon=ICON_HEAT_WAVE
    ),
    CONF_ENERGY_E9_OUTLET_OLD: sensor.sensor_schema(
        unit_of_measurement=UNIT_CUBIC_METER_TIMES_DEG_CELCIUS,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_ENERGY,#DEVICE_CLASS_HEAT,
        state_class=STATE_CLASS_MEASUREMENT,
        icon=ICON_HEAT_WAVE
    ), 
    CONF_POWER_MAX_YEAR_OLD: sensor.sensor_schema(
        unit_of_measurement=UNIT_KILOWATT,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_POWER,
        state_class=STATE_CLASS_MEASUREMENT,
        icon=ICON_HEAT_WAVE
    ),
    CONF_FLOW_V1_MAX_YEAR_OLD: sensor.sensor_schema(
        unit_of_measurement=UNIT_LITER_PER_HOUR,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_WATER,
        state_class=STATE_CLASS_MEASUREMENT,
        icon=ICON_WAVES_ARROW_RIGHT
    ),
    CONF_LOG_YEAR: sensor.sensor_schema(),
    CONF_LOG_MONTH: sensor.sensor_schema(),
    CONF_LOG_DAY: sensor.sensor_schema(),
    CONF_BUS_VOLTAGE: sensor.sensor_schema(
        unit_of_measurement=UNIT_VOLT,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_VOLTAGE,
        icon=ICON_LIGHTNING_BOLT
    )
}

BINARY_SENSORS = {
    CONF_INFO_V1_AIR: binary_sensor.BINARY_SENSOR_SCHEMA,
    CONF_INFO_NO_VOLTAGE_SUPPLY: binary_sensor.BINARY_SENSOR_SCHEMA,
    CONF_INFO_T1_ABOVE_RANGE_OR_DISCONNECTED: binary_sensor.BINARY_SENSOR_SCHEMA,
    CONF_INFO_T2_ABOVE_RANGE_OR_DISCONNECTED: binary_sensor.BINARY_SENSOR_SCHEMA,
    CONF_INFO_T1_BELOW_RANGE_OR_SHORTED: binary_sensor.BINARY_SENSOR_SCHEMA,
    CONF_INFO_T2_BELOW_RANGE_OR_SHORTED: binary_sensor.BINARY_SENSOR_SCHEMA,
    CONF_INFO_INVALID_TEMP_DIFFERENCE: binary_sensor.BINARY_SENSOR_SCHEMA,
    CONF_INFO_V1_AIR: binary_sensor.BINARY_SENSOR_SCHEMA,
    CONF_INFO_V1_WRONG_FLOW_DIRECTION: binary_sensor.BINARY_SENSOR_SCHEMA,
    CONF_INFO_V1_GREATER_THAN_QS_MORE_THAN_HOUR: binary_sensor.BINARY_SENSOR_SCHEMA
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
    )
    .extend(
        {
            cv.Optional(sensor_name): schema for sensor_name, schema in BINARY_SENSORS.items()
        }
    ).extend(uart.UART_DEVICE_SCHEMA)
)

async def to_code(config):
    cg.add_global(heatmetermbus_ns.using)
    cg.add_global(binary_sensor_ns.using)
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    for name in SENSORS:
        if name in config:
            sens = await sensor.new_sensor(config[name])
            cg.add(getattr(var, f"set_{name}_sensor")(sens))

    for name in BINARY_SENSORS:
        if name in config:
            sens = await binary_sensor.new_binary_sensor(config[name])
            cg.add(getattr(var, f"set_{name}_binary_sensor")(sens))
