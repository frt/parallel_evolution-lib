INCLUDE_DIR = include
SRC_DIR = src
BUILD_DIR = build

CFLAGS += -I$(INCLUDE_DIR)

.PHONY: all

all: $(BUILD_DIR)/migrant.o $(BUILD_DIR)/topology.o $(BUILD_DIR)/algorithm.o

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

$(BUILD_DIR)/migrant.o: $(SRC_DIR)/migrant.c $(INCLUDE_DIR)/migrant.h $(INCLUDE_DIR)/common.h $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/topology.o: $(SRC_DIR)/topology.c $(INCLUDE_DIR)/topology.h $(INCLUDE_DIR)/common.h $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/algorithm.o: $(SRC_DIR)/algorithm.c $(INCLUDE_DIR)/algorithm.h $(INCLUDE_DIR)/common.h $(INCLUDE_DIR)/migrant.h $(INCLUDE_DIR)/population.h $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<
