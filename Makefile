INCLUDE_DIR = include
SRC_DIR = src
BUILD_DIR = build
LIB_DIR = $(BUILD_DIR)/lib

OBJ_FILES = $(BUILD_DIR)/migrant.o $(BUILD_DIR)/topology.o $(BUILD_DIR)/algorithm.o $(BUILD_DIR)/topology_parser.o $(BUILD_DIR)/parallel_evolution.o

CFLAGS += -I$(INCLUDE_DIR)

.PHONY: all clean lib

all: lib

lib: $(LIB_DIR)/parallel_evolution.a
	
$(LIB_DIR)/parallel_evolution.a: $(LIB_DIR) $(OBJ_FILES)
	ar rcs $@ $(OBJ_FILES)

clean:
	-rm -r $(BUILD_DIR)

$(BUILD_DIR):
	mkdir $@

$(LIB_DIR):
	mkdir $@

$(BUILD_DIR)/migrant.o: $(SRC_DIR)/migrant.c $(INCLUDE_DIR)/migrant.h $(INCLUDE_DIR)/common.h $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/topology.o: $(SRC_DIR)/topology.c $(INCLUDE_DIR)/topology.h $(INCLUDE_DIR)/common.h $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/algorithm.o: $(SRC_DIR)/algorithm.c $(INCLUDE_DIR)/algorithm.h $(INCLUDE_DIR)/common.h $(INCLUDE_DIR)/migrant.h $(INCLUDE_DIR)/population.h $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/topology_parser.o: $(SRC_DIR)/topology_parser.c $(INCLUDE_DIR)/topology_parser.h $(INCLUDE_DIR)/common.h $(INCLUDE_DIR)/topology.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/parallel_evolution.o: \
		$(SRC_DIR)/parallel_evolution.c \
		$(INCLUDE_DIR)/parallel_evolution.h \
		$(INCLUDE_DIR)/common.h \
		$(INCLUDE_DIR)/topology.h \
		$(INCLUDE_DIR)/topology_parser.h \
		$(INCLUDE_DIR)/population.h \
		$(INCLUDE_DIR)/algorithm.h
	$(CC) $(CFLAGS) -c -o $@ $<
