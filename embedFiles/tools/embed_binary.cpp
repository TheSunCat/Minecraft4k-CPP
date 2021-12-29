// imgui/misc/fonts
#define main main_not_used
#include "binary_to_compressed_c.cpp"
#undef main

bool custom_binary_to_compressed_c(const char* filename, const char* symbol, bool use_base85_encoding, bool use_compression)
{
    FILE* out = stdout;

    fprintf(out, "#pragma once\n");
    fprintf(out, "\n");
    fprintf(out, "#include \"embed/types.h\"\n");
    fprintf(out, "\n");

    const auto result = binary_to_compressed_c(filename, symbol, use_base85_encoding, use_compression);
    if(!result) {return result;}

    if(use_base85_encoding == false)
    {
        const char* compressed_type = use_compression ? "compressed_binary" : "embedded_binary";
        const char* compressed_str = use_compression ? "compressed_" : "";
        fprintf(out, "constexpr %s %s = %s{%s_%sdata, %s_%ssize};\n",
            compressed_type, symbol,
            compressed_type,
            symbol, compressed_str,
            symbol, compressed_str
        );
    }
    else
    {
        // todo(Gustav): support base85 encodings
        fprintf(out, "// base85 encoding currently not supported\n");
    }
    fprintf(out, "\n");

    return result;
}


// copied from binary_to_compressed_c.cpp but changed call from binary_to_compressed_c to custom_binary_to_compressed_c

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        printf("Syntax: %s [-base85] [-nocompress] <inputfile> <symbolname>\n", argv[0]);
        return 0;
    }

    int argn = 1;
    bool use_base85_encoding = false;
    bool use_compression = true;
    if (argv[argn][0] == '-')
    {
        if (strcmp(argv[argn], "-base85") == 0) { use_base85_encoding = true; argn++; }
        else if (strcmp(argv[argn], "-nocompress") == 0) { use_compression = false; argn++; }
        else
        {
            fprintf(stderr, "Unknown argument: '%s'\n", argv[argn]);
            return 1;
        }
    }

    bool ret = custom_binary_to_compressed_c(argv[argn], argv[argn + 1], use_base85_encoding, use_compression);
    if (!ret)
        fprintf(stderr, "Error opening or reading file: '%s'\n", argv[argn]);
    return ret ? 0 : 1;
}

