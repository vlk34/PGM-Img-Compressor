#include "compression.h"

#include <iostream>
#include <fstream>
#include <string>
#include "tools.h"

using namespace std;
using namespace tool;

img::PGMFile::PGMFile() // default constructor which initializes everything empty
{
    this->header = "";
    this->length = 0;
    this->pair_count = 0;
    this->data = nullptr;
    this->pixels = nullptr;
    this->values = nullptr;
}

img::PGMFile::PGMFile(string filename, string output, int tolerance) // overloaded constructor which basically makes compression
{
    this->compress_from(filename, output, tolerance);
}

img::PGMFile::~PGMFile() // destructor
{
    this->delete_all();
}

void img::PGMFile::resolution(string &line, int &width, int &height) // extract resolution from header. ex: "3 2" --> width = 3, height = 2
{
    string pixel = ""; // temp string

    for (unsigned int i = 0; i < line.length(); i++) // iterate all the characters of line
    {
        if (line[i] == ' ') // if space character found, transform temp string into number which will be width
        {
            width = stoi(pixel);
            pixel = ""; // make temp string empty
        }
        else
        {
            pixel += line[i]; // if there is no space character, add current character to temp string
        }
    }

    height = stoi(pixel); // what remains on the temp string is height
}

void img::PGMFile::run_length_encoding() // lossless compression with run length encoding algorithm
{
    unsigned char last = this->data[0]; // first pixel
    unsigned int counter = 1; // there are 1 instance of first pixel

    if (this->pair_count > 0) // just to make sure if there are already values inside arrays
    {
        delete[] this->pixels;
        delete[] this->values;
    }

    this->pair_count = 0; // there are zero pairs available right now, a pair contains info about how many of which char

    this->pixels = new unsigned char[this->length]; // since we did not count how many pairs there are, we assume that it can be as much as the pixel count
    this->values = new unsigned int[this->length]; // same as above

    for (unsigned int i = 1; i < this->length; i++) // iterating all pixels of the image
    {
        if (this->data[i] == last) // if the current pixel is equal to the pixel before, increment counter
        {
            counter++;
        }
        else // if the current pixel is not equal to the last pixel, save current pair info which is the amount of the pixel and the value of the pixel
        {
            this->values[this->pair_count] = counter; // how many pixel
            this->pixels[this->pair_count++] = last; // which pixel and increment pair count
            counter = 1; // there are one instance of the new pixel
            last = this->data[i]; // save the new pixel
        }
    }
    
    this->values[this->pair_count] = counter; // if we did not encounter any new pixel, remaining info is the last pair
    this->pixels[this->pair_count++] = last; // we save the last pair and increment pair count after saving this which equals to the actual pair count instead of index
}

void img::PGMFile::run_length_encoding(int tolerance) // lossy compression with modified run length encoding algorithm, it is the same as above except for the if statement in line 104
{
    unsigned char last = this->data[0];
    unsigned int counter = 1;

    if (this->pair_count > 0)
    {
        delete[] this->pixels;
        delete[] this->values;
    }

    this->pair_count = 0;

    this->pixels = new unsigned char[this->length];
    this->values = new unsigned int[this->length];

    for (unsigned int i = 1; i < this->length; i++)
    {
        if (this->data[i] <= fix_overflow(last, tolerance) && this->data[i] >= fix_underflow(last, tolerance)) // it checks if the pixel value is within the upper and lower values defined by tolerance. ex: if tolerance = 5; 200, 205, 195 are all 200 
        {
            counter++;
        }
        else
        {
            this->values[this->pair_count] = counter;
            this->pixels[this->pair_count++] = last;
            counter = 1;
            last = this->data[i];
        }
    }
    
    this->values[this->pair_count] = counter;
    this->pixels[this->pair_count++] = last;
}

