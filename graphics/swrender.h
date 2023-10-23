#ifndef __SWRENDER_H__
#define __SWRENDER_H__


struct Vector2_u16 {
    u16 x;
    u16 y;
};


//
//


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
void ClearToZeroRGBA(u8* image_buffer, u32 w, u32 h) {
    memset(image_buffer, 0, 4 * w * h);
}
inline
Vector2_u16 NDC2Screen(u32 w, u32 h, Vector3f ndc) {
    Vector2_u16 pos;

    pos.x = (u32) ((ndc.x + 1) / 2 * w);
    pos.y = (u32) ((ndc.y + 1) / 2 * h);

    return pos;
}
u16 LinesToScreen(u32 w, u32 h, Vector2_u16* lines_idxbuffer, u16 nlines, Vector3f *ndc_buffer, Vector2_u16* lines_screen_buffer) {
    u16 nlines_torender = 0;
    u16 idx = 0;
    for (u32 i = 0; i < nlines; ++i) {
        Vector2_u16 line = lines_idxbuffer[i];
        Vector2_u16 a = NDC2Screen(w, h, ndc_buffer[line.x]);
        Vector2_u16 b = NDC2Screen(w, h, ndc_buffer[line.y]);

        // TODO: crop to NDC box
        if (Cull(a.x, a.y, w, h) || Cull(b.x, b.y, w, h)) {
            continue;
        }

        lines_screen_buffer[idx++] = a;
        lines_screen_buffer[idx++] = b;
        nlines_torender++;
    }
    return nlines_torender;
}


//
// Axis-aligned box


struct AABox {
    Vector3f center;
    f32 radius;
};
void AABoxGetCorners(AABox box, Vector3f *dest) {
    AABox *b = &box;
    *dest++ = Vector3f { b->center.x - b->radius, b->center.y - b->radius, b->center.z - b->radius };
    *dest++ = Vector3f { b->center.x - b->radius, b->center.y - b->radius, b->center.z + b->radius };
    *dest++ = Vector3f { b->center.x - b->radius, b->center.y + b->radius, b->center.z - b->radius };
    *dest++ = Vector3f { b->center.x - b->radius, b->center.y + b->radius, b->center.z + b->radius };
    *dest++ = Vector3f { b->center.x + b->radius, b->center.y - b->radius, b->center.z - b->radius };
    *dest++ = Vector3f { b->center.x + b->radius, b->center.y - b->radius, b->center.z + b->radius };
    *dest++ = Vector3f { b->center.x + b->radius, b->center.y + b->radius, b->center.z - b->radius };
    *dest++ = Vector3f { b->center.x + b->radius, b->center.y + b->radius, b->center.z + b->radius };
}
u16 AABoxGetLinesIndices(u16 offset, Vector2_u16 *dest) {
    *dest++ = Vector2_u16 { (u16) (offset + 0), (u16) (offset + 1) };
    *dest++ = Vector2_u16 { (u16) (offset + 0), (u16) (offset + 2) };
    *dest++ = Vector2_u16 { (u16) (offset + 0), (u16) (offset + 4) };

    *dest++ = Vector2_u16 { (u16) (offset + 3), (u16) (offset + 1) };
    *dest++ = Vector2_u16 { (u16) (offset + 3), (u16) (offset + 2) };
    *dest++ = Vector2_u16 { (u16) (offset + 3), (u16) (offset + 7) };

    *dest++ = Vector2_u16 { (u16) (offset + 5), (u16) (offset + 1) };
    *dest++ = Vector2_u16 { (u16) (offset + 5), (u16) (offset + 4) };
    *dest++ = Vector2_u16 { (u16) (offset + 5), (u16) (offset + 7) };

    *dest++ = Vector2_u16 { (u16) (offset + 6), (u16) (offset + 2) };
    *dest++ = Vector2_u16 { (u16) (offset + 6), (u16) (offset + 4) };
    *dest++ = Vector2_u16 { (u16) (offset + 6), (u16) (offset + 7) };

    return offset + 12;
}


//
// Render to image buffer


void DrawRandomPatternRGBA(u8* image_buffer, u32 w, u32 h) {
    RandInit();
    u32 pix_idx, r, g, b;
    for (u32 i = 0; i < h; ++i) {
        for (u32 j = 0; j < w; ++j) {
            pix_idx = j + i*w;
            r = RandIntMax(255);
            g = RandIntMax(255);
            b = RandIntMax(255);
            image_buffer[4 * pix_idx + 0] = r;
            image_buffer[4 * pix_idx + 1] = g;
            image_buffer[4 * pix_idx + 2] = b;
            image_buffer[4 * pix_idx + 3] = 255;
        }
    }
}

void DrawLineRGBA(u8* image_buffer, u16 w, u16 h, u16 ax, u16 ay, u16 bx, u16 by) {

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
            image_buffer[4 * pix_idx + 0] = 255;
            image_buffer[4 * pix_idx + 1] = 255;
            image_buffer[4 * pix_idx + 2] = 255;
            image_buffer[4 * pix_idx + 3] = 255;
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
            image_buffer[4 * pix_idx + 0] = 255;
            image_buffer[4 * pix_idx + 1] = 255;
            image_buffer[4 * pix_idx + 2] = 255;
            image_buffer[4 * pix_idx + 3] = 255;
        }
    }
}

inline
void DrawLineRGBA(u8* image_buffer, u16 w, u16 h, Vector2_u16 a, Vector2_u16 b) {
    DrawLineRGBA(image_buffer, w, h, a.x, a.y, b.x, b.y);
}


#endif
