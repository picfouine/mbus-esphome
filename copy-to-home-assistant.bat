@echo off
set targetDir=z:\esphome

REM YML file
set ymlFile=.\warmte-meter-mbus.yaml
copy %ymlFile% %targetDir%

REM Component files
set SrcDir=.\lib\heatmeter_mbus
set customComponentsTargetDir=%targetDir%\custom_components\heatmeter_mbus

copy %SrcDir%\__init__.py %customComponentsTargetDir%
copy %SrcDir%\DataBlockReader.h %customComponentsTargetDir%
copy %SrcDir%\DataBlockReader.cpp %customComponentsTargetDir%
copy %SrcDir%\EspArduinoUartInterface.h %customComponentsTargetDir%
copy %SrcDir%\HeatMeterMbus.cpp %customComponentsTargetDir%
copy %SrcDir%\HeatMeterMbus.h %customComponentsTargetDir%
copy %SrcDir%\IMbusSensor.h %customComponentsTargetDir%
copy %SrcDir%\Kamstrup303WA02.cpp %customComponentsTargetDir%
copy %SrcDir%\Kamstrup303WA02.h %customComponentsTargetDir%
copy %SrcDir%\MbusSensor.cpp %customComponentsTargetDir%
copy %SrcDir%\MbusSensor.h %customComponentsTargetDir%
copy %SrcDir%\Pwm.cpp %customComponentsTargetDir%
copy %SrcDir%\Pwm.h %customComponentsTargetDir%
copy %SrcDir%\sensor.py %customComponentsTargetDir%
copy %SrcDir%\UartInterface.h %customComponentsTargetDir%
