#include <stdio.h>
#include <ctype.h>

int main() {
    int c, line = 0, word = 0, cnt = 0;
    int inside = 0;

    // getchar() read from stdin, fd = 0
    // pipe will redirect the file for us
    while ((c = getchar()) != EOF) {
        ++cnt;
        if (c == '\n') {
            ++line;
        }
        if (isspace(c)) {
            inside = 0;
        } else if (inside == 0) {
            inside = 1;
            ++word;
        }
    }

    printf("\t%d\t%d\t%d\n", line, word, cnt);

    return 0;
}