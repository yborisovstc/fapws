includedir = @includedir@/fapws

AM_CPPFLAGS = -D_PLAT_LINUX_ -g -O0

lib_LTLIBRARIES = libfapws.la

libfapws_la_SOURCES = \
		       fapbase.cpp \
		       fapbase.h \
		       fapext.cpp \
		       fapext.h \
		       fapstext.cpp \
		       fapstext.h \
		       fapfact.cpp \
		       fapfact.h \
		       faplogger.cpp \
		       faplogger.h \
		       tadesl.cpp \
		       tadesl.h \
		       fapws.cpp \
		       fapws.h \
		       fapplat.h \
		       fapview.h \
		       viewgtk.h \
		       viewgtk.cpp \
		       fapviewprim.h \
		       fapviewprim.cpp \
		       deslbase.cpp \
		       deslbase.h 


include_HEADERS = \
		  fapbase.h \
		  fapext.h \
		  fapstext.h \
		  fapfact.h \
		  fapview.h \
		  viewgtk.h \
		  fapviewprim.h \
		  fapplat.h


libfapws_la_LDFLAGS = -rdynamic -no-undefined
