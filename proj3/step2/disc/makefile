target: server client_cli client_rand
server: src/BDS.c src/disc.c
	gcc $^ -o BDS -I include/ -Wall

client_cli: src/BDC_CLI.c
	gcc $^ -o BDC_CLI -I include/ -Wall

client_rand: src/BDC_RAND.c
	gcc $^ -o BDC_RAND -I include/ -Wall
clean:
	rm -f BDS *.txt BDC_CLI BDC_RAND