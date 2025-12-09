# Compiler and flags
CC = gcc
MPICC = mpicc
CFLAGS = -Wall -O2 -lm
MPI_FLAGS = -Wall -O2 -lm
BIN_DIR = bin

# Targets
TARGETS = sequential parallel spawned spawned_worker
ALL_TARGETS = $(TARGETS) clean run_sequential run_parallel run_spawned run_all

# Default total points (can be overridden)
TOTAL_POINTS ?= 100000000

# Source files
SOURCES = sequential.c parallel.c spawned.c spawned_worker.c

.PHONY: all clean run_sequential run_parallel run_spawned run_all test_small help

# Default target
all: $(TARGETS)

# Sequential version (regular C)
sequential: sequential.c | $(BIN_DIR)
	$(CC) -DTOTAL_POINTS=$(TOTAL_POINTS) $(CFLAGS) -o $(BIN_DIR)/$@ $<

# Parallel version (MPI)
parallel: parallel.c | $(BIN_DIR)
	$(MPICC) -DTOTAL_POINTS=$(TOTAL_POINTS) $(MPI_FLAGS) -o $(BIN_DIR)/$@ $<

# Dynamic spawning master
spawned: spawned.c | $(BIN_DIR)
	$(MPICC) -DTOTAL_POINTS=$(TOTAL_POINTS) $(MPI_FLAGS) -o $(BIN_DIR)/$@ $<

# Dynamic spawning worker
spawned_worker: spawned_worker.c | $(BIN_DIR)
	$(MPICC) -DTOTAL_POINTS=$(TOTAL_POINTS) $(MPI_FLAGS) -o $(BIN_DIR)/$@ $<

# Ensure bin directory exists
$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

# Run sequential version
run_sequential: sequential
	@echo "=========================================="
	@echo "Running Sequential Version"
	@echo "=========================================="
	./$(BIN_DIR)/sequential

# Run parallel versions with different process counts
run_parallel: parallel
	@echo "=========================================="
	@echo "Running Parallel Versions"
	@echo "=========================================="
	@for processes in 2 4 6 8; do \
		echo "Testing with $$processes processes..."; \
		mpirun --oversubscribe -np $$processes ./$(BIN_DIR)/parallel; \
		echo "------------------------------------------"; \
	done

# Run dynamic spawning versions
run_spawned: spawned spawned_worker
	@echo "=========================================="
	@echo "Running Dynamic Spawning Versions"
	@echo "=========================================="
	@for workers in 2 4 6 8; do \
		echo "Testing with $$workers dynamically spawned workers..."; \
		mpirun --oversubscribe -np 1 ./$(BIN_DIR)/spawned $$workers; \
		echo "------------------------------------------"; \
	done

# Run all experiments
run_all: run_sequential run_parallel run_spawned
	@echo "All experiments completed!"

# Test with smaller number of points
test_small:
	@$(MAKE) TOTAL_POINTS=1000000 run_all

# Clean up compiled files
clean:
	rm -f $(BIN_DIR)/* *.o

# Display help
help:
	@echo "Available targets:"
	@echo "  all              - Build all programs (default)"
	@echo "  sequential       - Build only sequential version"
	@echo "  parallel         - Build only parallel version"
	@echo "  spawned          - Build dynamic spawning master"
	@echo "  spawned_worker   - Build dynamic spawning worker"
	@echo "  run_sequential   - Run sequential version"
	@echo "  run_parallel     - Run parallel versions (2,4,6,8 processes)"
	@echo "  run_spawned      - Run dynamic spawning versions (2,4,6,8 workers)"
	@echo "  run_all          - Run all experiments"
	@echo "  test_small       - Run all experiments with smaller point count (1M)"
	@echo "  clean            - Remove compiled files"
	@echo "  help             - Show this help message"
	@echo ""
	@echo "Environment variables:"
	@echo "  TOTAL_POINTS     - Set number of points (default: 100000000)"
	@echo ""
	@echo "Examples:"
	@echo "  make                    # Build all programs"
	@echo "  make run_all            # Build and run all experiments"
	@echo "  make test_small         # Quick test with 1M points"
	@echo "  make TOTAL_POINTS=50000000 run_parallel  # Custom point count"