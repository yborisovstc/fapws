
AC_INIT([fapws], [0.1], [Yuri.Borisov@teleca.com])

AM_INIT_AUTOMAKE([-Wall])

AC_CONFIG_FILES([
		 Makefile
		 src/Makefile
		 ])


AC_PROG_CC
AC_PROG_CXX
AC_PROG_LIBTOOL

PKG_CHECK_MODULES(GLIB, [glib-2.0 >= 2.0.0], , [AC_MSG_ERROR([
You must have the GLib 2.0 and development headers installed to build.
])])

PKG_CHECK_MODULES(LIBXML, [libxml-2.0], , [AC_MSG_ERROR([
You must have the libxml-2.0 and development headers installed to build.
])])

AC_SUBST(LIBXML_CFLAGS)
AC_SUBST(LIBXML_LIBS)


CXXFLAGS="$CFLAGS $GLIB_CFLAGS $LIBXML_CFLAGS" 
LDFLAGS="$LDFLAGS $GLIB_LIBS $LIBXML_LIBS"

AC_CONFIG_FILES([libfapws.pc])

AC_OUTPUT

echo CXXFLAGS $CXXFLAGS
