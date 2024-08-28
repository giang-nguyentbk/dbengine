DBLOADER_DIR		:= $(SW_DIR)/dbloader
DBLOADER_SRC_DIR	:= $(DBLOADER_DIR)/src

DBLOADER_SRCS		=
DBLOADER_SRCS		+= dbLoader.cc

DBLOADER_OBJS		:= $(DBLOADER_SRCS:%.cc=$(OBJ_DIR)/%.o)

DBLOADER_INCS		:= \
			-I$(DBLOADER_DIR)/if \
			-I$(DBLOADER_DIR)/inc \
			-I$(SW_DIR)/databaseif/if \
			-I$(SW_DIR)/common \
			-I$(SDK_INC_DIR)

all: $(DBLOADER_OBJS)

# Build target 1 objects
$(OBJ_DIR)/%.o: $(DBLOADER_SRC_DIR)/%.cc
	@mkdir -p $(@D)
	@cd $(<D)
	@echo "  CXX \t\t $@"
	@$(SELF_CXX) $(SELF_LDFLAGS) $(SELF_CFLAGS) $(DBLOADER_INCS) $^ -o $@

clean-dbloader:
	@echo "  RMV \t\t $(BIN_DIR)/dbloader"
	@$(SELF_RMV) $(DBLOADER_OBJS)