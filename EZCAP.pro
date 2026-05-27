##-------------------------------------------------
#
# Project created by QtCreator 2013-09-05T17:30:21
#
#-------------------------------------------------

QT       += core gui network websockets
win32{ QT += axcontainer }

TRANSLATIONS += language/lan_zh_cn.ts \
                language/lan_en_us.ts \
                language/lan_ja_jp.ts

RC_FILE   = myRc.rc

ICON = image/ezcap.icns

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = EZCAP
TEMPLATE = app

SOURCES += src/main.cpp \
    src/dllqhyccd.cpp \
    src/borderLayout.cpp \
    src/ezCap.cpp \
    src/mcpIpcServer.cpp \
    src/mcpStdioClient.cpp \
    src/cameraChooser.cpp \
    src/frameToolCal.cpp \
    src/frameToolCapCal.cpp \
    src/gpsTool.cpp \
    src/imgAnalyze.cpp \
    src/liveCapThread.cpp \
    src/mainMenu.cpp \
    src/planner.cpp \
    src/about.cpp \
    src/favorite.cpp \
    src/fitHeader.cpp \
    src/tempControl.cpp \
    src/delegate.cpp \
    src/managementMenu.cpp \
    src/disktools.cpp \
    src/phdLink.cpp \
    src/darkFrameTool.cpp \
    src/downloadPreThread.cpp \
    src/downloadCapThread.cpp \
    src/downloadFocThread.cpp \
    src/cfwControl.cpp \
    src/cfwSetup.cpp \
    src/outputdebug.cpp \
    src/readmode.cpp \
    src/threadProcessImage.cpp \
    src/threadTempControl.cpp \
    src/videoShowThread.cpp \
    src/msgclient.cpp \
    src/technicalsupport.cpp \
    src/toolBurst.cpp \
    src/threadBurstCapture.cpp \
    src/toolCorrectCenter.cpp \
    src/toolTrigger.cpp \
    src/otherCameraSetup.cpp

HEADERS  += \
    include/dllqhyccd.h \
    include/ezCap.h \
    include/mcpIpcServer.h \
    include/mcpStdioClient.h \
    include/borderLayout.h \
    include/cameraChooser.h \
    include/frameToolCal.h \
    include/frameToolCapCal.h \
    include/gpsTool.h \
    include/imgAnalyze.h \
    include/liveCapThread.h \
    include/mainMenu.h \
    include/fitsio.h \
    include/fitsio2.h \
    include/longnam.h \
    include/tempControl.h \
    include/planner.h \
    include/fitHeader.h \
    include/favorite.h \
    include/about.h \
    include/delegate.h \
    #include/qhyccdStatus.h \
    include/managementMenu.h \
    include/disktools.h \
    #include/qhyccd.h \
    #include/qhyccdcamdef.h \
    #include/qhyccderr.h\
    include/qhyccdstruct.h \
    include/phdLink.h \
    #include/dithercontrol.h \
    include/darkFrameTool.h \
    include/downloadPreThread.h \
    include/downloadCapThread.h \
    include/downloadFocThread.h \
    include/myStruct.h \
    include/cfwControl.h \
    include/cfwSetup.h \
    include/outputdebug.h \
    include/readmode.h \
    include/threadProcessImage.h \
    include/threadTempControl.h \
    include/videoShowThread.h \
    include/msgclient.h \
    include/technicalsupport.h \
    include/toolBurst.h \
    include/threadBurstCapture.h \
    include/toolCorrectCenter.h \
    include/toolTrigger.h \
    include/otherCameraSetup.h

FORMS    += \
    ui/ezCap.ui \
    ui/cameraChooser.ui \
    ui/frameToolCal.ui \
    ui/frameToolCapCal.ui \
    ui/gpstool.ui \
    ui/imgAnalyze.ui \
    ui/otherCameraSetup.ui \
    ui/tempControl.ui \
    ui/planner.ui \
    ui/fitHeader.ui \
    ui/favorite.ui \
    ui/about.ui \
    ui/managementMenu.ui \
    ui/phdLink.ui \
    ui/darkFrameTool.ui \
    ui/cfwControl.ui \
    ui/cfwSetup.ui \
    ui/readmode.ui \
    ui/technicalsupport.ui \
    ui/toolBurst.ui \
    ui/toolCorrectCenter.ui \
    ui/toolTrigger.ui

