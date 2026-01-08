# Makefile for Task Management Server and Client

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread -I/usr/include/postgresql
LDFLAGS = -lpq -lssl -lcrypto -pthread

# Directories
SERVER_DIR = Server
CLIENT_DIR = Client
COMMON_DIR = Common
BUILD_DIR = build

# Server files
SERVER_SRC = $(SERVER_DIR)/server.cpp
SERVER_TARGET = $(BUILD_DIR)/server

# Client files
CLIENT_SRC = $(CLIENT_DIR)/client.cpp
CLIENT_TARGET = $(BUILD_DIR)/client

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build server
server: $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(SERVER_SRC) -o $(SERVER_TARGET) $(LDFLAGS)
	@echo "Server built successfully: $(SERVER_TARGET)"

# Build client
client: $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CLIENT_SRC) -o $(CLIENT_TARGET) $(LDFLAGS)
	@echo "Client built successfully: $(CLIENT_TARGET)"

# Build both
all: server client

# Clean build files
clean:
	rm -rf $(BUILD_DIR)
	rm -f *.log
	@echo "Clean complete"

# Run server
run-server: server
	./$(SERVER_TARGET)

# Run client
run-client: client
	./$(CLIENT_TARGET)

# Show help
help:
	@echo "Available targets:"
	@echo "  make all         - Build both server and client"
	@echo "  make server      - Build the server"
	@echo "  make client      - Build the client"
	@echo "  make run-server  - Build and run the server"
	@echo "  make run-client  - Build and run the client"
	@echo "  make clean       - Clean build files"
	@echo "  make help        - Show this help message"
	@echo ""
	@echo "Requirements:"
	@echo "  - PostgreSQL development libraries (libpq-dev)"
	@echo "  - OpenSSL development libraries (libssl-dev)"
	@echo "  - C++17 compiler"
	@echo ""
	@echo "Install dependencies on Ubuntu/Debian:"
	@echo "  sudo apt install g++ libpq-dev libssl-dev"

.PHONY: server client all clean run-server run-client help
