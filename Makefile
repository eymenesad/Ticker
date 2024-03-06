CXX=g++
CXXFLAGS=-std=c++17 -Wall
LIBS=-lcurl -pthread
TARGET=Ticker
SRC=Ticker.cpp
OBJ=$(SRC:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
run:
	./$(TARGET)
	
clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
