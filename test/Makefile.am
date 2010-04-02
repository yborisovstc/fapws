bin_PROGRAMS = ut-fapws-lib

ut_fapws_lib_SOURCES = \
	ut_fapws_main.cpp \
	ut_simple_increase.cpp \
	ut_simple_increase.h \
	ut_game_xo.cpp \
	ut_game_xo.h \
	ut_events_counter.cpp \
	ut_events_counter.h

AM_CPPFLAGS = -D_PLAT_LINUX_ -g -O0
ut_fapws_lib_LDFLAGS=-L../src -lfapws @CPPUNIT_LIBS@
ut_fapws_lib_CXXFLAGS=-I@srcdir@/../src @CPPUNIT_CFLAGS@

check: test_fapws_lib

test_fapws_lib:
	 ./ut-fapws-lib

