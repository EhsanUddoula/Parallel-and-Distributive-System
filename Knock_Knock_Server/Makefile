# Platform detection
ifeq ($(OS),Windows_NT)
    # Windows settings
    CC = gcc
    CFLAGS = -Wall -Wextra -std=c99 -I./include -D_WIN32_WINNT=0x0600
    LDFLAGS = -lws2_32 -lsqlite3
    SERVER_TARGET = $(BIN_DIR)/server.exe
    CLIENT_TARGET = $(BIN_DIR)/client.exe
    TEST_TARGET = $(BIN_DIR)/test_server.exe
    RM = del /Q
    MKDIR = mkdir
    RMDIR = rmdir /S /Q
    RUN_SERVER = .\$(SERVER_TARGET)
    RUN_CLIENT = .\$(CLIENT_TARGET)
    RUN_TEST = .\$(TEST_TARGET)
else
    # Linux settings
    CC = gcc
    CFLAGS = -Wall -Wextra -std=c99 -pthread -I./include
    LDFLAGS = -lpthread -lsqlite3
    SERVER_TARGET = $(BIN_DIR)/server
    CLIENT_TARGET = $(BIN_DIR)/client
    TEST_TARGET = $(BIN_DIR)/test_server
    RM = rm -rf
    MKDIR = mkdir -p
    RMDIR = rm -rf
    RUN_SERVER = ./$(SERVER_TARGET)
    RUN_CLIENT = ./$(CLIENT_TARGET)
    RUN_TEST = ./$(TEST_TARGET)
endif

# Directories
SRC_DIR = .
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin

# Source files
SERVER_SRC = $(SRC_DIR)/server.c
CLIENT_SRC = $(SRC_DIR)/client.c
TEST_SRC = test_server.c

# Header files
SERVER_HEADERS = $(INCLUDE_DIR)/server.h
CLIENT_HEADERS = $(INCLUDE_DIR)/client.h

# Default target
all: $(SERVER_TARGET) $(CLIENT_TARGET)

# Create server executable
$(SERVER_TARGET): $(SERVER_SRC) $(SERVER_HEADERS) | $(BIN_DIR)
	@echo "Compiling server..."
	$(CC) $(CFLAGS) -o $@ $(SERVER_SRC) $(LDFLAGS)
	@echo "Server built successfully: $@"

# Create client executable
$(CLIENT_TARGET): $(CLIENT_SRC) $(CLIENT_HEADERS) | $(BIN_DIR)
	@echo "Compiling client..."
	$(CC) $(CFLAGS) -o $@ $(CLIENT_SRC) $(LDFLAGS)
	@echo "Client built successfully: $@"

# Create test executable
$(TEST_TARGET): $(TEST_SRC) | $(BIN_DIR)
	@echo "Compiling test suite..."
	$(CC) $(CFLAGS) -o $@ $(TEST_SRC) $(LDFLAGS)

# Create necessary directories
$(BIN_DIR):
	$(MKDIR) $(BIN_DIR)

$(BUILD_DIR):
	$(MKDIR) $(BUILD_DIR)

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	$(RMDIR) $(BIN_DIR) $(BUILD_DIR)
	@echo "Clean complete"

# Run server (convenience target)
run-server: $(SERVER_TARGET)
	@echo "Starting server..."
	$(RUN_SERVER)

# Run client (convenience target)
run-client: $(CLIENT_TARGET)
	@echo "Starting client..."
	$(RUN_CLIENT)

# Test target
test: all $(TEST_TARGET)
	@echo "Running test suite..."
	$(RUN_TEST)

# Show project structure
tree:
	@echo "Project structure:"
ifeq ($(OS),Windows_NT)
	@echo "Knock_Knock_Server/"
	@echo "├── include/"
	@echo "│   ├── server.h"
	@echo "│   └── client.h"
	@echo "├── server.c"
	@echo "├── client.c"
	@echo "├── jokes.txt"
	@echo "└── Makefile"
else
	@tree -I 'build|bin' 2>/dev/null || echo "Knock_Knock_Server/"
	@echo "├── include/"
	@echo "│   ├── server.h"
	@echo "│   └── client.h"
	@echo "├── server.c"
	@echo "├── client.c"
	@echo "├── jokes.txt"
	@echo "└── Makefile"
endif

# Help target
help:
	@echo "Available targets:"
	@echo "  all       - Build both server and client (default)"
	@echo "  server    - Build only the server"
	@echo "  client    - Build only the client"
	@echo "  test      - Build and run tests"
	@echo "  clean     - Remove all build artifacts"
	@echo "  run-server- Build and run the server"
	@echo "  run-client- Build and run the client"
	@echo "  tree      - Show project structure"
	@echo "  help      - Show this help message"

# Platform-specific targets
ifeq ($(OS),Windows_NT)
# Windows-specific targets
install:
	@echo "Windows installation - copy binaries manually"

uninstall:
	@echo "Windows uninstallation - remove binaries manually"

else
# Linux-specific targets
install: $(SERVER_TARGET) $(CLIENT_TARGET)
	@echo "Installing binaries to /usr/local/bin/ (requires sudo)"
	sudo cp $(SERVER_TARGET) /usr/local/bin/knock-server
	sudo cp $(CLIENT_TARGET) /usr/local/bin/knock-client
	@echo "Installation complete"

uninstall:
	@echo "Uninstalling binaries (requires sudo)"
	sudo rm -f /usr/local/bin/knock-server /usr/local/bin/knock-client
	@echo "Uninstall complete"
endif

# Phony targets
.PHONY: all clean run-server run-client test tree help install uninstall

# Aliases for convenience
server: $(SERVER_TARGET)
client: $(CLIENT_TARGET)