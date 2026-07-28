cmake_minimum_required(VERSION 3.16)
project(QCAP)
if (WIN32)
  set(CMAKE_PREFIX_PATH "C:/Qt/Qt5.13.0/5.13.0/mingw73_64/lib/cmake/Qt5")    
endif (WIN32)



message(STATUS ${CMAKE_PREFIX_PATH})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
#set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTORCC ON)
#set(CMAKE_AUTOUIC ON)
add_definitions(-DUNICODE -D_UNICODE)


set(CMAKE_POSITION_INDEPENDENT_CODE ON)
find_package(Qt5 COMPONENTS Widgets Core Network WebSockets  REQUIRED)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/ui)


set(SOURCES
    src/about.cpp
    src/borderLayout.cpp
    src/cameraChooser.cpp
    src/cfwControl.cpp
    src/cfwSetup.cpp
    src/darkFrameTool.cpp
    src/delegate.cpp
    src/disktools.cpp
    src/dllqhyccd.cpp
    src/downloadCapThread.cpp
    src/downloadFocThread.cpp
    src/downloadPreThread.cpp
    src/ezCap.cpp
    src/mcpStdioClient.cpp
    src/favorite.cpp
    src/fitHeader.cpp
    src/main.cpp
    src/mainMenu.cpp
    src/managementMenu.cpp
    src/phdLink.cpp
    src/planner.cpp
    src/readmode.cpp
    src/tempControl.cpp
    src/gpsTool.cpp
    src/imgAnalyze.cpp
    src/toolBurst.cpp
    src/toolTrigger.cpp
    src/toolCorrectCenter.cpp
    src/outputdebug.cpp
    src/frameToolCapCal.cpp
    src/frameToolCal.cpp
    src/technicalsupport.cpp
    src/otherCameraSetup.cpp
    src/liveCapThread.cpp
    src/threadProcessImage.cpp
    src/videoShowThread.cpp
    src/threadTempControl.cpp
    src/msgclient.cpp
    src/mcpIpcServer.cpp
    src/pixelMagnifier.cpp
    src/toolBurst.cpp
    src/threadBurstCapture.cpp
    )
#qt5_generate_moc(${SOURCES} TARGET QCAP)
qt5_wrap_ui(SOURCES
    ui/about.ui
    ui/cameraChooser.ui
    ui/cfwControl.ui
    ui/cfwSetup.ui
    ui/darkFrameTool.ui
    ui/ezCap.ui
    ui/favorite.ui
    ui/fitHeader.ui
    ui/managementMenu.ui
    ui/phdLink.ui
    ui/planner.ui
    ui/readmode.ui
    ui/tempControl.ui
    ui/gpstool.ui
    ui/imgAnalyze.ui
    ui/toolBurst.ui
    ui/toolTrigger.ui
    ui/toolCorrectCenter.ui
    ui/frameToolCapCal.ui
    ui/frameToolCal.ui
    ui/technicalsupport.ui
    ui/otherCameraSetup.ui
    )
qt5_wrap_cpp(SOURCES
    include/about.h
    include/borderLayout.h
    include/cameraChooser.h
    include/cfwControl.h
    include/cfwSetup.h
    include/darkFrameTool.h
    include/delegate.h
    include/disktools.h
    include/dllqhyccd.h
    include/downloadCapThread.h
    include/downloadFocThread.h
    include/downloadPreThread.h
    include/ezCap.h
    include/mcpStdioClient.h
    include/favorite.h
    include/fitHeader.h
    include/mainMenu.h
    include/managementMenu.h
    include/phdLink.h
    include/planner.h
    include/readmode.h
    include/tempControl.h
    include/gpsTool.h
    include/imgAnalyze.h
    include/toolBurst.h
    include/toolTrigger.h
    include/toolCorrectCenter.h
    include/outputdebug.h
    include/frameToolCapCal.h
    include/frameToolCal.h
    include/technicalsupport.h
    include/otherCameraSetup.h
    include/liveCapThread.h
    include/threadProcessImage.h
    include/videoShowThread.h
    include/threadTempControl.h
    include/msgclient.h
    include/mcpIpcServer.h
    include/toolBurst.h
    include/threadBurstCapture.h

    include/fitsio.h
    include/opencv2/core/core_c.h
    )


