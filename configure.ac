
AC_INIT([fapws], [0.1], [Yuri.Borisov@teleca.com])

AM_INIT_AUTOMAKE([-Wall])

AC_CONFIG_FILES([
		 Makefile
		 src/Makefile
		 test/Makefile
		 ])


AC_PROG_CC
AC_PROG_CXX
AC_PROG_LIBTOOL

# Configure options: --enable-debug[=no].
AC_ARG_ENABLE([debug],
	      [AS_HELP_STRING([--enable-debug],
			      [enable debug code (default is no)])],
			      [debug="$withval"], [debug=no])

# Configure options: --enable-internal-test[=no].
AC_ARG_ENABLE([internal-test],
	      [AS_HELP_STRING([--enable-internal-test],
			      [turn on internal testing (default is no)])],
	      [internal_test="$withval"], [internal_test=no])
AM_CONDITIONAL([INTERNAL_TEST], [test x$internal_test = xyes])

PKG_CHECK_MODULES(GLIB, [glib-2.0 >= 2.0.0], , [AC_MSG_ERROR([
You must have the GLib 2.0 and development headers installed to build.
])])

PKG_CHECK_MODULES(LIBXML, [libxml-2.0], , [AC_MSG_ERROR([
You must have the libxml-2.0 and development headers installed to build.
])])

AC_SUBST(LIBXML_CFLAGS)
AC_SUBST(LIBXML_LIBS)

if test x$debug = xyes; then
    CXXFLAGS="$CXXFLAGS -O0"
fi

if test x$internal_test = xyes ; then
	PKG_CHECK_MODULES(CPPUNIT, cppunit, , [AC_MSG_ERROR([
	You must have the libcppunit-dev installed to build tests.
	])])
	AC_SUBST(CPPUNIT_CFLAGS)
	AC_SUBST(CPPUNIT_LIBS)
fi


CXXFLAGS="$CXXFLAGS $GLIB_CFLAGS $LIBXML_CFLAGS" 
LDFLAGS="$LDFLAGS $GLIB_LIBS $LIBXML_LIBS"

AC_CONFIG_FILES([libfapws.pc])

AC_OUTPUT

echo CXXFLAGS $CXXFLAGS
echo "
($PACKAGE_NAME) version $PACKAGE_VERSION
Prefix.........: $prefix
Debug Build....: $debug
Internal Test..: $internal_test
C++ Compiler...: $CXX $CXXFLAGS $CPPFLAGS
Linker.........: $LD $LDFLAGS $LIBS
Doxygen........: ${DOXYGEN:-NONE}
"

