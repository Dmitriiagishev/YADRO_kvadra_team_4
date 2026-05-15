CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O0
LDFLAGS = -lpthread

TARGET = crawler_server
SOURCES = crawler.cpp server_side.cpp
OBJECTS = $(SOURCES:.cpp=.o)

.PHONY: all clean rebuild

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

rebuild: clean all