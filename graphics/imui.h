#ifndef __IMUI_H__
#define __IMUI_H__


//
//  UI Panel quad layout
//


List<QuadHexaVertex> LayoutPanel(
    MArena *a_dest,
    s32 l, s32 t, s32 w, s32 h,
    s32 thic_border, Color col_border = { RGBA_GRAY_75 }, Color col_pnl = { RGBA_WHITE } )
{
    if (thic_border >= w / 2 || thic_border >= w / 2) {
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
        q.x0 = l + thic_border;
        q.x1 = l + w - thic_border;
        q.y0 = t + thic_border;
        q.y1 = t + h - thic_border;
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
//


//
//  Tree structure is built every turn
//  
//  How the tree structure links:
//      - siblings are iterated by next
//      - sub-branches are created from a node using first
//      - all nodes (except root) have parent set
//


struct CollRect {
    s32 x0;
    s32 x1;
    s32 y0;
    s32 y1;

    inline
    bool DidCollide(s32 x, s32 y) {
        bool bx = (x >= x0 && x <= x1);
        bool by = (y >= y0 && y <= y1);
        return bx && by;
    }
};


enum WidgetFlags {
    WF_PASSIVE = 0,

    WF_DRAW_BACKGROUND = 1 << 1,
    WF_DRAW_TEXT = 1 << 3,
    WF_DRAW_BORDER = 1 << 4,

    WF_LAYOUT_H = 1 << 10,
    WF_LAYOUT_V = 1 << 11,
    WF_LAYOUT_C = 1 << 12,
};
bool WidgetIsLayout(u32 features) {
    bool result =
        features & WF_LAYOUT_H ||
        features & WF_LAYOUT_V ||
        features & WF_LAYOUT_C ||
    false;
    return result;
}


struct Widget {
    Widget *next;       // sibling in the branch
    Widget *first;      // child sub-branch first
    Widget *parent;     // parent of the branch

    u64 hash_key;       // hash for frame-boundary persistence
    u64 frame_touched;  // expiration date

    s32 x0;
    s32 y0;
    s32 w;
    s32 h;
    s32 w_max;
    s32 h_max;

    Str text;
    FontSize sz_font;
    s32 sz_border;
    Color col_bckgrnd;
    Color col_text;
    Color col_border;

    u32 features;

    // everything below belongs in the layout algorithm
    CollRect rect;
    void CollRectClear() {
        rect = {};
    }
    void SetCollisionRectUsingX0Y0WH() {
        rect.x0 = x0;
        rect.x1 = x0 + w;
        rect.y0 = y0;
        rect.y1 = y0 + h;
    }
};


//
//  Core


static MArena _g_a_imui;
static MArena *g_a_imui;
static MPoolT<Widget> _p_widgets;
static MPoolT<Widget> *p_widgets;
static Stack<Widget*> _s_widgets;
static Stack<Widget*> *s_widgets;
static HashMap _map;
static HashMap *map;
static Widget _w_root;
static Widget *w_layout;
static Widget *w_hot;
static Widget *w_active;


s32 mdl;
bool ml;
bool mlclicked;
s32 mx;
s32 my;
u64 frameno;


void TreeSibling(Widget *w) {
    if (w_layout->first != NULL) {
        Widget *sib = w_layout->first;
        while (sib->next != NULL) {
            sib = sib->next;
        }
        sib->next = w;
        w->parent = sib->parent;
    }
    else {
        w_layout->first = w;
        w->parent = w_layout;
    }
}
void TreeBranch(Widget *w) {
    w_layout->first = w;
    w->parent = w;
    w_layout = w;
}
void TreePop() {
    Widget *parent = w_layout->parent;
    if (parent != NULL) {
        w_layout = parent;
    }
}


void UI_Init(u32 width = 1280, u32 height = 800) {
    if (g_a_imui != NULL) {
        printf("WARN: imui re-initialize\nd");

        // TODO: reset / clear
    }
    else {
        MArena _g_a_imui = ArenaCreate();
        g_a_imui = &_g_a_imui;

        u32 max_widgets = 1000;
        _p_widgets = PoolCreate<Widget>(max_widgets);
        p_widgets = &_p_widgets;

        _s_widgets = InitStack<Widget*>(g_a_imui, max_widgets);
        s_widgets = &_s_widgets;

        // TODO: It seems we do need to remove from the hash-map, thus impl. MapDelete()
        _map = InitMap(g_a_imui, max_widgets);
        map = &_map;

        _w_root = {};
        _w_root.features |= WF_LAYOUT_H;
        _w_root.w_max = width;
        _w_root.h_max = height,
        _w_root.x0 = 0;
        _w_root.y0 = 0;

        w_layout = &_w_root;
    }
}


void UI_FrameEnd(MArena *a_tmp) {
    if (ml == false) {
        w_active = NULL;
    }

    // collect widgets during layout pass
    List<Widget*> all_widgets = InitList<Widget*>(a_tmp, 0);

    // layout pass
    Widget *w = &_w_root;
    s32 x = w->x0;
    s32 y = w->y0;
    while (w != NULL) {
        ArenaAlloc(a_tmp, sizeof(Widget*));
        all_widgets.Add(w);


        if (w->features & WF_DRAW_TEXT) {
            // auto-size if not set
            if (w->h == 0 && w->w == 0) {
                SetFontAndSize(w->sz_font);
                w->w = TextLineWidth(g_text_plotter, w->text);;
                w->h = g_text_plotter->ln_measured;
            }
        }


        // set the collision rect for next frame code-interleaved mouse collision
        w->x0 = x;
        w->y0 = y;
        w->SetCollisionRectUsingX0Y0WH();

        // iterate the layout
        x += w->w;


        // iter
        if (w->first != NULL) {
            if (w->next) {
                s_widgets->Push(w->next);
            }
            w = w->first;
        }
        else if (w->next) {
            w = w->next;
        }
        else {
            w = s_widgets->Pop();
        }
    }

    // render pass
    for (u32 i = 0; i < all_widgets.len; ++i) {
        Widget *w = all_widgets.lst[i];

        if (w->features & WF_DRAW_BACKGROUND) {
            LayoutPanel(w->x0, w->y0, w->w, w->h, w->sz_border, w->col_border, w->col_bckgrnd);
        }

        if (w->features & WF_DRAW_TEXT) {
            SetFontAndSize(w->sz_font);
            s32 w_out;
            s32 h_out;
            List<QuadHexaVertex> txt_quads = LayoutTextLine(w->text, w->x0, w->y0, &w_out, &h_out, w->col_text);

            // position text at widget center
            s32 w_center_x = w->x0 + w->w / 2;
            s32 w_center_y = w->y0 + w->h / 2;

            s32 offset_x = (w->w - w_out) / 2;
            s32 offset_y = (w->h - h_out) / 2;
            for (u32 i = 0; i < txt_quads.len; ++i) {
                QuadOffset(txt_quads.lst + i, offset_x, offset_y);
            }
        }
    }


    // clean up pass
    _w_root.frame_touched = frameno;
    w_layout = &_w_root;
    for (u32 i = 0; i < all_widgets.len; ++i) {
        Widget *w = all_widgets.lst[i];

        // prune
        if (w->frame_touched < frameno) {
            MapRemove(map, w->hash_key, w);
            p_widgets->Free(w);
        }
        // clean
        else {
            w->parent = NULL;
            w->first = NULL;
            w->next = NULL;
        }
    }
}


//
//  Builder API


bool UI_Button(const char *text) {
    u64 key = HashStringValue(text);

    Widget *w = (Widget*) MapGet(map, key);
    if (w == NULL) {
        w = p_widgets->Alloc();
        w->features |= WF_DRAW_TEXT;
        w->features |= WF_DRAW_BACKGROUND;
        w->features |= WF_DRAW_BORDER;

        w->w = 100;
        w->h = 150;
        w->text = Str { (char*) text, _strlen( (char*) text) };
        w->sz_font = FS_36;

        MapPut(map, key, w);
    }
    w->frame_touched = frameno;

    bool hot = w->rect.DidCollide( mx, my ) && (w_active == NULL || w_active == w);
    if (hot) {
        if (ml) {
            w_active = w;
        }
    }
    bool active = (w_active == w);
    bool clicked = active && hot && (mdl == 1 || mlclicked);

    if (active) {
        // ACTIVE: mouse-down was engaged on this element

        // configure active properties
        w->sz_border = 3;
        w->col_bckgrnd = ColorGray(0.8f); // panel
        w->col_text = ColorBlack();        // text
        w->col_border = ColorBlack();     // border
    }
    else if (hot) {
        // HOT: currently hovering the mouse
        w_hot = w;

        // configure hot properties
        w->sz_border = 3;
        w->col_bckgrnd = ColorWhite(); // panel
        w->col_text = ColorBlack();        // text
        w->col_border = ColorBlack();     // border
    }
    else {
        // configure cold properties
        w->sz_border = 1;
        w->col_bckgrnd = ColorWhite(); // panel
        w->col_text = ColorBlack();        // text
        w->col_border = ColorBlack();     // border
    }

    // add into widget tree
    TreeSibling(w);

    return clicked;
}

void UI_CoolPanel(u32 width, u32 height) {
    // no frame persistence

    Widget *w = p_widgets->Alloc();
    w->features |= WF_DRAW_BACKGROUND;
    w->features |= WF_DRAW_BORDER;
    w->features |= WF_LAYOUT_C;
    w->w = width;
    w->h = height;
    w->sz_border = 20;
    w->col_bckgrnd = ColorGray(0.9f);
    w->col_border = ColorGray(0.7f);

    TreeSibling(w);
}


void UI_Label(const char *text) {
    // no frame persistence
    Widget *w = p_widgets->Alloc();
    w->features |= WF_DRAW_TEXT;

    w->w = 0;
    w->h = 0;
    w->text = Str { (char*) text, _strlen( (char*) text) };
    w->sz_font = FS_48;
    w->col_bckgrnd = ColorGray(0.9f);
    w->col_border = ColorBlack();
    w->col_text = ColorBlack();

    TreeSibling(w);
}
void UI_LayoutHoriz(u64 key) {}
void UI_LayoutVert(u64 key) {}
void UI_LayoutHorizC(u64 key) {}
void UI_LayoutVertC(u64 key) {}
void UI_Pop() {}


#endif