qt5_add_resources(SOURCES res.qrc)


add_executable(QCAP ${SOURCES} myRc.rc)
#include(${QT_USE_FILE})

target_include_directories(QCAP PUBLIC ${CMAKE_SOURCE_DIR})
target_include_directories(QCAP PUBLIC ${CMAKE_SOURCE_DIR}/include)
if (WIN32)
	target_include_directories(QCAP PUBLIC ${CMAKE_SOURCE_DIR}/include/opencv)
	target_include_directories(QCAP PUBLIC ${CMAKE_SOURCE_DIR}/include/opencv2)
endif (WIN32)

if (WIN32)
	target_link_directories(QCAP PUBLIC ${CMAKE_SOURCE_DIR}/Depend/x64)
endif (WIN32)

if (WIN32)
	target_link_libraries(QCAP Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Network Qt5::WebSockets cfitsio.dll opencv_core249.dll opencv_highgui249.dll opencv_imgproc249.dll)
else ()
	target_link_libraries(QCAP Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Network Qt5::WebSockets cfitsio opencv_highgui opencv_imgproc opencv_core opencv_imgcodecs opencv_video opencv_videoio usb-1.0)
endif ()

if (WIN32)
	add_custom_command(
			TARGET QCAP POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Core> $<TARGET_FILE_DIR:QCAP>
			COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Gui> $<TARGET_FILE_DIR:QCAP>
			COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Widgets> $<TARGET_FILE_DIR:QCAP>
			COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Network> $<TARGET_FILE_DIR:QCAP>
			COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::WebSockets> $<TARGET_FILE_DIR:QCAP>

	)
	configure_file(${CMAKE_SOURCE_DIR}/Depend/x64/qhyccd.dll ${CMAKE_BINARY_DIR}/qhyccd.dll COPYONLY)
	configure_file(${CMAKE_SOURCE_DIR}/Depend/x64/cfitsio.dll ${CMAKE_BINARY_DIR}/cfitsio.dll COPYONLY)
	configure_file(${CMAKE_SOURCE_DIR}/Depend/x64/opencv_core249.dll ${CMAKE_BINARY_DIR}/opencv_core249.dll COPYONLY)
	configure_file(${CMAKE_SOURCE_DIR}/Depend/x64/opencv_highgui249.dll ${CMAKE_BINARY_DIR}/opencv_highgui249.dll COPYONLY)
	configure_file(${CMAKE_SOURCE_DIR}/Depend/x64/opencv_imgproc249.dll ${CMAKE_BINARY_DIR}/opencv_imgproc249.dll COPYONLY)
endif (WIN32)


install(TARGETS QCAP DESTINATION bin)

set(CPACK_PACKAGE_VENDOR "QHYCCD")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "QCAP for camera test")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
#set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENCE")
#set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

if (WIN32)
    set(CPACK_NSIS_MUI_ICON ${CMAKE_SOURCE_DIR}/image/EZCAP_Icon.ico)
#    set(CPACK_NSIS_MENU_LINKS "QCap" "$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\QCAP.lnk' '$INSTDIR\\\\bin\\\\QCAP.exe")
	set(CPACK_NSIS_CREATE_ICONS_EXTRA "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\QCAP.lnk' '$INSTDIR\\\\bin\\\\QCAP.exe' ")
	set(CPACK_NSIS_CREATE_ICONS_EXTRA "CreateShortCut '$desktop\\\\QCAP.lnk' '$INSTDIR\\\\bin\\\\QCAP.exe' '' '$INSTDIR\\\\bin\\\\QCAP.exe'")
	install(FILES ${CMAKE_BINARY_DIR}/qhyccd.dll
			${CMAKE_BINARY_DIR}/cfitsio.dll
			${CMAKE_BINARY_DIR}/opencv_core249.dll
			${CMAKE_BINARY_DIR}/opencv_highgui249.dll
			${CMAKE_BINARY_DIR}/opencv_imgproc249.dll
			DESTINATION bin)
endif(WIN32)

set(CPACK_SOURCE_GENERATOR "TGZ;ZIP")
set(CPACK_SOURCE_IGNORE_FILES
        /.git
        /dist
        /.*build.*
        /\\\\.DS_Store
        )

include(CPack)
