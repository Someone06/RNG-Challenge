sequential:
	g++ -O3 -std=c++20 -Wall -Wextra random.cpp


parallel:
	g++ -O3 -std=c++20 -fopenmp -Wall -Wextra random_parallel.cpp
