######################################################################
# Automatically generated by qmake (1.07a) ??? ?? 30 10:58:24 2007
######################################################################

TEMPLATE        = lib
CONFIG         += dll  debug_and_release
INCLUDEPATH    += .
INCLUDEPATH    += ../include/
INCLUDEPATH    += c:/boost/
HEADERS         = weby.h
SOURCES         = weby.cpp 
#TARGET          = $$qtLibraryTarget(mergethread)
#! [0]
DEFINES += WIN32
DEFINES += WEBY_DLL
QT += sql
CONFIG -= embed_manifest_dll
LIBS 		+= shell32.lib user32.lib gdi32.lib comctl32.lib
if(!debug_and_release|build_pass) {
   CONFIG(debug, debug|release) {
    DESTDIR =     ../debug/
    LIBS +=../debug/bmapi.lib
    LIBS +=../debug/catalog.lib
   }
   CONFIG(release, debug|release) {
#    CONFIG +=     embed_manifest_dll
    DESTDIR = ../release/dll/
    LIBS +=../release/dll/bmapi.lib
    LIBS +=../release/dll/catalog.lib
   }
 }