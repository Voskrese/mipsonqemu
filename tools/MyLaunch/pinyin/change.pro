######################################################################
# Automatically generated by qmake (2.01a) ??? ??? 6 14:25:24 2009
######################################################################

TEMPLATE = app
win32 {
  TARGET = change
}
DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += .
CONFIG += debug_and_release
CONFIG += console
#INCLUDEPATH += ../include/


# Input
SOURCES += change.cpp
 
HEADERS += 
win32 {
  INCLUDEPATH += c:/boost/
  LIBS 		+= shell32.lib user32.lib
  CONFIG += embed_manifest_exe

  if(!debug_and_release|build_pass) {
   CONFIG(debug, debug|release) {
    DESTDIR =     ./
   }
   CONFIG(release, debug|release) {
      DESTDIR =    ../release/
  }
}
}