RESOURCES += \
    res.qrc

OTHER_FILES += \
    uac.manifest

INCLUDEPATH += include
unix:{
    INCLUDEPATH += /usr/local/include
#    INCLUDEPATH += /usr/local/Cellar/libusb/1.0.20/include

    LIBS += -L/usr/local/lib -lqhyccd
    LIBS += -L/usr/local/lib -lopencv_imgproc -lopencv_highgui -lopencv_core -lopencv_imgcodecs -lopencv_video -lopencv_videoio
    LIBS += -L/usr/lib/aarch64-linux-gnu -lusb-1.0 -lcfitsio
}

win32: {
    contains(QT_ARCH, i386) {
        QMAKE_PRE_LINK += del config.bat

        INCLUDEPATH += -I$$PWD/include

        #LIBS += -L../EZCAP_Qt/winlib/x86 -lqhyccd
        LIBS += -L$$PWD/winlib/x86 -lopencv_core3416
        LIBS += -L$$PWD/winlib/x64 -lopencv_core3416    -lopencv_highgui3416 -lopencv_imgcodecs3416
        LIBS += -L$$PWD/winlib/x64 -lopencv_imgproc3416 -lopencv_video3416   -lopencv_videoio3416
        LIBS += -L$$PWD/winlib/x86 -lcfitsio

        QMAKE_POST_LINK += echo @ECHO OFF >> config.bat &
        QMAKE_POST_LINK += echo SETLOCAL ENABLEDELAYEDEXPANSION >> config.bat &
        CONFIG(debug, debug|release) {
            QMAKE_POST_LINK += echo PATH=$$PWD/Depend/x86/*.dll >> config.bat &
            QMAKE_POST_LINK += echo copy %%PATH:/=\%% .\debug >> config.bat &
        }else{
            QMAKE_POST_LINK += echo PATH=$$PWD/Depend/x86/*.dll >> config.bat &
            QMAKE_POST_LINK += echo copy %%PATH:/=\%% .\release >> config.bat &
        }
    } else {
        QMAKE_PRE_LINK += del config.bat

        INCLUDEPATH += -I$$PWD/include

        #LIBS += -L../EZCAP_Qt/winlib/x64 -lqhyccd
        LIBS += -L$$PWD/winlib/x64 -lopencv_core3416    -lopencv_highgui3416 -lopencv_imgcodecs3416
        LIBS += -L$$PWD/winlib/x64 -lopencv_imgproc3416 -lopencv_video3416   -lopencv_videoio3416
        LIBS += -L$$PWD/winlib/x64 -lcfitsio
#        LIBS += -L"/usr/local/lib" -lusb-1.0

        QMAKE_POST_LINK += echo @ECHO OFF >> config.bat &
        QMAKE_POST_LINK += echo SETLOCAL ENABLEDELAYEDEXPANSION >> config.bat &
        CONFIG(debug, debug|release) {
            QMAKE_POST_LINK += echo PATH=$$PWD/Depend/x64/*.dll >> config.bat &
            QMAKE_POST_LINK += echo copy %%PATH:/=\%% .\debug >> config.bat &
        }else{
            QMAKE_POST_LINK += echo PATH=$$PWD/Depend/x64/*.dll >> config.bat &
            QMAKE_POST_LINK += echo copy %%PATH:/=\%% .\release >> config.bat &
        }
    }

    CONFIG(release, debug|release) {
        QMAKE_POST_LINK += echo PATH=$$PWD/Depend/qhyccd.ini >> config.bat &
        QMAKE_POST_LINK += echo copy %%PATH:/=\%% . >> config.bat &
        QMAKE_POST_LINK += echo copy %%PATH:/=\%% .\release >> config.bat &

        QMAKE_POST_LINK += echo mkdir .\release\platforms >> config.bat &
        QMAKE_POST_LINK += echo PATH=$$[QT_HOST_DATA]/plugins/platforms/qminimal.dll >> config.bat &
        QMAKE_POST_LINK += echo copy %%PATH:/=\%% .\release\platforms >> config.bat &
        QMAKE_POST_LINK += echo PATH=$$[QT_HOST_DATA]/plugins/platforms/qoffscreen.dll >> config.bat &
        QMAKE_POST_LINK += echo copy %%PATH:/=\%% .\release\platforms >> config.bat &
        QMAKE_POST_LINK += echo PATH=$$[QT_HOST_DATA]/plugins/platforms/qwindows.dll >> config.bat &
        QMAKE_POST_LINK += echo copy %%PATH:/=\%% .\release\platforms >> config.bat &

        QMAKE_POST_LINK += echo PATH=$$[QT_HOST_DATA]/bin/Qt*Core.dll >> config.bat &
        QMAKE_POST_LINK += echo copy %%PATH:/=\%% .\release >> config.bat &
        QMAKE_POST_LINK += echo PATH=$$[QT_HOST_DATA]/bin/Qt*Gui.dll >> config.bat &
        QMAKE_POST_LINK += echo copy %%PATH:/=\%% .\release >> config.bat &
        QMAKE_POST_LINK += echo PATH=$$[QT_HOST_DATA]/bin/Qt*Widgets.dll >> config.bat &
        QMAKE_POST_LINK += echo copy %%PATH:/=\%% .\release >> config.bat &
        QMAKE_POST_LINK += echo PATH=$$[QT_HOST_DATA]/bin/Qt*Network.dll >> config.bat &
        QMAKE_POST_LINK += echo copy %%PATH:/=\%% .\release >> config.bat &
        QMAKE_POST_LINK += echo PATH=$$[QT_HOST_DATA]/bin/Qt*WebSockets.dll >> config.bat &
        QMAKE_POST_LINK += echo copy %%PATH:/=\%% .\release >> config.bat &

        QMAKE_POST_LINK += echo del .\release\Qt*3DCore.dll >> config.bat &
        QMAKE_POST_LINK += echo del .\release\Qt*SvgWidgets.dll >> config.bat &
        QMAKE_POST_LINK += echo del .\release\Qt*QuickWidgets.dll >> config.bat &
        QMAKE_POST_LINK += echo del .\release\Qt*QmlNetwork.dll >> config.bat &
        QMAKE_POST_LINK += echo del .\release\Qt*QmlCore.dll >> config.bat &
        QMAKE_POST_LINK += echo del .\release\Qt*PdfWidgets.dll >> config.bat &
        QMAKE_POST_LINK += echo del .\release\Qt*OpenGLWidgets.dll >> config.bat &
        QMAKE_POST_LINK += echo del .\release\Qt*MultimediaWidgets.dll >> config.bat &

        QMAKE_POST_LINK += echo PATH=$$[QT_HOST_DATA]/bin/libgcc*.dll >> config.bat &
        QMAKE_POST_LINK += echo copy %%PATH:/=\%% .\release >> config.bat &
        QMAKE_POST_LINK += echo PATH=$$[QT_HOST_DATA]/bin/libstdc*.dll >> config.bat &
        QMAKE_POST_LINK += echo copy %%PATH:/=\%% .\release >> config.bat &
        QMAKE_POST_LINK += echo PATH=$$[QT_HOST_DATA]/bin/libwinpthread-1.dll >> config.bat &
        QMAKE_POST_LINK += echo copy %%PATH:/=\%% .\release >> config.bat &
    }
    else
    {
        QMAKE_POST_LINK += echo PATH=$$PWD/Depend/qhyccd.ini >> config.bat &
        QMAKE_POST_LINK += echo copy %%PATH:/=\%% . >> config.bat &
        QMAKE_POST_LINK += echo copy %%PATH:/=\%% .\debug >> config.bat &
    }
    QMAKE_POST_LINK += .\config.bat

}

release: {
    DEFINES += QT_NO_WARNING_OUTPUT\
    QT_NO_DEBUG_OUTPUT
#   CONFIG += console
}

