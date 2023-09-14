@echo off
set targetDir=z:\esphome

REM YML file
set ymlFile=.\warmte-meter-mbus.yaml
copy %ymlFile% %targetDir%

REM Component files
set SrcDir=.\lib\heatmeter_mbus
set customComponentsTargetDir=%targetDir%\custom_components\heatmeter_mbus

copy %SrcDir%\__init__.py %customComponentsTargetDir%
copy %SrcDir%\data_block_reader.h %customComponentsTargetDir%
copy %SrcDir%\data_block_reader.cpp %customComponentsTargetDir%
copy %SrcDir%\esp32_arduino_uart_interface.h %customComponentsTargetDir%
copy %SrcDir%\heat_meter_mbus.cpp %customComponentsTargetDir%
copy %SrcDir%\heat_meter_mbus.h %customComponentsTargetDir%
copy %SrcDir%\i_mbus_sensor.h %customComponentsTargetDir%
copy %SrcDir%\kamstrup303wa02.cpp %customComponentsTargetDir%
copy %SrcDir%\kamstrup303wa02.h %customComponentsTargetDir%
copy %SrcDir%\mbus_sensor.cpp %customComponentsTargetDir%
copy %SrcDir%\mbus_sensor.h %customComponentsTargetDir%
copy %SrcDir%\pwm.cpp %customComponentsTargetDir%
copy %SrcDir%\pwm.h %customComponentsTargetDir%
copy %SrcDir%\sensor.py %customComponentsTargetDir%
copy %SrcDir%\uart_interface.h %customComponentsTargetDir%
