# Process this file with autoconf to produce a configure script.
# Copyright (C) 2002 lignum Computing, Inc. <oglft@lignumcomputing.com>
# $Id: configure.ac,v 1.12 2003/10/01 14:12:29 allen Exp $

AC_INIT(OGLFT, 0.9, oglft@lignumcomputing.com)
AC_CONFIG_AUX_DIR(config)
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER([config/config.h])
AC_CONFIG_SRCDIR([liboglft/OGLFT.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL
AM_PROG_LIBTOOL
AC_PROG_MAKE_SET

# Checks for libraries.
AC_CHECK_LIB(m, sqrt)
AC_CHECK_LIB(GL, glGetString)
AC_CHECK_LIB(GLU, gluGetString)
#AC_CHECK_LIB(glut, glutInit,,,[-L/usr/X11R6/lib -lXi -lXmu])
AC_CHECK_LIB(freetype, FT_Init_FreeType)

#AM_WITH_MPATROL

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h unistd.h GL/gl.h GL/glu.h GL/glut.h])
AC_CHECK_HEADER(ft2build.h,,
  AC_MSG_WARN([could not successfully compile with FreeType2 using the])
  AC_MSG_WARN([default compiler flags; re-trying with])
  AC_MSG_WARN([CPPFLAGS=$CPPFLAGS -I/usr/include/freetype2])
  CPPFLAGS="$CPPFLAGS -I/usr/include/freetype2"
  unset ac_cv_header_ft2build_h
  [AC_CHECK_HEADER(ft2build.h,,
    AC_MSG_ERROR([could not find FreeType2 headers])])
)

# Make sure we've got an SGI sample implementation compatible libGLU
AC_CHECK_TYPE([GLUtesselator*],,
              AC_MSG_ERROR([must have SGI sample implementation compatible libGLU]),
              [#include <GL/glu.h>])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.
AC_CHECK_FUNCS([atexit memset])

# Autoheader seems to complain about a lack of these (perhaps rightly)
AH_TEMPLATE(OGLFT_DEBUG, [Turn on any additional debugging edits])
AH_TEMPLATE(OGLFT_NO_SOLID, [Turn off the solid type])
AH_TEMPLATE(OGLFT_NO_QT, [Turn off the Qt demos if you do not have Qt])

AC_ARG_ENABLE(debug,
[[  --enable-debug          enable OGLFT debugging [default=no]]],
  enable_debug=$enableval, enable_debug=no)
if test "x$enable_debug" = xyes; then
  AC_DEFINE(OGLFT_DEBUG, 1)
  if test "x$GCC" = xyes; then
    CFLAGS="$CFLAGS -g -O0 -Wall -W"
    CXXFLAGS="$CXXFLAGS -g -O0 -Wall -W"
  else
    AC_MSG_WARN([compiler unknown, please disable optimizations manually in \
your CFLAGS.])
  fi
fi

# You can turn off the solid type if you don't have GLE.

AC_ARG_ENABLE(solid,
[[  --disable-solid         disable the solid type [default=yes]]],
  enable_solid=$enableval, enable_solid=yes)
if test $enable_solid = yes; then
  AC_CHECK_HEADER(GL/gle.h)
  if test $ac_cv_header_GL_gle_h = no; then
    AC_MSG_WARN([could not find GLE header. Solid face disabled.])
    enable_solid=no
  else
    AC_CHECK_LIB(gle, gleExtrusion)
    if test $ac_cv_lib_gle_gleExtrusion = no; then
      AC_MSG_WARN([could not find GLE library. Solid face disabled.])
      enable_solid=no
    fi
  fi
fi

if test $enable_solid = no; then
  AC_DEFINE(OGLFT_NO_SOLID, 1)
fi

# You can turn off UNICODE support and the demos which depend on Qt
# if you don't have it

AC_ARG_ENABLE(qt,
[[  --disable-qt            disable UNICODE and the demo programs which depend on Qt [default=yes]]],
  enable_qt=$enableval, enable_qt=yes)

if test $enable_qt = yes; then
  AC_LANG_PUSH(C++)
  # Take a stab at compiling a Qt program with OpenGL support
  if test "x$QTDIR" = x; then
    AC_MSG_WARN([you must set \$QTDIR in order to enable the Qt demo programs])
    enable_qt=no
  else
    qt_save_CPPFLAGS=$CPPFLAGS
    qt_save_LDFLAGS=$LDFLAGS
    qt_save_LIBS=$LIBS
    CPPFLAGS="$CPPFLAGS -I${QTDIR}/include"
    LDFLAGS="$LDFLAGS -L${QTDIR}/lib"

    QTLIB=""
    LIBS="$qt_save_LIBS -lqt"
    AC_MSG_CHECKING([checking for QGLWidget in -lqt])
    AC_TRY_LINK( [#include <qgl.h>], [QGLWidget* glw;], qtlib=yes, qtlib=no )
    AC_MSG_RESULT([$qtlib])

    if test $qtlib = yes; then
      QTLIB=-lqt
    else
      LIBS="$qt_save_LIBS -lqt-mt"
      AC_MSG_CHECKING([checking for QGLWidget in -lqt-mt])
      AC_TRY_LINK( [#include <qgl.h>], [QGLWidget* glw;], qtlib=yes, qtlib=no )
      AC_MSG_RESULT([$qtlib])

      if test $qtlib = yes; then
        QTLIB=-lqt-mt
      else
        AC_MSG_WARN([could not find a version of Qt with OpenGL support. Qt demos disabled.])
        enable_qt=no
      fi
    fi

    AC_SUBST(QTLIB, $QTLIB)

#    CPPFLAGS=$qt_save_CPPFLAGS
#    LDFLAGS=$qt_save_LDFLAGS
#    LIBS=$qt_save_LIBS
  fi
  AC_LANG_POP(C++)
fi

if test $enable_qt = no; then
  AC_DEFINE(OGLFT_NO_QT, 1)
fi

AM_CONDITIONAL(OGLFT_DEBUG, test $enable_debug = yes)
AM_CONDITIONAL(OGLFT_NO_SOLID, test $enable_solid = no)
AM_CONDITIONAL(OGLFT_NO_QT, test $enable_qt = no)

AC_CONFIG_FILES([Makefile liboglft/Makefile doc/Makefile tests/Makefile])
AC_OUTPUT
