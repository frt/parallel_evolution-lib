AM_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir)/include
lib_LIBRARIES = libparallel_evolution.a
libparallel_evolution_a_SOURCES = algorithm.c \
				  log.c \
				  migrant.c \
				  mpi_util.c \
				  parallel_evolution.c \
				  population.c \
				  processes.c \
				  report.c \
				  topology.c \
				  topology_parser.c
nobase_include_HEADERS = ../include/parallel_evolution.h \
			 ../include/parallel_evolution/algorithm.h \
			 ../include/parallel_evolution/log.h \
			 ../include/parallel_evolution/mpi_util.h \
			 ../include/parallel_evolution/population.h \
			 ../include/parallel_evolution/report.h \
			 ../include/parallel_evolution/topology_parser.h \
			 ../include/parallel_evolution/common.h \
			 ../include/parallel_evolution/migrant.h \
			 ../include/parallel_evolution/processes.h \
			 ../include/parallel_evolution/topology.h

mpi_util.o: mpi_util.c
	$(MPICC) $(AM_CPPFLAGS) $(CFLAGS) -c -o $@ $<

parallel_evolution.o: parallel_evolution.c
	$(MPICC) $(AM_CPPFLAGS) $(CFLAGS) -c -o $@ $<
