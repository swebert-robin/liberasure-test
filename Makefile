TARGET := main

BIN := bin
SRC := src
UTIL := util
BUILD := build
INCLUDE := include /usr/include/liberasurecode /usr/local/include/liberasurecode

CPP_FLAGS := -lerasurecode -ldl -lpthread -lz -lgcov

LD_LIBRARY_PATH := LD_LIBRARY_PATH=/usr/lib:/usr/local/lib

all:
	gcc $(INCLUDE:%=-I %) $(CPP_FLAGS) -o $(BUILD)/data.o -c $(SRC)/data.c
	gcc $(INCLUDE:%=-I %) $(CPP_FLAGS) -o $(BUILD)/timing.o -c $(SRC)/timing.c
	gcc $(INCLUDE:%=-I %) $(CPP_FLAGS) -o $(BIN)/$(TARGET) $(BUILD)/* $(SRC)/main.c

clean:
	rm -f $(BIN)/* $(BUILD)/*

rotate:
	util/rotate_file.sh

run:
	util/rotate_file.sh
	$(LD_LIBRARY_PATH) $(UTIL)/run.sh $(be)
