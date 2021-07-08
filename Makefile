.PHONY : all clean

COMPILER=g++ -std=c++11 -Ofast 
FLAGS=`root-config --glibs --cflags`

SRCS := $(wildcard *.cpp)
BINS := $(SRCS:%.cpp=%)

all: ${BINS}

%: %.cpp postprocessing.h
	${COMPILER} $< -o $@ ${FLAGS}

clean:
	rm ${BINS}
