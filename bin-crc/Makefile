.PHONY: all clean

EXECUTABLE := bin-crc
CXX ?= g++
CXXFLAGS = -O3 -g0 -std=c++20 -Wall -Wextra -Wpedantic

all: $(EXECUTABLE)

$(EXECUTABLE): main.cpp
	$(CXX) $(CXXFLAGS) $< -o $(EXECUTABLE)

clean:
	rm -rf $(EXECUTABLE)
