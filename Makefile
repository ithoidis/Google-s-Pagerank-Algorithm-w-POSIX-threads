
all:
	gcc pagerank.c -lpthread -O3

clean:
	rm -f a.out

run: all
	./a.out 10

