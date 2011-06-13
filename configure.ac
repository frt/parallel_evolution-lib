AC_PREREQ([2.65])
AC_INIT([parallel_evolution-lib], [0.1], [fr.teodoro@gmail.com])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

# Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB
AC_CHECK_PROGS([MPICC], [mpicc], [:])
if test "$MPICC" = :; then
	AC_MSG_ERROR([This package needs mpicc.])
fi

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h])
AC_CHECK_HEADERS([mpi.h])
AC_CONFIG_HEADERS([config.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC

AC_CONFIG_FILES([Makefile src/Makefile])

AC_OUTPUT
