CFLAGS = -Wall
LDFLAGS = $(shell pkg-config --libs libhackrf)

all: ring

ring: ring.cpp
	$(CXX) ring.cpp -o ring -std=c++11 $(CFLAGS) $(LDFLAGS)
