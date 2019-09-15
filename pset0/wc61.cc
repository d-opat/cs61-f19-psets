#include <cstdio>
#include <cctype>

int main() {
    unsigned long nc = 0, nw = 0, nl = 0;
    bool inspace = true;
    while (true) {
        int ch = fgetc(stdin);
        if (ch == EOF) {
            break;
        }
        ++nc;

        bool thisspace = isspace((unsigned char) ch);
        if (inspace && !thisspace) {
            ++nw;
        }
        inspace = thisspace;

        if (ch == '\n') {
            ++nl;
        }
    }
    fprintf(stdout, "%8lu %7lu %7lu\n", nl, nw, nc);
}



