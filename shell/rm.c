#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Missing parameters\n");
        exit(-1);
    }
    for (int i = 1; i < argc; i++) {
        if(remove(argv[i]) == 0) {
            printf("File deleted successfully\n");
        } else {
            printf("Unable to delete the file %s\n", argv[i]);
        }
    }

    return 0;
}