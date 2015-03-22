#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT(clipt, 1.0.1, faraz@fzv.ca)
AM_INIT_AUTOMAKE(clipt, 1.0.1)
AC_CONFIG_SRCDIR([utils.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h netdb.h stdlib.h string.h sys/file.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_MODE_T
AC_TYPE_PID_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_WAIT3
AC_TYPE_SIGNAL
AC_CHECK_FUNCS([alarm memset select socket strcasecmp strdup])

AC_CONFIG_FILES([Makefile]) 
AC_OUTPUT
