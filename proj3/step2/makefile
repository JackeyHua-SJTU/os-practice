target: fs fc

fs: $(filter-out src/FC.c, $(wildcard src/*.c))
	gcc $^ -I include/ -o FS

fc: src/FC.c
	gcc $^ -o FC

clean:
	rm -f FS FC