@echo off
set targetDir=z:\esphome

REM YML file
set ymlFile=.\warmte-meter-mbus.yaml
copy %ymlFile% %targetDir%

REM Component files
set customComponentsSrcDir=.\custom-components\warmtemetermbus
set customComponentsTargetDir=%targetDir%\custom_components\warmte-meter-mbus
copy %customComponentsSrcDir%\__init__.py %customComponentsTargetDir%
REM copy %customComponentsSrcDir%\Adc.cpp %customComponentsTargetDir%
REM copy %customComponentsSrcDir%\Adc.h %customComponentsTargetDir%
copy %customComponentsSrcDir%\HeatMeterMbus.cpp %customComponentsTargetDir%
copy %customComponentsSrcDir%\HeatMeterMbus.h %customComponentsTargetDir%
copy %customComponentsSrcDir%\Kamstrup303WA02.cpp %customComponentsTargetDir%
copy %customComponentsSrcDir%\Kamstrup303WA02.h %customComponentsTargetDir%
copy %customComponentsSrcDir%\Pwm.cpp %customComponentsTargetDir%
copy %customComponentsSrcDir%\Pwm.h %customComponentsTargetDir%
copy %customComponentsSrcDir%\sensor.py %customComponentsTargetDir%
