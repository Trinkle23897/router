SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.o)
HEADERS := $(SRCS:.cpp=.h)

EXE := main

CC := ${CROSS_COMPILE}g++
CFLAGS := -O2 -g -Wall

all: $(EXE)


$(EXE): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -pthread

%.o: %.c %.h
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -rf $(EXE) $(OBJS)