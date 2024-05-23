#include <string>

using namespace std;

namespace img
{
    class PGMFile
    {
        string header;
        unsigned int length, pair_count;
        unsigned char* data;
        unsigned char* pixels;
        unsigned int* values;

        public:
            PGMFile();
            PGMFile(string filename, string output, int tolerance);
            ~PGMFile();
            void resolution(string &line, int &width, int &height);
            void run_length_encoding();
            void run_length_encoding(int tolerance);
            void compress_from(string filename, string output, int tolerance);
            void decompress_to(string input, string filename);
            void delete_all();
    };
}