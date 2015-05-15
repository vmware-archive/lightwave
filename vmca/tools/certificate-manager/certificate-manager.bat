@echo off
REM chomps off .bat and runs a python script with the same name is this .bat file
set dirpath=%~dp0
set fname=%~n0

IF "%VMWARE_CIS_HOME%"=="" (
ECHO VMWARE_CIS_HOME is not defined
EXIT /B
)

IF "%VMWARE_PYTHON_BIN%"=="" (
ECHO VMWARE_PYTHON_BIN is not defined
EXIT /B
)


set PYTHONPATH=%VMWARE_CIS_HOME%\python-modules;%VMWARE_CIS_HOME%\vpxd\py
"%VMWARE_PYTHON_BIN%" "%dirpath%\%fname%" %*
