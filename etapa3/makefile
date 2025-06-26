CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra
TARGET = solver
SOURCES = main.cpp
HEADERS = Graph.hpp Solver.hpp

$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET) *.o *.txt *.dot *.png sol-*.dat

test: $(TARGET)
	./$(TARGET)

.PHONY: clean test