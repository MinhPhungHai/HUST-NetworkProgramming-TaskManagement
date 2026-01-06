CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++17 `pkg-config --cflags gtk+-3.0`
LDFLAGS := `pkg-config --libs gtk+-3.0`

SRC := start.cpp app_state.cpp login.cpp register.cpp project_list.cpp project_view.cpp settings.cpp gantt.cpp
BUILD_DIR := build
OBJ := $(addprefix $(BUILD_DIR)/,$(SRC:.cpp=.o))
TARGET := $(BUILD_DIR)/start

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
