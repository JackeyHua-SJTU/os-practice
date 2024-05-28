target: fs fc

fs: $(filter-out src/FC.c src/main.c, $(wildcard src/*.c)) disc/src/disc.c
	gcc $^ -I include/ -I disc/include -o FS

fc: src/FC.c
	gcc $^ -o FC

clean:
	rm -f FS FC