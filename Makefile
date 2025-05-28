CXX = g++
CXXFLAGS = -std=c++17 -O3 -I./include -w

# Targets
TARGET_MAIN = main
TARGET_INSERT = insert
TARGET_DELETE = delete

# Common source files
SRC_COMMON = src/*.cpp

# Source files for each target
SRC_MAIN = $(SRC_COMMON) test/main.cpp
SRC_INSERT = $(SRC_COMMON) test/insert.cpp
SRC_DELETE = $(SRC_COMMON) test/delete.cpp

# Default target: build all
all: $(TARGET_MAIN) $(TARGET_INSERT) $(TARGET_DELETE)

# Build main
$(TARGET_MAIN): $(SRC_MAIN)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Build insert
$(TARGET_INSERT): $(SRC_INSERT)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Build delete
$(TARGET_DELETE): $(SRC_DELETE)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Clean
clean:
	@echo "Cleaning up..."
	-@rm -f *.o *.gcno *~ $(TARGET_MAIN) $(TARGET_INSERT) $(TARGET_DELETE) 2>/dev/null || true

.PHONY: all clean
