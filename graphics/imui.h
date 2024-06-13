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


struct Widget {
    s32 x0;
    s32 y0;
    s32 x;
    s32 y;
    s32 w_max;
    s32 h_max;
    s32 marg;
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


Widget UI_HorizontalLayout(s32 left, s32 top) {
    Widget layout;
    _memzero(&layout, sizeof(Widget));
    layout.layout_kind = LK_HORIZONTAL;
    layout.marg = 0;
    layout.x0 = left;
    layout.y0 = top;
    layout.x = left;
    layout.y = top;
    return layout;
}


Widget UI_VerticalLayout(s32 left, s32 top) {
    Widget layout = UI_HorizontalLayout(left, top);
    layout.layout_kind = LK_VERTICAL;
    layout.marg = 0;
    return layout;
}


Widget UI_VerticalLayoutBelow(Widget layout) {
    Widget vert = UI_VerticalLayout(layout.x, layout.y);
    vert.IncItem(layout.w_max, layout.h_max);
    return vert;
}


inline
void UI_ButtonProperties(s32 *width, s32 *height, s32 *border, Color *col_btn, Color *col_hot) {
    *width = 100;
    *height = 50;
    *border = 4;
    *col_btn = ColorGray(1.0f);
    *col_hot = ColorGray(0.9f);
}


bool UI_Button(Widget *layout, const char *lbl) {
    s32 btn_w;
    s32 btn_h;
    s32 btn_brd;
    Color col_btn;
    Color col_hot;
    UI_ButtonProperties(&btn_w, &btn_h, &btn_brd, &col_btn, &col_hot);

    Color col = col_btn;
    bool clicked = false;
    if (g_mouse->LimsLTWHLastFrame(layout->x, layout->y, btn_w, btn_h)) {
        col = col_hot;
        if (g_mouse->l) {
            btn_brd = 6;
        }
        clicked = g_mouse->ClickedRecently();

        if (clicked == true) {
            printf("click!\n");
        }
    }

    LayoutPanel(layout->x + layout->marg, layout->y + layout->marg, btn_w, btn_h, btn_brd, ColorBlack(), col);
    List<QuadHexaVertex> quads = LayoutText(lbl, layout->x + layout->marg, layout->y + layout->marg, btn_w, btn_h, ColorBlack(), FS_24, TAL_CENTER);

    // vertical align center
    s32 offset_y = btn_h / 2 + GetLineCenterVOffset();
    for (u32 i = 0; i < quads.len; ++i) {
        QuadOffset(quads.lst + i, 0, offset_y);
    }

    layout->IncItem(btn_w, btn_h);
    return clicked;
}


#endif
