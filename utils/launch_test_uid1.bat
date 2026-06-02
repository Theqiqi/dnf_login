@echo off
title DNF Client - Static Token Direct Launch Bypass Test
echo 🚀 Launching game client directly bypassing login validation (Using static fallback token)...

:: 直接执行你的黄金命令行参数
"X:\Game\DNFTW\client\DNF.exe" Z3PQGRM08FQBbbqCq8D2MQcZ91PPHH8ccDnFXBGEfc/s08hbSu8P4D8BCfViRIKrNMUDB65ePdeZ5m3WyccLjkXArhEZV1pC+YM/w5+ynDDzjdL+23TH79q3taU9WB6iY3GzwbAHhint8YBVyIQn5B8Ccm09odcohfVVPPBZShfFLDUFnMvAcY+uZ4Cla6j5uNM5SDUjqhhcAql9X9vJxSRYsv54sW9qa0OdSaJasxd83QWu3RNFyaWcvb9m7KYpEUrT7wmT/fN8g4tTg4lEJCqn+vKQ/OWVCv0Dly8tcvI4manqObBJts4yoYfiUt7RSTgclLT5FP5+pQsMXcs/

if %ERRORLEVEL% equ 0 (
    echo ✅ Client initialised successfully.
) else (
    echo ❌ Failed to execute process. Verify if X:\Game\DNFTW\client\DNF.exe exists.
)
pause
