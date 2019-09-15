#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

int main(int c, char** argv) {
    std::vector<std::string> args(&argv[1], &argv[argc]); // vector using range intialization
    std::sort(args.begin(), args.end());
    for (auto& s : args) {
        std::cout << s << '\n';
    }
}