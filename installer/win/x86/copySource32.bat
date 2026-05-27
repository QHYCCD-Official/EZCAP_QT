echo y | del "C:\SoftwareSVN\QHY_SVN_Root\EZCAP_QT\EZCAP_QT\installer\{source}\EZCAP_Qt\*"

echo y | del "C:\SoftwareSVN\QHY_SVN_Root\EZCAP_QT\build-EZCAP-Desktop_Qt_5_12_9_MinGW_32_bit-Release\release\log\*.log"

echo y | del "C:\SoftwareSVN\QHY_SVN_Root\EZCAP_QT\EZCAP_QT\installer\Output\*.exe"

copy "C:\SoftwareSVN\QHY_SVN_Root\EZCAP_QT\build-EZCAP-Desktop_Qt_5_12_9_MinGW_32_bit-Release\release\*.*" "C:\SoftwareSVN\QHY_SVN_Root\EZCAP_QT\EZCAP_QT\installer\{source}\EZCAP_Qt\"

echo y | del "C:\SoftwareSVN\QHY_SVN_Root\EZCAP_QT\EZCAP_QT\installer\{source}\EZCAP_Qt\*.o"
echo y | del "C:\SoftwareSVN\QHY_SVN_Root\EZCAP_QT\EZCAP_QT\installer\{source}\EZCAP_Qt\*.cpp"
echo y | del "C:\SoftwareSVN\QHY_SVN_Root\EZCAP_QT\EZCAP_QT\installer\{source}\EZCAP_Qt\*.h"

xcopy "C:\SoftwareSVN\QHY_SVN_Root\EZCAP_QT\build-EZCAP-Desktop_Qt_5_12_9_MinGW_32_bit-Release\release\platforms\*" "C:\SoftwareSVN\QHY_SVN_Root\EZCAP_QT\EZCAP_QT\installer\{source}\EZCAP_Qt\platforms\" /E /Y

set innoSetupCompile32="C:\Program Files (x86)\Inno Setup 5\ISCC.exe"
set srcName="Installer_mail_Qt.iss"


cd /d C:\SoftwareSVN\QHY_SVN_Root\EZCAP_QT\EZCAP_QT\installer
%innoSetupCompile32% /cc %srcName%

copy "C:\SoftwareSVN\QHY_SVN_Root\EZCAP_QT\EZCAP_QT\installer\Output\*.exe" "C:\SoftwareSVN\QHY_SVN_Root\ZBH_WorkSpace\QHYCCDAllInOne\{input}\ezcap_qt\"
pause