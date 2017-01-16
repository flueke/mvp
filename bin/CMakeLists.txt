if (WIN32)
set(DLL_FILES
	win32/libgcc_s_dw2-1.dll
	win32/libstdc++-6.dll
	win32/libusb0.dll
	win32/libwinpthread-1.dll
	win32/Qt5Concurrent.dll
	win32/Qt5Core.dll
	win32/Qt5Gui.dll
	win32/Qt5Network.dll
	win32/Qt5OpenGL.dll
	win32/Qt5PrintSupport.dll
	win32/Qt5SerialPort.dll
	win32/Qt5Svg.dll
	win32/Qt5Widgets.dll
	win32/quazip.dll
	)

set(DLL_SUBDIRS
    win32/iconengines
	win32/imageformats
	win32/platforms
	)

INSTALL(FILES ${DLL_FILES} DESTINATION bin)
INSTALL(DIRECTORY ${DLL_SUBDIRS} DESTINATION bin)

else (WIN32)
endif (WIN32)