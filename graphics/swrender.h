#ifndef __SWRENDER_H__
#define __SWRENDER_H__

inline
bool Cull(u32 pos_x, u32 pos_y, u32 w, u32 h) {
    bool not_result = pos_x >= 0 && pos_x < w && pos_y >= 0 && pos_y < h;
    return !not_result;
}
inline
u32 Pos2Idx(u32 pos_x, u32 pos_y, u32 width) {
    u32 result = pos_x + width * pos_y;
    return result;
}

void DrawRandomPatternRGBA(u8* buffer, u32 w, u32 h) {
    RandInit();
    u32 pix_idx, r, g, b;
    for (u32 i = 0; i < h; ++i) {
        for (u32 j = 0; j < w; ++j) {
            pix_idx = j + i*w;
            r = RandIntMax(255);
            g = RandIntMax(255);
            b = RandIntMax(255);
            buffer[4 * pix_idx + 0] = r;
            buffer[4 * pix_idx + 1] = g;
            buffer[4 * pix_idx + 2] = b;
            buffer[4 * pix_idx + 3] = 255;
        }
    }
}

void DrawLineRGBA(u8* buffer, u16 w, u16 h, u16 ax, u16 ay, u16 bx, u16 by) {

    // initially working from a to b
    // there are four cases:
    // 1: slope <= 1, ax < bx
    // 2: slope <= 1, ax > bx 
    // 3: slope > 1, ay < by
    // 4: slope > 1, ay > by 

    f32 slope_ab = (f32) (by - ay) / (bx - ax);

    if (abs(slope_ab) <= 1) {
        // draw by x
        f32 slope = slope_ab;

        // swap?
        if (ax > bx) {
            u16 swapx = ax;
            u16 swapy = ay;

            ax = bx;
            ay = by;
            bx = swapx;
            by = swapy;
        }

        u16 x, y;
        u32 pix_idx;
        for (u16 i = 0; i <= bx - ax; ++i) {
            x = ax + i;
            y = ay + floor(slope * i);

            pix_idx = x + y*w;
            buffer[4 * pix_idx + 0] = 255;
            buffer[4 * pix_idx + 1] = 255;
            buffer[4 * pix_idx + 2] = 255;
            buffer[4 * pix_idx + 3] = 255;
        }
    }
    else {
        // draw by y
        float slope_inv = 1 / slope_ab;

        // swap a & b ?
        if (ay > by) {
            u16 swapx = ax;
            u16 swapy = ay;

            ax = bx;
            ay = by;
            bx = swapx;
            by = swapy;
        }

        u16 x, y;
        u32 pix_idx;
        for (u16 i = 0; i <= by - ay; ++i) {
            y = ay + i;
            x = ax + floor(slope_inv * i);

            pix_idx = x + y*w;
            buffer[4 * pix_idx + 0] = 255;
            buffer[4 * pix_idx + 1] = 255;
            buffer[4 * pix_idx + 2] = 255;
            buffer[4 * pix_idx + 3] = 255;
        }
    }

    /*
    for (u32 i = 0; i < h; ++i) {
        for (u32 j = 0; j < w; ++j) {
            pix_idx = j + i*w;

            buffer[4 * pix_idx + 0] = 255;
            buffer[4 * pix_idx + 1] = 255;
            buffer[4 * pix_idx + 2] = 255;
            buffer[4 * pix_idx + 3] = 255;
        }
    }
    */
}


#endif
