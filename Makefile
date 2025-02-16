all: hike test

clean:
	rm hike
	rm test

hike: hike.cpp
	g++ -o hike hike.cpp

# -Iollama-hpp/singleheader -std=c++11
test: test.cpp ollama.hpp
	g++ -o test test.cpp