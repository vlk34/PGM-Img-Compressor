#include "tools.h"

string* tool::parse(string &str, int &size) // parse args from user input
{
    size = 0; // arg size counter
    int counter = 1; // space character counter which starts from 1 because space count + 1 is usually the word count
    for (int i = 0; i < str.length(); i++) // count how many space characters there are
        if (str[i] == ' ')
            counter++;

    string* arr = new string[counter]; // assume that there are as much words as the space character count + 1

    string temp = ""; // temp string

    for (int i = 0; i < str.length(); i++) // iterate all of the characters of input string
    {
        if (str[i] == ' ' && temp.length() > 0) // if we encounter space character and we have contents in temp string, we add this string to the string array of args
        {
            arr[size++] = temp; // when we add something, size becomes the word count. ex: size = 0, arr[size++] is array[0] = temp and size = 1
            temp = ""; // empty temp
        }
        else
        {
            temp += str[i]; // add current character to temp string
        }
    }

    if (temp.length() > 0) // if there are remaining things in the temp string, they are the last argument
        arr[size++] = temp; // save the last argument and since size is passed by reference, it already is modified outside the function

    return arr; // return arguments' string array
}

unsigned char tool::fix_overflow(unsigned char c, unsigned char x)
{
    return (255 - c < x) ? 255 : c + x; // if pixel plus tolerance is above 255, stop at 255 so it does not overflow
}

unsigned char tool::fix_underflow(unsigned char c, unsigned char x)
{
    return (c < x) ? 0 : c - x; // if pixel minus tolerance is below 0, stop at 0 so it does not underflow
}