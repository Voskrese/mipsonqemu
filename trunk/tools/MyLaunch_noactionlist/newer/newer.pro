######################################################################
# Automatically generated by qmake (2.01a) ??? ??? 6 14:25:24 2009
######################################################################

TEMPLATE = app
win32 {
  TARGET = newer
}
DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += .
CONFIG += debug_and_release
CONFIG += console
INCLUDEPATH += ../include/


# Input
SOURCES += main.cpp \
	..\lzma\7zFile.c\
	..\lzma\7zStream.c\
	..\lzma\Alloc.c\
	..\lzma\LzFind.c\
	..\lzma\LzFindMt.c\
	..\lzma\LzmaDec.c\
	..\lzma\LzmaEnc.c\
	..\lzma\Threads.c 

HEADERS = 
win32 {
  INCLUDEPATH += c:/boost/
  LIBS 		+= shell32.lib user32.lib
  CONFIG += embed_manifest_exe

  if(!debug_and_release|build_pass) {
   CONFIG(debug, debug|release) {
    DESTDIR =     ../debug/
   }
   CONFIG(release, debug|release) {
      DESTDIR =    ../release/
  }
}
}