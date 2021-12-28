@echo off
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::
:: $Id: windows_env64.bat 797 2013-12-25 03:40:26Z weegreenblobbie $
::
:: Windows DOS box evironment setup.
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

set VS_DIR=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build
set PYTHON_DIR=C:\Python310
set PYTHON_DLL_DIR=C:\Python310
set SCONS_DIR=%PYTHON_DIR%\Scripts
set SWIG_DIR=C:\Users\weegr\bin
set NSOUND_DIR=C:\Users\weegr\code\nsound

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

    pushd "%PYTHON_DIR%"

    if %ERRORLEVEL% neq 0 goto PYTHON_DIR_NOT_FOUND

    if not exist python.exe goto PYTHON_EXE_NOT_FOUND

    popd

    pushd "%PYTHON_DLL_DIR%"

    if %ERRORLEVEL% neq 0 goto PYTHON_DLL_DIR_NOT_FOUND

    if not exist python*.dll goto PYTHON_DLL_NOT_FOUND

:PYTHON_OK

    echo Adding %PYTHON_DIR% to the path
    set PATH=%PYTHON_DIR%;%PYTHON_DLL_DIR%;%PATH%
    echo Adding %PYTHON_DIR%\Scripts to the path
    set PATH=%PYTHON_DIR%\Scripts;%PATH%

    popd

    pushd "%PYTHON_DIR%"

    if not exist Scripts\activate.bat goto SCONS_SETUP

    call Scripts\activate.bat

    popd

    echo Setting PYTHONPATH="%VIRTUAL_ENV%\Lib\site-packages"

    set PYTHONPATH=%VIRTUAL_ENV%\Lib\site-packages
    set PATH=%PYTHON_DIR%;%PYTHON_DLL_DIR%;%PATH%

    goto SCONS_SETUP

:PYTHON_DIR_NOT_FOUND

    echo.
    echo ERROR: Could not find directory PYTHON_DIR=%PYTHON_DIR%
    goto FAILURE

:PYTHON_EXE_NOT_FOUND

    pushd "%PYTHON_DIR%\Scripts"

    if %ERRORLEVEL% equ 0 goto PYTHON_OK

    echo.
    echo ERROR: Could not find python.exe in %PYTHON_DIR%
    echo WTF?
    goto FAILURE

:PYTHON_DLL_NOT_FOUND

    echo.
    echo ERROR: Could not find python*.dll in %PYTHON_DLL_DIR%
    echo WTF?
    goto FAILURE

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: SCONS setup

:SCONS_SETUP

    pushd "%SCONS_DIR%"

    if %ERRORLEVEL% neq 0 goto SCONS_DIR_NOT_FOUND

    if not exist scons.bat goto SCONS_BAT_NOT_FOUND

:SCONS_OK

    echo Adding %SCONS_DIR% to path
    set PATH=%SCONS_DIR%;%PATH%

    popd
    goto SWIG_SETUP

:SCONS_DIR_NOT_FOUND

    echo.
    echo ERROR: Could not find directory SCONS_DIR=%SCONS_DIR%
    goto FAILURE

:SCONS_BAT_NOT_FOUND

    if exist scons.exe goto SCONS_OK

    echo.
    echo ERROR: Could not find scons.bat or scons.exe in %SCONS_DIR%
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
    cd %NSOUND_DIR%

