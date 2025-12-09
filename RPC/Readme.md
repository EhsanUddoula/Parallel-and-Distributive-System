# 🧮 Matrix RPC Operator

A **distributed matrix operations system** using **Remote Procedure Calls (RPC)** that allows clients to perform complex matrix computations on a **remote server** efficiently.

---

## 🚀 Features

✅ **Matrix Addition** — Add two matrices of the same dimensions  
✅ **Matrix Multiplication** — Multiply dimensionally compatible matrices  
✅ **Matrix Transpose** — Transpose any matrix  
✅ **Matrix Inverse** — Compute the inverse of any square matrix (N×N)  
✅ **Multiple Client Support** — Handle concurrent client connections seamlessly  
✅ **Interactive Mode** — Simple and user-friendly interface for manual operations  
✅ **Automated Testing** — Comprehensive suite for validation and reliability  

---

## 📁 Project Structure

RPC/
├── bin/ # Compiled binaries
│ ├── matrixOp_server # Server executable
│ ├── matrixOp_client # Client executable
│ └── matrixOp_test # Test suite executable
├── obj/ # Object files
├── matrixOp.x # IDL interface definition
├── matrixOp.h # Generated header file
├── matrixOp_client.c # Client implementation
├── matrixOp_server.c # Server implementation
├── matrixOp_test.c # Test suite implementation
├── matrixOp_clnt.c # Generated client stub
├── matrixOp_svc.c # Generated server stub
├── matrixOp_xdr.c # Generated XDR routines
└── Makefile # Build configuration

---

## 🧩 Prerequisites

**Platform:** Ubuntu / Debian / WSL2 / Linux

### Installation

```bash
# Update package list
sudo apt update

# Install essential build tools and RPC libraries
sudo apt install build-essential gcc make
sudo apt install rpcgen libtirpc-dev

# Verify installation
rpcgen --version
gcc --version
```

## ⚙️ Building the Project

### Build all components

```bash
make
```

### Build and run tests

```bash
make check
```

### Clean build artifacts

```bash
make clean
```

## Run

```bash

# Terminal-1: Server
./bin/matrixOp_server

# Automated Test (Terminal 2)
# Run comprehensive test suite
./bin/matrixOp_test localhost

# Or using make
make run-test

# Launch interactive mode with menu
./bin/matrixOp_client localhost interactive

# Run predefined client-side test cases
./bin/matrixOp_client localhost test 
```
