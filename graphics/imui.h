#ifndef __IMUI_H__
#define __IMUI_H__


//
//  UI Panel quad layout
//


List<QuadHexaVertex> LayoutPanel(
        MArena *a_dest,
        s32 l, s32 t, s32 w, s32 h,
        s32 border,
        Color col_border = { RGBA_GRAY_75 }, Color col_pnl = { RGBA_WHITE } )
{
    if (border >= w / 2 || border >= w / 2) {
        return List<QuadHexaVertex> { NULL, 0 };
    }

    DrawCall dc;
    dc.texture = 0;
    dc.quads = InitList<QuadHexaVertex>(a_dest, 2);
    {
        Quad q;
        _memzero(&q, sizeof(Quad));
        q.x0 = l;
        q.x1 = l + w;
        q.y0 = t;
        q.y1 = t + h;
        q.c = col_border;
        dc.quads.Add(QuadCook(q));
    }
    {
        Quad q;
        _memzero(&q, sizeof(Quad));
        q.x0 = l + border;
        q.x1 = l + w - border;
        q.y0 = t + border;
        q.y1 = t + h - border;
        q.c = col_pnl;
        dc.quads.Add(QuadCook(q));
    }

    List<QuadHexaVertex> quads = SR_Push(dc);
    return quads;
}
inline
List<QuadHexaVertex> LayoutPanel(
        s32 l, s32 t, s32 w, s32 h,
        s32 border,
        Color col_border = { RGBA_GRAY_75 }, Color col_pnl = { RGBA_WHITE } )
{
    return LayoutPanel(g_a_quadbuffer, l, t, w, h, border, col_border, col_pnl);
}


//
//  IMUI system experiments


enum LayoutKind {
    LK_INDEICISIVE,
    LK_VERTICAL,
    LK_HORIZONTAL,

    LK_CNT,
};


//  How the tree structure links:
//
//  - a new branch is created using first
//  - each node in a branch link the same first (first node in branch)
//  - each node in a branch link the same parent (parent node of branch)
//  - each node links next (next node in branch)
//
//  Tree structure is built every turn
//  


struct Widget {
    Widget *next;
    Widget *first;
    Widget *parent;

    u64 hash_key;

    // layout info

    s32 x0;
    s32 y0;
    s32 x;
    s32 y;
    s32 marg;

    // everything below probably belongs in the layout algorithm: 
    s32 w_max;
    s32 h_max;
    LayoutKind layout_kind;

    void IncItem(s32 w, s32 h) {
        if (layout_kind == LK_HORIZONTAL) {
            x += w + marg*2;
        }
        else if (layout_kind == LK_VERTICAL) {
            y += h + marg*2;
        }
        w_max = MaxS32(w_max, w);
        h_max = MaxS32(h_max, h);
    }
    void NewRowOrCol() {
        if (layout_kind == LK_HORIZONTAL) {
            x = x0;
            y += h_max;
        }
        else if (layout_kind == LK_VERTICAL) {
            x += w_max;
            y = y0;
        }
        h_max = 0;
        w_max = 0;
    }
};


//
//  Mock bnuilder code API


bool UI_Button(const char *text) {return true; }
void UI_Label(const char *text) {}
void UI_Panel() {}
void UI_LayoutHoriz() {}
void UI_LayoutVert() {}
void UI_LayoutCenterHoriz() {}
void UI_LayoutCenterVert() {}


#endif
