#include "baselayer.h"

#include "test.cpp"


void RunProgram() {
    printf("Use:\n");
    printf("./baselayer --help\n");
}


Str LoadTextFile(MArena *a_files, const char *f_path) {
    Str f_str = {};
    f_str.str = (char*) LoadFileFSeek(a_files, f_path, &f_str.len);

    return f_str;
}

Str LoadTextFile(MArena *a_files, Str f_path) {
    Str f_str = {};
    f_str.str = (char*) LoadFileFSeek(a_files, StrZeroTerm(f_path), &f_str.len);

    return f_str;
}


int main (int argc, char **argv) {
    TimeProgram;
    bool force_tests = false;

    if (CLAContainsArg("--help", argc, argv)) {
        printf("Usage: ./baselayer <args>\n");
        printf("--help:         Display help (this text)\n");
        printf("--version:      Print baselayer version\n");
        printf("--release:      Combine source files into jg_baselayer.h\n");
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

        StrLst *f_sources = NULL;
        f_sources = StrLstPush("../base.h", f_sources);
        f_sources = StrLstPush("../profile.h", f_sources);
        f_sources = StrLstPush("../memory.h", f_sources);
        f_sources = StrLstPush("../string.h", f_sources);
        f_sources = StrLstPush("../hash.h", f_sources);
        f_sources = StrLstPush("../utils.h", f_sources);
        f_sources = StrLstPush("../platform.h", f_sources);
        f_sources = f_sources->first;

        StrBuff buff = StrBuffInit();
        StrBuffPrint1K(&buff, "/*\n", 0);
        StrBuffAppend(&buff, LoadTextFile(a_files, "../LICENSE"));
        StrBuffPrint1K(&buff, "*/\n\n\n", 0);
        StrBuffPrint1K(&buff, "#ifndef __JG_BASELAYER_H__\n", 0);
        StrBuffPrint1K(&buff, "#define __JG_BASELAYER_H__\n\n\n", 0);
        StrBuffPrint1K(&buff, "#define BASELAYER_VERSION_MAJOR %d\n", 1, BASELAYER_VERSION_MAJOR);
        StrBuffPrint1K(&buff, "#define BASELAYER_VERSION_MINOR %d\n", 1, BASELAYER_VERSION_MINOR);
        StrBuffPrint1K(&buff, "#define BASELAYER_VERSION_PATCH %d\n", 1, BASELAYER_VERSION_PATCH);
        StrBuffPrint1K(&buff, "\n\n", 0);

        while (f_sources) {
            StrBuffAppend(&buff, LoadTextFile(a_files, f_sources->GetStr()));
            StrBuffPrint1K(&buff, "\n\n", 0);

            f_sources = f_sources->next;
        }

        StrBuffPrint1K(&buff, "#endif // __JG_BASELAYER_H__\n", 0);
        SaveFile("jg_baselayer.h_OUT", buff.str, buff.len);
    }
    else {
        RunProgram();
    }
}
