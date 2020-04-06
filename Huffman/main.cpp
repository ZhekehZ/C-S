#include <string>
#include <fstream>

#include "huffman.hpp"

using namespace std;

const string USAGE{
    "Usage:\n"
    "    ./huffman [-v] OPTION SOURCE DEST\n"
    "\n"
    "DESCRIPTION\n"
    "    Encodes and decodes a file using the Huffman algorithm.\n"
    "\n"
    "OPTIONS\n"
    "    -c\n"
    "        encode SOURCE and save to DEST\n"
    "    -d\n"
    "        decode SOURCE and save to DEST\n"
    "    -v\n"
    "        display the encoding table\n"
};

int main(int argc, char** argv) {
    if (argc < 4 || argc > 5) {
        cout << USAGE;
        return 1;
    }
    const bool verbose = string(argv[1]) == "-v";

    if (argc != (verbose ? 5 : 4)) {
        cout << USAGE;
        return 1;
    }

    const int n_cmd = verbose ? 2 : 1;
    const int n_arg1 = verbose ? 3 : 2;
    const int n_arg2 = verbose ? 4 : 3;

    if (string(argv[n_cmd]) == "-c") {
        std::ifstream fin(argv[n_arg1], std::ios::binary);
        std::ofstream fout(argv[n_arg2], std::ios_base::binary);
        encode(fin, fout, verbose);
    } else if (string(argv[n_cmd]) == "-d") {
        std::ifstream fin(argv[n_arg1], std::ios::binary);
        std::ofstream fout(argv[n_arg2], std::ios_base::binary);
        decode(fin, fout, verbose);
    } else {
        cout << USAGE;
        return 1;
    }

    return 0;
}
