directories:
	@if [ ! -d $(BINARY_DIRECTORY) ]; then mkdir -p $(BINARY_DIRECTORY); echo "WRITE | $(BINARY_DIRECTORY)"; fi
	@if [ ! -d $(BINARY_OBJECT_DIRECTORY) ]; then mkdir -p $(BINARY_OBJECT_DIRECTORY); echo "WRITE | $(BINARY_OBJECT_DIRECTORY)"; fi
	@if [ ! -d $(BINARY_PLATFORM_DIRECTORY) ]; then mkdir -p $(BINARY_PLATFORM_DIRECTORY); echo "WRITE | $(BINARY_PLATFORM_DIRECTORY)"; fi

$(BINARY_OBJECT_DIRECTORY)/%.o: $(PROGRAM_SOURCE_DIRECTORY)/%.cpp
	@$(CXX) $(CXX_FLAGS) $(WARNINGS) $(INCLUDES) $(SYSTEM_INCLUDES) -c $< -o $@
	@echo "CXX   | $@"
-include $(DEPENDENCY_FILES)

$(OUTPUT_FILE): $(OBJECT_FILES)
	@$(CXX) $(CXX_FLAGS) $(WARNINGS) $(INCLUDES) $(SYSTEM_INCLUDES) $^ $(LIBRARIES) -o $@
	@echo "LINK  | $@"

delete:
	@if [ -d $(BINARY_OBJECT_DIRECTORY) ]; then rm -r $(BINARY_OBJECT_DIRECTORY); echo "RM    | $(BINARY_OBJECT_DIRECTORY)"; fi
	@if [ -f $(OUTPUT_FILE) ]; then rm -r $(OUTPUT_FILE); echo "RM    | $(OUTPUT_FILE)"; fi
