all: shell server ls wc cat pwd rm

shell: shell.c
	gcc shell.c -o shell

server: shell_server.c
	gcc shell_server.c -o server -Wno-format-truncation -Wno-format

ls: ls.c
	gcc ls.c -o ls -Wno-format

wc: wc.c
	gcc wc.c -o wc

cat: cat.c
	gcc cat.c -o cat

pwd: pwd.c
	gcc pwd.c -o pwd

rm: rm.c
	gcc rm.c -o rm

clean:
	rm -f shell server ls wc pwd cat rm
