######################################################################
# Automatically generated by qmake (2.01a) ??? ??? 5 17:56:12 2009
######################################################################

TEMPLATE = app
TARGET = 
CONFIG		+=debug_and_release
DEPENDPATH += .
INCLUDEPATH += .
SOURCES += main.cpp \
	..\lzma\7zFile.c\
	..\lzma\7zStream.c\
	..\lzma\Alloc.c\
	..\lzma\LzFind.c\
	..\lzma\LzFindMt.c\
	..\lzma\LzmaDec.c\
	..\lzma\LzmaEnc.c\
	..\lzma\Threads.c
	
CONFIG += console 
if(!debug_and_release|build_pass) {
   CONFIG(debug, debug|release) {
    DESTDIR = ../resource/
   }
   CONFIG(release, debug|release) {
    DESTDIR = ../resource/
   }
 }

