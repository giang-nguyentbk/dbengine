DATABASEIF_DIR		:= $(SW_DIR)/databaseif
DATABASEIF_SRC_DIR	:= $(DATABASEIF_DIR)/src

DATABASEIF_CXXFLAGS	:= -L$(LIB_DIR) -L$(SDK_LIB_DIR) -ltraceif
# DATABASEIF_LIBAR	:= libdatabaseifa.a # Static libary
DATABASEIF_LIBSO	:= libdatabaseif.so # Dynamic libary

DATABASEIF_SRCS		=
DATABASEIF_SRCS		+= databaseImpl.cc

DATABASEIF_OBJS		:= $(DATABASEIF_SRCS:%.cc=$(OBJ_DIR)/%.o)
REQUIRED_OBJS		:= $(OBJ_DIR)/dbLoader.o

DATABASEIF_INCS		:= \
			-I$(DATABASEIF_DIR)/if \
			-I$(DATABASEIF_DIR)/inc \
			-I$(SW_DIR)/dbloader/inc \
			-I$(SW_DIR)/common \
			-I$(SDK_INC_DIR)

# all: $(DATABASEIF_OBJS) $(LIB_DIR)/$(DATABASEIF_LIBAR) $(LIB_DIR)/$(DATABASEIF_LIBSO) install-header-files-databaseif
all: $(DATABASEIF_OBJS) $(LIB_DIR)/$(DATABASEIF_LIBSO) install-header-files-databaseif

# Build target 1 objects
$(OBJ_DIR)/%.o: $(DATABASEIF_SRC_DIR)/%.cc
	@mkdir -p $(@D)
	@cd $(<D)
	@echo "  CXX \t\t $@"
	@$(SELF_CXX) $(SELF_LDFLAGS) $(SELF_CFLAGS) $(DATABASEIF_INCS) $^ -o $@

##### CANNOT BUILD STATIC LIBRARY FOR LEVEL-2 LIBRARY OR UPWARD #####
##### TO DO THIS, COMBINE THIS LIBRARY'S OBJECTS WITH ALL OTHER LEVEL-1 OBJECT BY YOURSELF WITH 'AR' COMMAND #####
##### OR JUST USING GNU-LIBTOOL OR SIMILARITIES INSTEAD #####
# $(LIB_DIR)/$(DATABASEIF_LIBAR): $(DATABASEIF_OBJS) $(REQUIRED_OBJS)
# 	@mkdir -p $(@D)
# 	@cd $(<D)
# 	@echo "  AR \t\t $@"
# 	@$(SELF_AR) $(SELF_ARFLAGS) $@ $^

$(LIB_DIR)/$(DATABASEIF_LIBSO): $(DATABASEIF_OBJS) $(REQUIRED_OBJS)
	@mkdir -p $(@D)
	@cd $(<D)
	@echo "  CXXLD \t $@"
	@$(SELF_CXX) $(SELF_SOFLAGS) $^ $(DATABASEIF_CXXFLAGS) -o $@

install-header-files-databaseif:
	@mkdir -p $(INC_DIR)
	@echo "  COPY \t\t $(DATABASEIF_DIR)/if"
	@$(SELF_CPY) $(DATABASEIF_DIR)/if/*.h $(INC_DIR)

clean-databaseif:
	@echo "  RMV \t\t $(BIN_DIR)/databaseif"
	@$(SELF_RMV) $(DATABASEIF_OBJS) $(LIB_DIR)/$(DATABASEIF_LIBSO)
	@$(SELF_RMV) $(INC_DIR)/databaseIf.h