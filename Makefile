CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2
SRC := src/task_manager.cpp
BIN := task_manager

.PHONY: all run clean

all: $(BIN)

$(BIN): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(BIN) $(SRC)

run: $(BIN)
	./$(BIN)

clean:
	rm -f $(BIN)
	rm -rf projects
