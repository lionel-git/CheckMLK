
main : main.cpp checker.h
	clang++ -std=c++20 main.cpp -ldl -o main

clean:
	rm  -f *.o ./main
