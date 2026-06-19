@echo off
setlocal

cd /d "%~dp0.."

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
set GIT_SSL_NO_VERIFY=1

cmake --preset debug
cmake --build --preset debug --parallel

endlocal
rem pause
