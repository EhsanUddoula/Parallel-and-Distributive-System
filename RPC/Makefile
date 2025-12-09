# Simple Makefile for Matrix RPC Project

# Parameters
CLIENT = matrixOp_client
SERVER = matrixOp_server
TEST = matrixOp_test
OBJ_DIR = obj
BIN_DIR = bin

# Source files
CLIENT_SRC = matrixOp_client.c
SERVER_SRC = matrixOp_server.c
TEST_SRC = matrixOp_test.c

# Generated files (by rpcgen)
GENERATED_SRC = matrixOp_clnt.c matrixOp_svc.c matrixOp_xdr.c
GENERATED_HDR = matrixOp.h

# Object files
CLIENT_OBJS = $(addprefix $(OBJ_DIR)/, matrixOp_client.o matrixOp_clnt.o matrixOp_xdr.o)
SERVER_OBJS = $(addprefix $(OBJ_DIR)/, matrixOp_server.o matrixOp_svc.o matrixOp_xdr.o)
TEST_OBJS = $(addprefix $(OBJ_DIR)/, matrixOp_test.o matrixOp_clnt.o matrixOp_xdr.o)

# Compiler flags
CFLAGS += -g -I/usr/include/tirpc
LDLIBS += -ltirpc
RPCGENFLAGS = -C

# Targets
all: $(BIN_DIR)/$(CLIENT) $(BIN_DIR)/$(SERVER) $(BIN_DIR)/$(TEST)

# Create directories
$(OBJ_DIR) $(BIN_DIR):
	@mkdir -p $@

# Compile object files
$(OBJ_DIR)/%.o: %.c $(GENERATED_HDR) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Build binaries
$(BIN_DIR)/$(CLIENT): $(CLIENT_OBJS) | $(BIN_DIR)
	$(LINK.c) -o $@ $(CLIENT_OBJS) $(LDLIBS)

$(BIN_DIR)/$(SERVER): $(SERVER_OBJS) | $(BIN_DIR)
	$(LINK.c) -o $@ $(SERVER_OBJS) $(LDLIBS)

$(BIN_DIR)/$(TEST): $(TEST_OBJS) | $(BIN_DIR)
	$(LINK.c) -o $@ $(TEST_OBJS) $(LDLIBS) -lm

# Clean - only removes obj and bin directories
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Run targets
run-server: $(BIN_DIR)/$(SERVER)
	./$(BIN_DIR)/$(SERVER)

run-client: $(BIN_DIR)/$(CLIENT) 
	./$(BIN_DIR)/$(CLIENT) localhost test

run-test: $(BIN_DIR)/$(TEST)
	./$(BIN_DIR)/$(TEST) localhost

# Test with different server addresses
test-local: $(BIN_DIR)/$(TEST)
	./$(BIN_DIR)/$(TEST) localhost

test-remote: $(BIN_DIR)/$(TEST)
	@echo "Usage: make test-remote SERVER=192.168.1.100"
	@echo "Then run: ./bin/matrixOp_test \$$SERVER"

# Build and run all tests
check: all run-test

# Debug info
debug:
	@echo "CLIENT_OBJS: $(CLIENT_OBJS)"
	@echo "SERVER_OBJS: $(SERVER_OBJS)"
	@echo "TEST_OBJS: $(TEST_OBJS)"
	@echo "BIN_DIR: $(BIN_DIR)"
	@echo "OBJ_DIR: $(OBJ_DIR)"

.PHONY: all clean run-server run-client run-test test-local test-remote check debug