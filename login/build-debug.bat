@echo off
setlocal
REM 初始化 VS 环境
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

REM 在项目根目录运行 cmake
cd /d C:\d\code\dnflogin

cmake --preset debug
cmake --build --preset debug --parallel
endlocal
pause
