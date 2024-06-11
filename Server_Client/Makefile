CXX = g++
CXXFLAGS = -std=c++20 -pthread -Wall -Wextra -g

# List of source files
SRCS = p4.cpp

# Generate list of object files by replacing .cpp extension with .o
OBJS = $(SRCS:.cpp=.o)

# Default target
all: p4

# Compile p4.cpp into p4.o
p4.o: p4.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link object files into executable
p4: $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Clean up intermediate object files and executable
clean:
	rm -f *.o p4
