AC_PREREQ([2.65])
AC_CONFIG_MACRO_DIRS([m4])
AC_INIT([parallel_evolution-lib], [0.1], [fr.teodoro@gmail.com])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
LT_INIT

# Checks for programs.
AC_PROG_CC
AC_CHECK_PROGS([MPICC], [mpicc], [:])
if test "$MPICC" = :; then
	AC_MSG_ERROR([This package needs mpicc.])
fi

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h])
AC_CHECK_HEADERS([mpi.h],,AC_CHECK_HEADERS([mpi/mpi.h],,AC_MSG_ERROR([oops! is mpi installed?])))
AC_CONFIG_HEADERS([config.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC

AC_CONFIG_FILES([Makefile src/Makefile])

AC_OUTPUT
