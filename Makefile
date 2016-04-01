.PHONY : clean

CPPFLAGS=-fPIC -g --std=c++14
LDFLAGS=-shared 

SOURCES = FutureFreePool.cpp
HEADERS = FutureFreePool.hpp  ProcessPool.hpp  TaskPool.hpp  ThreadPool.hpp  ctpl_stl.h
OBJECTS=$(SOURCES:.cpp=.o)

TARGET=mutils-tasks.so

all: $(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CXX) $(CFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

