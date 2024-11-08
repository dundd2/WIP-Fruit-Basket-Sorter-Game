CXX = g++
CXXFLAGS = -Wall -std=c++17
INCLUDES = -IC:/msys64/mingw64/include -IC:/msys64/mingw64/include/SDL2
LIBS = -LC:/msys64/mingw64/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf

TARGET = fruit_game
SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	del *.o $(TARGET).exe