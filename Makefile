CXX = g++
CXXFLAGS = -Wall -std=c++17
INCLUDES = 
LIBS = 

TARGET = fruit_game
SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	del *.o $(TARGET).exe
