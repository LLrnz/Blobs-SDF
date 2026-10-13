CXX ?= g++
CXXFLAGS = -g -std=c++17 -Wall -Wextra -pedantic

# Raylib
RAYLIB_CFLAGS = $(shell pkg-config --cflags raylib)
RAYLIB_LIBS = $(shell pkg-config --libs raylib)

# Paths
SRC_PATH = src
TEST_PATH = test
INCLUDE_PATH = include
BUILD_PATH = build
BIN_PATH = $(BUILD_PATH)/bin

# Executables
BIN_NAME = app
TEST_NAME = tests

# Source files
SOURCES = $(shell find $(SRC_PATH) -name '*.cpp')
TEST_SOURCES = $(shell find $(TEST_PATH) -name '*.cpp')
HEADERS = $(shell find $(INCLUDE_PATH) -name '*.h' -o -name '*.hpp')

# Application objects
OBJECTS = $(SOURCES:$(SRC_PATH)/%.cpp=$(BUILD_PATH)/%.o)

# Test objects
TEST_OBJECTS = $(TEST_SOURCES:$(TEST_PATH)/%.cpp=$(BUILD_PATH)/test/%.o)

# Remove src/main.o from objects used by tests
APP_OBJECTS_FOR_TESTS = $(filter-out $(BUILD_PATH)/main.o,$(OBJECTS))

# Dependencies
DEPS = $(OBJECTS:.o=.d) $(TEST_OBJECTS:.o=.d)

# Includes
INCLUDES = \
	-I $(realpath $(INCLUDE_PATH)) \
	-I /usr/local/include \
	-I /usr/local/Cellar/catch2/3.16.0/include \
	-I /home/linuxbrew/.linuxbrew/Cellar/catch2/3.16.0/include

# Libraries
TEST_LIBS = \
	-L/usr/local/Cellar/catch2/3.16.0/lib \
	-lCatch2

.PHONY: default_target release dirs clean all test

default_target: release

release: dirs
	@$(MAKE) all

dirs:
	@mkdir -p $(dir $(OBJECTS))
	@mkdir -p $(dir $(TEST_OBJECTS))
	@mkdir -p $(BIN_PATH)

clean:
	@$(RM) $(BIN_NAME)
	@$(RM) -r $(BUILD_PATH)

all: $(BIN_PATH)/$(BIN_NAME)
	@$(RM) $(BIN_NAME)
	@ln -s $(BIN_PATH)/$(BIN_NAME) $(BIN_NAME)

test: $(BIN_PATH)/$(TEST_NAME)
	@$(BIN_PATH)/$(TEST_NAME)

# Application executable
$(BIN_PATH)/$(BIN_NAME): $(OBJECTS)
	@echo "Linking: $@"
	$(CXX) $(OBJECTS) -o $@ $(RAYLIB_LIBS)

# Test executable
$(BIN_PATH)/$(TEST_NAME): $(TEST_OBJECTS) $(APP_OBJECTS_FOR_TESTS)
	@echo "Linking tests: $@"
	$(CXX) $(TEST_OBJECTS) $(APP_OBJECTS_FOR_TESTS) -o $@ $(TEST_LIBS)

# Header changes trigger recompilation of affected source files.
$(OBJECTS) $(TEST_OBJECTS): $(HEADERS)

# Automatically generated header dependencies
-include $(DEPS)

# Compile application sources
$(BUILD_PATH)/%.o: $(SRC_PATH)/%.cpp
	@echo "Compiling: $< -> $@"
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(RAYLIB_CFLAGS) -MP -MMD -c $< -o $@

# Compile test sources
$(BUILD_PATH)/test/%.o: $(TEST_PATH)/%.cpp
	@echo "Compiling test: $< -> $@"
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MP -MMD -c $< -o $@