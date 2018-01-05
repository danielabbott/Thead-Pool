USE_CPP=0
COMPILE_PARTICLE_SYSTEM_DEMO=1

CC=gcc
CXX=g++

ifeq ($(USE_CPP),0)
LINKER=$(CC)
else
LINKER=$(CXX)
endif


SOURCES_C=ThreadPool.c

ifeq ($(COMPILE_PARTICLE_SYSTEM_DEMO), 1)
SOURCES_C+=psysdemo.c
endif

ifeq ($(USE_CPP),0)
SOURCES_C+=linux_thread.c linux_mutex.c linux_cv.c
SOURCES_CPP=
else
SOURCES_CPP=cpp11_thread.cpp cpp11_mutex.cpp cpp11_cv.cpp
endif

OBJECTS_C=$(SOURCES_C:.c=.o)
OBJECTS_CPP=$(SOURCES_CPP:.cpp=.o)

# Add '-DNDEBUG' to disable debug assertations
FLAGS=-c -Wall -Wextra -O3 -Wpedantic -fstack-protector-all
CFLAGS=-std=gnu11 $(FLAGS)
CXXFLAGS=-std=c++11 $(FLAGS)

LDFLAGS=-O3 -lpthread

ifeq ($(COMPILE_PARTICLE_SYSTEM_DEMO), 1)
all: $(OBJECTS_C) $(OBJECTS_CPP) psysdemo.o
	$(LINKER) $(OBJECTS_C) $(OBJECTS_CPP) -o particle_system_demo `pkg-config --libs glfw3` -lGL $(LDFLAGS)
else
all: $(OBJECTS_C) $(OBJECTS_CPP)
endif

.cpp.o:
	$(CXX) $(CXXFLAGS) $< -o $@

psysdemo.o: psysdemo.c
	$(CC) $(CFLAGS) -msse psysdemo.c -o psysdemo.o

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o

