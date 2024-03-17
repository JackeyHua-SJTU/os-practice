all: single multiple

single: MergesortSingle.c
	gcc -o MergesortSingle MergesortSingle.c

multiple: MergesortMulti.c
	gcc -o MergesortMulti MergesortMulti.c -lpthread

clean:
	rm -f MergesortSingle MergesortMulti