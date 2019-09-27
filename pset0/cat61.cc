#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cerrno>


static void transfer(const char* filename) {
    // open input file
    FILE* in;
    
    // check for cl-named files, else use standard in
    if (strcmp(filename, "-") == 0) {
        in = stdin;
    } else {
        in = fopen(filename, "r");
    }

    // if no input is provided, show an error message
    if (!in) {
        fprintf(stderr, "%s: %s\n", filename, strerror(errno));
    }

    // transfer data to the standard output until end of file or error 
    while (!feof(in) && !ferror(in) && !ferror(stdout)) {
        // feof() check if the end of file indicator is set
        // returns a value other than zero if so
        char buf[BUFSIZ];
        size_t nr = fread(buf, 1, BUFSIZ, in);
        (void) fwrite(buf, 1, nr, stdout);
    }

    //exit on error
    if (ferror(in) || ferror(stdout)) {
        // ferror() check if the error indicator associated with a stream is set
        // returns a value other than zero if it is
        fprintf(stderr, "%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    //close input file if required
    if (in != stdin) {
        fclose(in);
    }
}



int main(int argc, char** argv) {
    if (argc == 1) {
        transfer("-");
    }

    for (int i = 1; i < argc; ++i) {
        transfer(argv[i]);
    }
}