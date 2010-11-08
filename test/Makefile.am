bin_PROGRAMS = ut-fapws-lib

ut_fapws_lib_SOURCES = \
	ut_fapws_main.cpp \
	ut_simple_increase.cpp \
	ut_simple_increase.h \
	ut_spec_creat.cpp \
	ut_spec_creat.h \
	ut_ext_mut.cpp \
	ut_game_xo.cpp \
	ut_game_xo.h

AM_CPPFLAGS = -D_PLAT_LINUX_ -g -O0
ut_fapws_lib_LDFLAGS=-L../src -lfapws  -lcppunit
ut_fapws_lib_CXXFLAGS=-I@srcdir@/../src

check: test_fapws_lib

test_fapws_lib:
	 ./ut-fapws-lib

