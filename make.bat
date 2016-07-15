@ECHO OFF

:optloop
IF NOT "%1"=="" (
  IF "%1"=="help" (
    GOTO help
  )
  IF "%1"=="CFG" (
    SET CFG=%2
    SHIFT
  )
  IF "%1"=="distclean" (
    GOTO distclean
  )
  SHIFT
  GOTO optloop
)

IF "%CFG%"=="Release" (
  SET CMAKE_BUILD_TYPE=Release
) else (
  SET CMAKE_BUILD_TYPE=Debug
)

REM Load Visual Studio environment
setlocal enabledelayedexpansion
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\Tools\VsDevCmd.bat" x86

REM Establish BUILD_DIR variable and directory
SET BUILD_DIR=%CMAKE_BUILD_TYPE%-Host
IF EXIST %BUILD_DIR%\NUL GOTO generate
mkdir %BUILD_DIR%

REM Generate NMake Makefiles for the project
:generate
CD %BUILD_DIR%
IF EXIST Makefile GOTO build
cmake .. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=%CMAKE_BUILD_TYPE%
GOTO build

REM Build the project using the NMake Makefiles generated above
:build
nmake
GOTO end

:distclean
rmdir /s /q Debug-Host 2>NUL
rmdir /s /q Release-Host 2>NUL
GOTO end

:help
ECHO Usage: %~n0 [CFG ^<Release^|Debug^> ^| distclean ^| help]
ECHO.
ECHO Arguments:
ECHO CFG - used to specify build type Debug or Release (defaults to Debug)
ECHO distclean - used to remove Debug and Release directories for publishing
ECHO help - used to see this usage message
ECHO.
ECHO Description:
ECHO Uses CMake to generate NMake Makefiles and then uses nmake to build the
ECHO project.
ECHO.
GOTO end

:end
