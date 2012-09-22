#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([SurfaceEmbedder], [0.9], [pskoda@sfu.ca])
AC_CONFIG_SRCDIR([cascades/cascades.cpp])
AM_INIT_AUTOMAKE
# AC_CONFIG_HEADER([config.h])

#Enable debug mode
AC_ARG_ENABLE([verbose],
 [  --enable-verbose    Turn on verbose output],
 [case "${enableval}" in
   yes) verbose=true;;
   no)  verbose=false ;;
   *) AC_MSG_ERROR([bad value ${enableval} for --enable-verbose]) ;;
   esac],[verbose=false])
AC_ARG_ENABLE([debug],
 [  --enable-debug    Turn on debugging],
 [case "${enableval}" in
   yes) debug=true; verbose=true;;
   no)  debug=false ;;
   *) AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
   esac],[debug=false])
AM_CONDITIONAL([DEBUG], [test x$debug = xtrue])
AM_CONDITIONAL([VERBOSE], [test x$verbose = xtrue])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lOGDF':
AC_CHECK_LIB([OGDF], [isConnected])
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [main])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE

# Checks for library functions.
AC_FUNC_MALLOC

dnl AC_CONFIG_FILES([cascades/Makefile
dnl                  embedder/Makefile
dnl                  factory/Makefile
dnl                  genstats/Makefile
dnl                  klein/Makefile
dnl                  mintester/Makefile
dnl                  ogdfbase/Makefile
dnl                  ogdftools/Makefile
dnl                  sieve/Makefile
dnl                  torus/Makefile])
AC_CONFIG_FILES([Makefile]) 
#		 mintester/Makefile 
#		 scripts/Makefile])
AC_OUTPUT
