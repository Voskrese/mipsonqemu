######################################################################
# Automatically generated by qmake (2.01a) ??? ??? 5 17:56:12 2009
######################################################################

TEMPLATE = app
TARGET = 
CONFIG		+=debug_and_release
DEPENDPATH += .
INCLUDEPATH += .
CONFIG += console 
# Input
SOURCES += main.cpp
if(!debug_and_release|build_pass) {
   CONFIG(debug, debug|release) {
    DESTDIR =    ../include/
   }
   CONFIG(release, debug|release) {
    DESTDIR = ../include/
   }
 }

