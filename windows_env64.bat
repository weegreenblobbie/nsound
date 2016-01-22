@echo off
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::
:: $Id: windows_env64.bat 797 2013-12-25 03:40:26Z weegreenblobbie $
::
:: Windows DOS box evironment setup.
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

set VS_DIR=C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC
set PYTHON_DIR=C:\Python27
set SCONS_DIR=%PYTHON_DIR%\Scripts
set SWIG_DIR=C:\swigwin-2.0.11

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: Vistual Studio setup

:VS_SETUP

    pushd "%VS_DIR%"

    if %ERRORLEVEL% neq 0 goto VC_DIR_NOT_FOUND
    if not exist vcvarsall.bat goto VC_BAT_NOT_FOUND

    echo Adding Visual Studio Environment for x86_amd64
    call vcvarsall.bat x86_amd64

    popd

    goto PYTHON_SETUP

:VC_DIR_NOT_FOUND

    echo.
    echo ERROR: Could not find directory VS_DIR=%VS_DIR%
    goto FAILURE

:VC_BAT_NOT_FOUND

    echo.
    echo ERROR: Could not find file vcvarsall.bat in "%VS_DIR%"
    echo WTF?
    goto FAILURE

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: Python setup

:PYTHON_SETUP

    :: If you are using VS11+ (2012+), fool Python distutils and scons into
    :: using it.

    set VS110COMNTOOLS=%VS110COMNTOOLS%
    set VS100COMNTOOLS=%VS110COMNTOOLS%
    set VS90COMNTOOLS=%VS110COMNTOOLS%

    pushd "%PYTHON_DIR%"

    if %ERRORLEVEL% neq 0 goto PYTHON_DIR_NOT_FOUND

    if not exist python.exe goto PYTHON_EXE_NOT_FOUND

    echo Adding %PYTHON_DIR% to the path
    set PATH=%PYTHON_DIR%;%PATH%

    popd

    goto SCONS_SETUP

:PYTHON_DIR_NOT_FOUND

    echo.
    echo ERROR: Could not find directory PYTHON_DIR=%PYTHON_DIR%
    goto FAILURE

:PYTHON_EXE_NOT_FOUND

    echo.
    echo ERROR: Could not find python.exe in %PYTHON_DIR%
    echo WTF?
    goto FAILURE

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: SCONS setup

:SCONS_SETUP

    pushd "%SCONS_DIR%"

    if %ERRORLEVEL% neq 0 goto SCONS_DIR_NOT_FOUND

    if not exist scons.bat goto SCONS_BAT_NOT_FOUND

    echo Adding %SCONS_DIR% to path
    set PATH=%SCONS_DIR%;%PATH%

    popd
    goto SWIG_SETUP

:SCONS_DIR_NOT_FOUND

    echo.
    echo ERROR: Could not find directory SCONS_DIR=%SCONS_DIR%
    goto FAILURE

:SCONS_BAT_NOT_FOUND

    echo.
    echo ERROR: Could not find scons.bat in %SCONS_DIR%
    echo WTF?
    goto FAILURE

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: SWIG setup

:SWIG_SETUP

    pushd "%SWIG_DIR%"

    if %ERRORLEVEL% neq 0 goto SWIG_DIR_NOT_FOUND

    if not exist swig.exe goto SWIG_EXE_NOT_FOUND

    echo Adding %SWIG_DIR% to path
    set PATH=%SWIG_DIR%;%PATH%

    popd
    goto SUCCESS

:SWIG_DIR_NOT_FOUND

    echo.
    echo ERROR: Could not find directory SWIG_DIR=%SWIG_DIR%
    goto FAILURE

:SWIG_EXE_NOT_FOUND

    echo.
    echo ERROR: Could not find swig.exe in %SWIG_DIR%
    echo WTF?
    goto FAILURE

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: exits

:SUCCESS
    goto ALL_DONE

:FAILURE
    echo.
    echo FAILURE
    popd

:ALL_DONE

