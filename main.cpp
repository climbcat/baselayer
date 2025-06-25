#include "baselayer.h"

#include "test.cpp"


void RunProgram() {
    printf("Usage:\n");
    printf("./baselayer\n");
    printf("./baselayer --help\n");
    printf("./baselayer --test\n");
}


Str LoadTextFile(MArena *a_files, const char *f_path) {
    Str f_license = {};
    f_license.str = (char*) LoadFileFSeek(a_files, f_path, &f_license.len);

    return f_license;
}


int main (int argc, char **argv) {
    TimeProgram;
    bool force_tests = false;

    if (CLAContainsArg("--help", argc, argv)) {
        printf("Usage: ./baselayer <args>\n");
        printf("--help:         Display help (this text)\n");
        printf("--release:      Combine source files into jg_baselayer.h\n");
        printf("--version:      Print baselayer version\n");
        printf("--test:         Run test functions\n");
        exit(0);
    }
    else if (CLAContainsArg("--test", argc, argv) || force_tests) {
        Test();
    }
    else if (CLAContainsArg("--version", argc, argv) || force_tests) {
        BaselayerPrintVersion();
        exit(0);
    }
    else if (CLAContainsArg("--release", argc, argv) || force_tests) {

        MArena *a_files = InitBaselayer()->a_life;

        Str f_license = LoadTextFile(a_files, "../LICENSE");
        Str f_base = LoadTextFile(a_files, "../base.h");
        Str f_profile = LoadTextFile(a_files, "../profile.h");
        Str f_memory = LoadTextFile(a_files, "../memory.h");
        Str f_string = LoadTextFile(a_files, "../string.h");
        Str f_hash = LoadTextFile(a_files, "../hash.h");
        Str f_utils = LoadTextFile(a_files, "../utils.h");
        Str f_platform = LoadTextFile(a_files, "../platform.h");

        StrBuff buff = StrBuffInit();
        StrBuffPrint1K(&buff, "/*\n", 0);
        StrBuffAppend(&buff, f_license);
        StrBuffPrint1K(&buff, "*/\n", 0);
        StrBuffPrint1K(&buff, "\n", 0);
        StrBuffPrint1K(&buff, "\n", 0);
        StrBuffPrint1K(&buff, "#ifndef __JG_BASELAYER_H__\n", 0);
        StrBuffPrint1K(&buff, "#define __JG_BASELAYER_H__\n", 0);
        StrBuffPrint1K(&buff, "\n", 0);
        StrBuffPrint1K(&buff, "\n", 0);
        StrBuffPrint1K(&buff, "#define BASELAYER_VERSION_MAJOR %d\n", 1, BASELAYER_VERSION_MAJOR);
        StrBuffPrint1K(&buff, "#define BASELAYER_VERSION_MINOR %d\n", 1, BASELAYER_VERSION_MINOR);
        StrBuffPrint1K(&buff, "#define BASELAYER_VERSION_PATCH %d\n", 1, BASELAYER_VERSION_PATCH);
        StrBuffPrint1K(&buff, "\n", 0);
        StrBuffPrint1K(&buff, "\n", 0);

        StrBuffAppend(&buff, f_base);

        StrBuffPrint1K(&buff, "\n", 0);
        StrBuffPrint1K(&buff, "\n", 0);

        StrBuffAppend(&buff, f_profile);

        StrBuffPrint1K(&buff, "\n", 0);
        StrBuffPrint1K(&buff, "\n", 0);

        StrBuffAppend(&buff, f_memory);

        StrBuffPrint1K(&buff, "\n", 0);
        StrBuffPrint1K(&buff, "\n", 0);

        StrBuffAppend(&buff, f_string);

        StrBuffPrint1K(&buff, "\n", 0);
        StrBuffPrint1K(&buff, "\n", 0);

        StrBuffAppend(&buff, f_hash);

        StrBuffPrint1K(&buff, "\n", 0);
        StrBuffPrint1K(&buff, "\n", 0);

        StrBuffAppend(&buff, f_utils);

        StrBuffPrint1K(&buff, "\n", 0);
        StrBuffPrint1K(&buff, "\n", 0);

        StrBuffAppend(&buff, f_platform);

        StrBuffPrint1K(&buff, "\n", 0);
        StrBuffPrint1K(&buff, "\n", 0);
        StrBuffPrint1K(&buff, "#endif\n", 0);
        SaveFile("jg_baselayer.h_OUT", buff.str, buff.len);
    }
    else {
        RunProgram();
    }
}
