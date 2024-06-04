#ifndef __GTYPES_H__
#define __GTYPES_H__


#include <math.h>


//
// Colors


#define RGBA_BLACK      0, 0, 0, 255
#define RGBA_WHITE      255, 255, 255, 255
#define RGBA_GRAY_75    192, 192, 192, 192
#define RGBA_GRAY_50    128, 128, 128, 255
#define RGBA_GRAY_25    64, 64, 64, 255
#define RGBA_RED        255, 0, 0, 255
#define RGBA_GREEN      0, 255, 0, 255
#define RGBA_BLUE       0, 0, 255, 255
#define RGBA_YELLOW     0, 255, 255, 255


#define BYTES_RGB 3
#define BYTES_RGBA 4


struct Color {
    u8 r;
    u8 g;
    u8 b;
    u8 a;
};
void PrintColorInline(Color c) {
    printf("%hhx %hhx %hhx %hhx", c.r, c.g, c.b, c.a);
}
Color ColorRandom() {
    Color c;
    c.r = RandMinMaxU(100, 255);
    c.g = RandMinMaxU(100, 255);
    c.b = RandMinMaxU(100, 255);
    c.a = 255;
    return c;
}
Color ColorWhite() {
    return Color { RGBA_WHITE };
}
Color ColorBlack() {
    return Color { RGBA_BLACK };
}
Color ColorGray(f32 grayness) {
    u8 g = floor(grayness * 255);
    return Color { g, g, g, 255 };
}


//
// Rectangles


struct Rect {
    u16 width;
    u16 height;
    s16 left;
    s16 top;

    void Print() {
        printf("rect: w: %u, h: %u, left: %d, top: %d\n", width, height, left, top);
    }
};


Rect InitRectangle(u16 width, u16 height, u16 left = 0, u16 top = 0) {
    Rect r;
    r.width = width;
    r.height = height;
    r.left = left;
    r.top = top;
    return r;
}
Rect RectangleCrop(Rect us, Rect other) {
    Rect rect = InitRectangle(other.width, other.height, other.left, other.top);
    bool occluded = false;
    bool partially_occluded = false;

    // cases where other is completely outside of us
    if (other.left > us.left + us.width) { // to the right of us
        rect.left = us.left + us.width;
        rect.width = 0;
        occluded = true;
    }
    if (other.left + other.width < us.left) { // to the left of us
        rect.left = us.left;
        rect.width = 0;
        occluded = true;
    }
    if (other.top > us.top + us.height) { // above us
        rect.top = us.top + us.height;
        rect.height = 0;
        occluded = true;
    }
    if (other.top + other.height < us.top) { // below us
        rect.top = us.top;
        rect.height = 0;
        occluded = true;
    }
    if (occluded) {
        return rect;
    }

    // at least partially visible
    if (other.left < us.left) {
        rect.left = us.left;
        s16 diff = us.left - other.left;
        rect.width = rect.width - diff;
    }
    if (other.top < us.top) {
        rect.top = us.top;
        s16 diff = us.top - other.top;
        rect.height = rect.height - diff;
    }
    if (other.left + other.width > us.left + us.width) {
        rect.width = us.top + us.width;
    }
    if (other.top + other.height > us.top + us.height) {
        rect.height = us.top + us.height;
    }
    return rect;
}


//
//  Image type structs


struct ImageB {
    u32 width;
    u32 height;
    u8 *img;
};

struct ImageRGBX {
    u32 width;
    u32 height;
    u32 pixel_size;
    u8 *img;
};
ImageRGBX InitImageRGBX(void *data, u32 width, u32 height, u32 pixel_size) {
    ImageRGBX img;
    img.width = width;
    img.height = height;
    img.pixel_size = pixel_size;
    img.img = (u8*) data;
    return img;
}

struct ImageRGBA {
    u32 width;
    u32 height;
    Color *img;
};

struct ImageF32 {
    u32 width;
    u32 height;
    f32 *data;
};

struct ImageU32 {
    u32 width;
    u32 height;
    f32 *data;
};


#endif
