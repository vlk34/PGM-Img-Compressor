#include <iostream>
#include <string>
#include "compression.h"
#include "tools.h"

using namespace std;
using namespace img;
using namespace tool;

int main()
{
    int size; // number of args
    string line; // variable to store user input

    getline(cin, line); // get user input
    string *args = parse(line, size); // splitting user input by spaces to get args

    PGMFile file; // PGMFile object created with default constructor

    try
    {
        if (size == 2) // if there are two args then we have input file and output file
            file.compress_from(args[0], args[1], 0); // since tolerance is not specified, it is zero
        else if (size == 3)
            file.compress_from(args[0], args[1], stoi(args[2])); // third arg is tolerance
        else
            return 1; // there are not enough or too many arguments therefore we exit the program with error code 1

        file.decompress_to(args[1], args[0] + ".pgm"); // decompress the input file to output file

        delete[] args; // delete args since it is a dynamically allocated array
    }
    catch(const runtime_error &e) // error handling for wrong file type or cannot open file
    {
        cerr << e.what() << '\n';
        return 1; // there is something wrong with the input or the output file so we return exit code 1
    }

    return 0; // program ran successfully
}