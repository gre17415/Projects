# Makefile for Nuclear Framework Echo Lab with GTest
# Students compile their client implementation with the hidden test

CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -Werror -pedantic
INCLUDES = -I/opt/nuclear/include -I/opt/nuclear/build/_deps/googletest-src/googletest/include
LDFLAGS = -L/opt/nuclear/build -lnuclear -pthread
GTEST_LIBS = -L/opt/nuclear/build/lib -lgtest -lgtest_main

TARGET = echo_lab
OBJS = echo_client.o test_echo_lab.o

.PHONY: all clean test

all: $(TARGET)
	./$(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(GTEST_LIBS)

echo_client.o: echo_client.cpp echo_client.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c echo_client.cpp -o echo_client.o

test_echo_lab.o: test_echo_lab.cpp echo_client.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c test_echo_lab.cpp -o test_echo_lab.o

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)
