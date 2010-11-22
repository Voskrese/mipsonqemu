TEMPLATE	= lib
TARGET		= platform
QT += sql
QT += network
CONFIG		+= plugin qt_warn debug_and_release
VPATH 		+= ../../src/
INCLUDEPATH += ../../src/
INCLUDEPATH += c:/boost/
INCLUDEPATH += ../../win/
INCLUDEPATH += ../../include/
VPATH		+= src/
SOURCES		= platform_win.cpp platform_base_hotkey.cpp platform_win_hotkey.cpp platform_win_util.cpp
HEADERS		= platform_base.h platform_win.h platform_base_hotkey.h platform_base_hottrigger.h platform_win_util.h
LIBS 		+= shell32.lib user32.lib gdi32.lib comctl32.lib
CONFIG		-= embed_manifest_dll

win32:debug {
	DESTDIR = ../../debug/
	LIBS +=   ../../debug/bmapi.lib
}

win32:release {
	DESTDIR = ../../release/
	LIBS +=   ../../debug/bmapi.lib
}