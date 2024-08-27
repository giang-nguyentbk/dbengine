# SDKSYSROOT is an env variable which should be exported by doing "source <path-to-SDK>/SDK-***/sysroot/env.sh
# which is automatically done by running atbuild-sdk.sh"
SDK_SYSROOT_DIR		:= $(SDKSYSROOT)
SDK_USR_DIR		:= $(SDK_SYSROOT_DIR)/usr
SDK_LIB_DIR		:= $(SDK_USR_DIR)/lib
SDK_INC_DIR		:= $(SDK_USR_DIR)/include

ROOT_DIR 	:= $(shell git rev-parse --show-toplevel)
TARGET 		:= databaseIfTest
BIN_DIR 	:= $(ROOT_DIR)/sw/databaseif/unittest/bin

CFLAGS 		:= -c -g -Wall -Wextra
CXX 		:= g++

INCLUDE_DIR 	:= \
		-I$(ROOT_DIR)/sw/databaseif/if \
		-I$(ROOT_DIR)/sw/databaseif/inc \
		-I$(ROOT_DIR)/sw/dbloader/inc \
		# -I$(SDK_INC_DIR)

MAIN		:= $(ROOT_DIR)/sw/databaseif/unittest/databaseIfTest.cc
SRC1		:= $(ROOT_DIR)/sw/databaseif/src/databaseImpl.cc
SRC2		:= $(ROOT_DIR)/sw/dbloader/src/dbLoader.cc

OBJECTS 	=
OBJECTS 	+= $(BIN_DIR)/databaseIfTest.o
OBJECTS 	+= $(BIN_DIR)/databaseImpl.o
OBJECTS 	+= $(BIN_DIR)/dbLoader.o

all: create_bin $(OBJECTS) $(BIN_DIR)/$(TARGET)

create_bin:
	@mkdir -p $(BIN_DIR)

$(BIN_DIR)/databaseIfTest.o: $(MAIN)
	@echo "  CXX \t\t $@"
	@$(CXX) $(CFLAGS) $^ $(INCLUDE_DIR) -o $@

$(BIN_DIR)/databaseImpl.o: $(SRC1)
	@echo "  CXX \t\t $@"
	@$(CXX) $(CFLAGS) $^ $(INCLUDE_DIR) -o $@

$(BIN_DIR)/dbLoader.o: $(SRC2)
	@echo "  CXX \t\t $@"
	@$(CXX) $(CFLAGS) $^ $(INCLUDE_DIR) -o $@

$(BIN_DIR)/$(TARGET): $(OBJECTS)
	@echo "  CXXLD \t $@"
	@$(CXX) $^ -o $@

run:
	@$(BIN_DIR)/$(TARGET)

val:
	sudo valgrind --leak-check=yes --leak-check=full --show-leak-kinds=all $(BIN_DIR)/$(TARGET)

clean:
	rm -rf $(BIN_DIR)