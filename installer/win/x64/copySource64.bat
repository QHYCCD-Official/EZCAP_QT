echo y | del "C:\Jenkins\workspace\EZCAP_QT\installer_x64\{source}\EZCAP_Qt\*"
echo y | del "C:\SoftwareSVN\QHY_SVN_Root\EZCAP_QT\make-qt-64\release\log\*.log"
echo y | del "C:\Jenkins\workspace\EZCAP_QT\installer_x64\Output\*.exe"
copy "C:\SoftwareSVN\QHY_SVN_Root\EZCAP_QT\make-qt-64\release\*.*" "C:\Jenkins\workspace\EZCAP_QT\installer_x64\{source}\EZCAP_Qt\"

echo y | del "C:\Jenkins\workspace\EZCAP_QT\installer_x64\{source}\EZCAP_Qt\*.o"
echo y | del "C:\Jenkins\workspace\EZCAP_QT\installer_x64\{source}\EZCAP_Qt\*.cpp"
echo y | del "C:\Jenkins\workspace\EZCAP_QT\installer_x64\{source}\EZCAP_Qt\*.h"
xcopy "C:\SoftwareSVN\QHY_SVN_Root\EZCAP_QT\make-qt-64\release\platforms\*" "C:\Jenkins\workspace\EZCAP_QT\installer_x64\{source}\EZCAP_Qt\platforms\" /E /Y

set innoSetupCompile32="C:\Program Files (x86)\Inno Setup 5\ISCC.exe"
set srcName="Installer_mail_Qt.iss"


cd /d C:\Jenkins\workspace\EZCAP_QT\installer_x64
%innoSetupCompile32% /cc %srcName%

copy "C:\Jenkins\workspace\EZCAP_QT\installer_x64\Output\*.exe" "C:\SoftwareSVN\QHY_SVN_Root\ZBH_WorkSpace\QHYCCDAllInOne\{input}\ezcap_qt\"
copy "C:\Jenkins\workspace\EZCAP_QT\installer_x64\Output\*.exe" "D:\ftpRoot\ezcap_qt\"

pause