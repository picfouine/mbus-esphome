@echo off
set targetDir=z:\esphome

REM YML file
set ymlFile=.\warmte-meter-mbus.yml
copy %ymlFile% %targetDir%

REM Component files
set customComponentsSrcDir=.\custom-components\warmtemetermbus
set customComponentsTargetDir=%targetDir%\custom_components\warmtemetermbus
copy %customComponentsSrcDir%\__init__.py %customComponentsTargetDir%
copy %customComponentsSrcDir%\HeatMeterMbus.cpp %customComponentsTargetDir%
copy %customComponentsSrcDir%\HeatMeterMbus.h %customComponentsTargetDir%
copy %customComponentsSrcDir%\Kamstrup303WA02.cpp %customComponentsTargetDir%
copy %customComponentsSrcDir%\Kamstrup303WA02.h %customComponentsTargetDir%
copy %customComponentsSrcDir%\Pwm.cpp %customComponentsTargetDir%
copy %customComponentsSrcDir%\Pwm.h %customComponentsTargetDir%
copy %customComponentsSrcDir%\sensor.py %customComponentsTargetDir%
