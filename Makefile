test : main.o Graph.o SkipList.o threadpool.o
	g++ -o test main.o Graph.o SkipList.o threadpool.o -pthread
	
main.o : main.cpp
	g++ -c main.cpp -pthread
	
SkipList.o : SkipList.h
	g++ -c SkipList.cpp 
Graph.o : Graph.cpp Graph.h
	g++ -c Graph.cpp
threadpool.o : threadpool.h
	g++ -c threadpool.cpp
	
clean: 
	rm test main.o Graph.o SkipList.o threadpool.o