.PHONY: test

all:
	clang++ -o test ./test.cc -std=c++1y -lpthread

test:
	./test

