@echo off



signtools\signtool sign /ac MSCV-GlobalSign.cer /s my /n "Light Speed Vision (Beijing) Co.,Ltd" /t http://timestamp.verisign.com/scripts/timestamp.dll Output\*


signtools\signTool verify /v /kp Output\*

pause