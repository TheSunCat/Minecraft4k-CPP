#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

bool write_file(const std::string& input_file, std::string_view symbol)
{
    std::ifstream f;
    f.open(input_file);
    if(!f.good())
    {
        return false;
    }

    std::string line;

    std::cout << "// Generated from " << input_file << "\n"
        << "\n"
        << "constexpr const char* " << symbol << " = \"\"\n";

    while(std::getline(f, line))
    {
        std::cout << "    \"";
        for(char c: line)
        {
            
            switch(c)
            {
            case '"':
                std::cout << "\\\"";
                break;
            case '\t':
                std::cout << "\\t";
                break;
            case '\\':
                std::cout << "\\\\";
                break;
            default:
                std::cout << c;
                break;
            }
        }
        std::cout << "\\n\"\n";
    }

    std::cout << "    ;\n\n";

    return true;
}


int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "Syntax: " << argv[0] << " <inputfile> <symbolname>\n";
        return 2;
    }

    const auto argn = 1;
    const auto ok = write_file(argv[argn], argv[argn + 1]);
    if (!ok)
    {
        fprintf(stderr, "Error opening or reading file: '%s'\n", argv[argn]);
        return 1;
    }
    else
    {
        return 0;
    }
}

