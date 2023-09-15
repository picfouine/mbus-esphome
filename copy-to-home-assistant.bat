@echo off
set targetDir=z:\esphome

REM YML file
set ymlFile=.\warmte-meter-mbus.yaml
copy %ymlFile% %targetDir%

REM Component files
set SrcDir=.\lib\mbus_controller
set customComponentsTargetDir=%targetDir%\custom_components\mbus_controller

copy %SrcDir%\__init__.py %customComponentsTargetDir%
copy %SrcDir%\data_block_reader.h %customComponentsTargetDir%
copy %SrcDir%\data_block_reader.cpp %customComponentsTargetDir%
copy %SrcDir%\esp32_arduino_uart_interface.h %customComponentsTargetDir%
copy %SrcDir%\i_mbus_sensor.h %customComponentsTargetDir%
copy %SrcDir%\mbus_reader.cpp %customComponentsTargetDir%
copy %SrcDir%\mbus_reader.h %customComponentsTargetDir%
copy %SrcDir%\mbus_sensor.cpp %customComponentsTargetDir%
copy %SrcDir%\mbus_sensor.h %customComponentsTargetDir%
copy %SrcDir%\mbus_controller.cpp %customComponentsTargetDir%
copy %SrcDir%\mbus_controller.h %customComponentsTargetDir%
copy %SrcDir%\pwm.cpp %customComponentsTargetDir%
copy %SrcDir%\pwm.h %customComponentsTargetDir%
copy %SrcDir%\sensor.py %customComponentsTargetDir%
copy %SrcDir%\uart_interface.h %customComponentsTargetDir%
