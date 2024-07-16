#include <math.h>
#include <assert.h>

#include "../../baselayer.h"
#include "../gtypes.h"
#include "../atlas.h"
#include "../resource.h"


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


void RunProgram() {
    MContext *ctx = InitBaselayer();

    s32 x;
    s32 y;
    s32 channels_in_file;
    u8 *data_01 = stbi_load("aliens_01.png", &x, &y, &channels_in_file, 3);
    printf("%d %d %d\n", x, y, channels_in_file);

    u8 *data_02 = stbi_load("aliens_02.png", &x, &y, &channels_in_file, 3);
    printf("%d %d %d\n", x, y, channels_in_file);


    stbi_write_bmp("spritemap.bmp", x, y, 3, data_02);
}


void Test() {
    printf("Running tests ...\n");
}


int main (int argc, char **argv) {
    TimeProgram;

    if (CLAContainsArg("--help", argc, argv) || CLAContainsArg("-h", argc, argv)) {
        printf("--help:          display help (this text)\n");
    }
    else if (CLAContainsArg("--test", argc, argv)) {
        Test();
    }
    else {
        InitBaselayer();
        RunProgram();
    }
}
