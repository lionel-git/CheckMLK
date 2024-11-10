
main : main.cpp checker.h
	g++ -std=c++20 main.cpp -o main

clean:
	rm  -f *.o ./main
