.PHONY : clean

CPPFLAGS=-fPIC -g --std=c++14 -I../mutils -L../mutils -DMAX_THREADS=10000
LDFLAGS=-shared 

SOURCES = FutureFreePool.cpp GlobalPool.cpp
HEADERS = FutureFreePool.hpp  ProcessPool.hpp  TaskPool.hpp  ThreadPool.hpp  ctpl_stl.h
OBJECTS=$(SOURCES:.cpp=.o)

TARGET=libmutils-tasks.so

all: $(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CXX) $(CFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

