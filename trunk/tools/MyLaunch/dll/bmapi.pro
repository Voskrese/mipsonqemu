######################################################################
# Automatically generated by qmake (1.07a) ??? ?? 30 10:58:24 2007
######################################################################

TEMPLATE	= lib
TARGET		= bmapi
CONFIG		+=dll  qt_warn debug_and_release
VPATH 		+= ../../src/
INCLUDEPATH += ../../src/
INCLUDEPATH += c:/boost/
INCLUDEPATH += ../../win/
INCLUDEPATH += ../include/
VPATH		+= src/
SOURCES		=bmapi.cpp
LIBS 		+= shell32.lib user32.lib gdi32.lib comctl32.lib Advapi32.lib
CONFIG -= embed_manifest_dll
DEFINES += WIN32
DEFINES += BMAPI_DLL
MOC_DIR += ../tmp
OBJECTS_DIR += ../tmp
QT += sql
QT += network
RC_FILE =   bmapi.rc
if(!debug_and_release|build_pass) {
   CONFIG(debug, debug|release) {
    DESTDIR =     ../debug/
   }
   CONFIG(release, debug|release) {
   DESTDIR = ../release/
   }
 }
