INCLUDE_DIR = include
SRC_DIR = src
BUILD_DIR = build
LIB_DIR = $(BUILD_DIR)/lib
BIN_DIR = $(BUILD_DIR)/bin

OBJ_FILES = $(BUILD_DIR)/migrant.o $(BUILD_DIR)/topology.o $(BUILD_DIR)/algorithm.o $(BUILD_DIR)/topology_parser.o $(BUILD_DIR)/population.o $(BUILD_DIR)/parallel_evolution.o $(BUILD_DIR)/mpi_util.o

CFLAGS += -I$(INCLUDE_DIR)

.PHONY: all clean lib programs

all: lib programs

lib: $(LIB_DIR)/parallel_evolution.a

programs: $(BIN_DIR)/parallel_evolution_bfo
	
$(LIB_DIR)/parallel_evolution.a: $(LIB_DIR) $(OBJ_FILES)
	ar rcs $@ $(OBJ_FILES)

$(BIN_DIR)/parallel_evolution_bfo: $(BIN_DIR) $(BUILD_DIR)/main_bfo.o $(LIB_DIR)/parallel_evolution.a
	mpicc $(CFLAGS) -o $@ $*

clean:
	-rm -r $(BUILD_DIR)

$(BUILD_DIR):
	mkdir $@

$(LIB_DIR):
	mkdir -p $@

$(BIN_DIR):
	mkdir -p $@

$(BUILD_DIR)/migrant.o: $(SRC_DIR)/migrant.c $(INCLUDE_DIR)/migrant.h $(INCLUDE_DIR)/common.h $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/topology.o: $(SRC_DIR)/topology.c $(INCLUDE_DIR)/topology.h $(INCLUDE_DIR)/common.h $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/algorithm.o: $(SRC_DIR)/algorithm.c $(INCLUDE_DIR)/algorithm.h $(INCLUDE_DIR)/common.h $(INCLUDE_DIR)/migrant.h $(INCLUDE_DIR)/population.h $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/topology_parser.o: $(SRC_DIR)/topology_parser.c $(INCLUDE_DIR)/topology_parser.h $(INCLUDE_DIR)/common.h $(INCLUDE_DIR)/topology.h $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/population.o: $(SRC_DIR)/population.c $(INCLUDE_DIR)/population.h $(INCLUDE_DIR)/common.h $(INCLUDE_DIR)/migrant.h $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/main_bfo.o: $(SRC_DIR)/main_bfo.c $(INCLUDE_DIR)/parallel_evolution.h $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/parallel_evolution.o: \
		$(SRC_DIR)/parallel_evolution.c \
		$(INCLUDE_DIR)/parallel_evolution.h \
		$(INCLUDE_DIR)/common.h \
		$(INCLUDE_DIR)/topology.h \
		$(INCLUDE_DIR)/topology_parser.h \
		$(INCLUDE_DIR)/population.h \
		$(INCLUDE_DIR)/algorithm.h
	mpicc $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/mpi_util.o: $(SRC_DIR)/mpi_util.c $(INCLUDE_DIR)/mpi_util.h $(INCLUDE_DIR)/topology.h $(INCLUDE_DIR)/common.h
	mpicc $(CFLAGS) -c -o $@ $<