void img::PGMFile::compress_from(string filename, string output, int tolerance)
{
    string line; // temp string

    ifstream file(filename, ios::binary); // input file

    if (!file.is_open()) // check if the file is not open
        throw runtime_error("Cannot open file!");

    getline(file, line); // get the first line which has PGM type

    if (line != "P5") // we can only read binary PGM files which are P5, if it is P2 which is ascii type PGM, we abort
        throw runtime_error("File is not P5!");

    this->header = "P5\n"; // start writing the header info

    getline(file, line); // read the next line which can be a comment or the resolution info

    while (line[0] == '#') // if line starts with # which specifies that this line is a comment, go inside the loop
        getline(file, line); // read another line and check if it starts with a comment as well
    
    this->header += line + '\n'; // when we skip all the comments, next line is the resolution info and we will save it to the header

    int width, height; // these variables will store width and height thus will be passed by reference since we cannot return two variables within one function

    resolution(line, width, height); // read resolution info from line and modify width and height variables

    getline(file, line); // this line tells us how many bits our pixels are which usually is 255 which means 8-bit PGM file
    this->header += line + '\n'; // save this info to header as well

    if (this->length > 0) // just making sure that there is no data inside pointer
        delete[] this->data;

    this->length = width * height; // pixel count is width multiplied by height
    this->data = new unsigned char[this->length]; // every pixel of an 8-bit PGM is 1 byte so we use unsigned char array to store pixels of the image

    file.read((char*) this->data, this->length); // all remaining bytes after the header are pixels therefore we read as much bytes as the pixel count
    file.close(); // we do not need the file anymore

    if (tolerance == 0) // if tolerance is 0, execute the normal function if not execute the overloaded function 
        this->run_length_encoding();
    else
        this->run_length_encoding(tolerance);

    ofstream writer("images/" + output + ".kvif", ios::binary); // output file

    if (!writer.is_open()) // check if the file is not open
        throw runtime_error("Cannot open file!");

    writer.write(this->header.data(), this->header.length()); // write the header as char array
    writer.write((char*) &(this->pair_count), 4); // write the pair count which is unsigned int
    writer.write((char*) this->values, this->pair_count * 4); // write all of the pixel counts which are unsigned int array which is 4 bytes each so we multiply pair count with 4
    writer.write((char*) this->pixels, this->pair_count); // write all of the pixel values which are unsigned char array which is 1 bytes each
    writer.close(); // we do not need the file anymore

    this->delete_all(); // delete everything to free memory
}

void img::PGMFile::decompress_to(string input, string filename)
{
    string line; // temp string

    ifstream file("images/" + input + ".kvif", ios::binary); // input file

    if (!file.is_open()) // check if the file is not open
        throw runtime_error("Cannot open file!");

    getline(file, line); // get the first line which has PGM type

    if (line != "P5") // our compressed image format only has P5 support so if it is not P5, abort
        throw runtime_error("File is not P5!");

    this->header = "P5\n"; // start writing the header info

    getline(file, line); // read the next line which is for sure the resolution info because our image format does not have comments
    
    this->header += line + '\n'; // write resolution info to the header

    int width, height; // these variables will store width and height thus will be passed by reference

    resolution(line, width, height); // read resolution info from line and modify width and height variables

    getline(file, line); // this line tells us how many bits our pixels are which usually is 255 which means 8-bit PGM file
    this->header += line + '\n'; // save this info to the header as well

    if (this->length > 0) // just making sure that there is no data inside pointer
        delete[] this->data;

    this->length = width * height; // pixel count is width multiplied by height
    this->data = new unsigned char[this->length]; // every pixel of an 8-bit PGM is 1 byte so we use unsigned char array to store pixels of the image

    file.read((char*) &(this->pair_count), 4); // first we read the pair count

    this->values = new unsigned int[this->pair_count]; // we are creating arrays exactly as long as the pair count
    this->pixels = new unsigned char[this->pair_count]; // same as above

    file.read((char*) this->values, this->pair_count * 4); // we are reading pair count multiplied by 4 bytes to unsigned integer array for obtaining the pixel counts
    file.read((char*) this->pixels, this->pair_count); // we are reading pixel values to unsigned char array for obtaining the pixel values 
    file.close(); // we do not need the file anymore

    int counter = 0; // keeping track of which pixel has been put into the data which contains unsigned char array of decompressed image pixels

    for (unsigned int i = 0; i < pair_count; i++) // check all pairs
        for (unsigned int j = 0; j < this->values[i]; j++) // put as much pixels as the pixel count
            this->data[counter++] = this->pixels[i]; // put the pixel value into decompressed image data and increment counter

    ofstream writer(filename, ios::binary); // output file

    if (!writer.is_open()) // check if the file is not open
        throw runtime_error("Cannot open file!");

    writer.write(this->header.data(), this->header.length()); // write the header bytes
    writer.write((char*) this->data, this->length); // write all of the pixels
    writer.close(); // we do not need the file anymore

    this->delete_all(); // delete everything to free memory
}

void img::PGMFile::delete_all()
{
    if (this->data == nullptr) // if it is already deleted, exit the function
        return;

    this->header = ""; // reset header
    this->length = 0; // zero pixels
    this->pair_count = 0; // zero pairs
    delete[] this->data; // delete pixels
    delete[] this->pixels; // delete pixel values
    delete[] this->values; // delete pixel counts
    this->data = nullptr; // make data nullptr to check if it is nullptr later on
    this->pixels = nullptr; // make pixel values nullptr just in case
    this->values = nullptr; // make pixel counts nullptr just in case
}