TEXTTOBIN_DIR			:= $(SW_DIR)/texttobin
TEXTTOBIN_SRC_DIR		:= $(TEXTTOBIN_DIR)/src

# TEXTTOBIN_CXXLDFLAGS		:= -lpthread -lrt -L$(LIB_DIR) -litc -L$(SDK_LIB_DIR) -ltraceif
TEXTTOBIN_CXXLDFLAGS		:=

TARGET_TEXTTOBIN_W_LIBAR	:= textToBin_ar
TARGET_TEXTTOBIN_W_LIBSO	:= textToBin_so

TEXTTOBIN_SRC			= 
TEXTTOBIN_SRC			+= textToBin.cc

TEXTTOBIN_OBJ			:= $(TEXTTOBIN_SRC:%.cc=$(OBJ_DIR)/%.o)

TEXTTOBIN_INC			:= \
				-I$(TEXTTOBIN_DIR)/if \
				-I$(TEXTTOBIN_DIR)/inc \
				# -I$(SDK_INC_DIR)

all: $(TEXTTOBIN_OBJ) $(EXEC_DIR)/$(TARGET_TEXTTOBIN_W_LIBAR) $(EXEC_DIR)/$(TARGET_TEXTTOBIN_W_LIBSO)

# Build target 1 objects
$(OBJ_DIR)/%.o: $(TEXTTOBIN_SRC_DIR)/%.cc
	@mkdir -p $(@D)
	@cd $(<D)
	@echo "  CXX \t\t $@"
	@$(SELF_CXX) $(SELF_CFLAGS) $(TEXTTOBIN_INC) $< -o $@

$(EXEC_DIR)/$(TARGET_TEXTTOBIN_W_LIBAR): $(TEXTTOBIN_OBJ)
	@mkdir -p $(@D)
	@cd $(<D)
	@echo "  CXXLD \t $@"
	@$(SELF_CXX) $^ $(TEXTTOBIN_CXXLDFLAGS) -o $@

$(EXEC_DIR)/$(TARGET_TEXTTOBIN_W_LIBSO): $(TEXTTOBIN_OBJ)
	@mkdir -p $(@D)
	@cd $(<D)
	@echo "  CXXLD \t $@"
	@$(SELF_CXX) $^ $(TEXTTOBIN_CXXLDFLAGS) -o $@

run:
	@echo "Generating binary database..."
	@mkdir -p "swdb"
	@$(shell echo ./text-db/*.txt | xargs cat > ./swdb/swdb.txt)
	@../bin/exec/textToBin_ar -i ./swdb/swdb.txt -o ./swdb/swdb.bin
#	@sudo valgrind --leak-check=yes --leak-check=full --show-leak-kinds=all ../bin/exec/textToBin_ar -i ./swdb/swdb.txt -o ./swdb/swdb.bin