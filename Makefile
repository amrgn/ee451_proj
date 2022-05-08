CXX = g++
CXXFlags = -lrt -lpthread -std=c++11

all: test_fft test_strong_fft

test_strong_fft: test_strong_fft.cpp strong_fft.o util.o strong_fft.h
	$(CXX) $(CXXFlags) test_strong_fft.cpp strong_fft.o util.o -o test_strong_fft

strong_fft.o: strong_fft.cpp util.h strong_fft.h
	$(CXX) $(CXXFlags) strong_fft.cpp -c -o strong_fft.o

test_fft: test_fft.cpp fft.o util.o fft.h
	$(CXX) $(CXXFlags) test_fft.cpp fft.o util.o -o test_fft

fft.o: fft.cpp util.h fft.h
	$(CXX) $(CXXFlags) fft.cpp -c -o fft.o

util.o: util.cpp util.h
	$(CXX) $(CXXFlags) util.cpp -c -o util.o

.PHONY: clean all
clean:
	rm -f test_fft test_strong_fft fft.o util.o strong_fft.o