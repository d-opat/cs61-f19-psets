

#include <cstring>
#include <cstdio>


int main(int arc, char** argv) {
    while (argc > 1) {
        //find the smallest argument
        int smallest = 1;
        for (int i = 2; i < argc; ++i) {
            if (strcmp(argv[i], argv[smallest]) < 0) {
                smallest = i;
            }
        }

        // print it
        fprintf(stdout, "%s\n", argv[smallest]);

        //remove it from the argument set
    }
}