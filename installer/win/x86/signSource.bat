@echo off



signtools\signtool sign /ac MSCV-GlobalSign.cer /s my /n "Light Speed Vision (Beijing) Co.,Ltd" /t http://timestamp.verisign.com/scripts/timestamp.dll {source}\EZCAP_Qt\*


signtools\signTool verify /v /kp {source}\EZCAP_Qt\*

pause