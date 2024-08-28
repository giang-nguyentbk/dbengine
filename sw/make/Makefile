# --------------------------------------------------
# Call Makefiles for init setups and build database
# --------------------------------------------------
include Makefile.config

# --------------------------------------------------
# Call Makefiles for modules of the project
# --------------------------------------------------
include $(SW_DIR)/texttobin/Makefile
include $(SW_DIR)/dbloader/Makefile
include $(SW_DIR)/databaseif/Makefile




all: rootMakefile

rootMakefile:
	@echo "sw/make/Makefile: Compilation $(PROJ_NAME) done!!! [OK]"

clean:
	@echo "  RMV \t\t $(BIN_DIR)"
	@$(SELF_RMV) $(BIN_DIR)