#include <string>

using namespace std;

namespace tool
{
    string* parse(string &str, int &size);
    unsigned char fix_overflow(unsigned char c, unsigned char x);
    unsigned char fix_underflow(unsigned char c, unsigned char x);
}