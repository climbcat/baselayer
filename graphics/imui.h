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
    LK_HORIZONTAL,
    LK_VERTICAL,
    LK_CENTER,

    LK_CNT,
};


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
    WF_SOLID = 1 << 0,
    WF_CLICKABLE = 1 << 1,
    WF_DRAW_BACKGROUND = 1 << 2,
    WF_DRAW_TEXT = 1 << 3,
    WF_DRAW_BORDER = 1 << 4,
    WF_HAS_HOT = 1 << 5,
    WF_HAS_ACTIVE = 1 << 6,
};


struct Widget {
    Widget *next;   // sibling in the branch
    Widget *first;  // child sub-branch first
    Widget *parent; // parent of the branch

    u64 hash_key;   // hashes frame-boundary persistent widgets
    u64 frame_touched;

    // layout info
    s32 x0;
    s32 y0;
    s32 w;
    s32 h;

    CollRect rect;

    s32 marg;
    //s32 padding;

    Str text;
    s32 border;
    Color col_bckgrnd;
    Color col_alt;
    Color col_border;

    u32 features;

    // everything below probably belongs in the layout algorithm
    LayoutKind layout_kind;

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
static Widget *w_branch;
static Widget *w_hot;
static Widget *w_active;


s32 mdl;
bool ml;
bool mlclicked;
s32 mx;
s32 my;
u64 frameno;



void TreeSibling(Widget *w) {
    if (w_branch->first != NULL) {
        Widget *sib = w_branch->first;
        while (sib->next != NULL) {
            sib = sib->next;
        }
        sib->next = w;
        w->parent = sib->parent;
    }
    else {
        w_branch->first = w;
        w->parent = w_branch;
    }
}
void TreeBranch(Widget *w) {
    w_branch->first = w;
    w->parent = w;
    w_branch = w;
}
void TreePop() {
    Widget *parent = w_branch->parent;
    if (parent != NULL) {
        w_branch = parent;
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

        w_branch = &_w_root;
        _w_root.w = width;
        _w_root.h = height,
        _w_root.x0 = 0;
        _w_root.y0 = 0;
    }
}


void UI_FrameEnd(MArena *a_tmp) {
    // TODO: offline auto-layout
    if (frameno % 1 == 0) {
        //printf("%d %d %d %d fn: %lu\n", mx, my, mdown, mup, frameno);
    }
    if (ml == false) {
        w_active = NULL;
    }

    Widget *w = &_w_root;
    s32 x = w->x0;
    s32 y = w->y0;

    List<Widget*> all_widgets = InitList<Widget*>(a_tmp, 0);
    while (w != NULL) {
        // collect all widgets
        ArenaAlloc(a_tmp, sizeof(Widget*));
        all_widgets.Add(w);


        // layout (may iter all children, up to read parent properties, etc. etc.)

        w->x0 = x;
        w->y0 = y;
        w->SetCollisionRectUsingX0Y0WH();

        if (w->features & WF_CLICKABLE) {

            LayoutPanel(x + w->marg, y + w->marg, w->w, w->h, w->border, ColorBlack(), w->col_bckgrnd);

        }
        if (w->features & WF_DRAW_TEXT) {
            // TODO: test for draw_text
            List<QuadHexaVertex> quads = LayoutText(w->text.str, x + w->marg, w->y0 + w->marg, w->w, w->h, ColorBlack(), FS_24, TAL_CENTER);

            // vertical align center
            s32 offset_y = w->h / 2 + GetLineCenterVOffset();
            for (u32 i = 0; i < quads.len; ++i) {
                QuadOffset(quads.lst + i, 0, offset_y);
            }
        }


        // only for the horizontal layout
        if (w->features & WF_SOLID) {
            x += w->w;
        }


        // iter depth-first
        if (w->first != NULL) {
            // descend
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

    // clean all tree links for frame-persistent widgets & free untouched widgets
    _w_root.frame_touched = frameno;
    w_branch = &_w_root;
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
        w->features |= WF_SOLID;
        w->features |= WF_CLICKABLE;
        w->features |= WF_DRAW_TEXT;
        w->features |= WF_DRAW_BACKGROUND;
        w->features |= WF_DRAW_BORDER;

        u32 f = w->features;

        printf("%d ", f & WF_CLICKABLE);
        printf("%d ", f & WF_DRAW_TEXT);
        printf("%d ", f & WF_DRAW_BORDER);
        printf("%d ", f & WF_HAS_ACTIVE);
        printf("\n");

        w->w = 100;
        w->h = 50;
        w->text = Str { (char*) text, _strlen( (char*) text) };

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
        w->border = 3;
        w->col_bckgrnd = ColorGray(0.8f); // panel
        w->col_alt = ColorBlack();        // text
        w->col_border = ColorBlack();     // border
    }
    else if (hot) {
        // HOT: currently hovering the mouse
        w_hot = w;

        // configure hot properties
        w->border = 3;
        w->col_bckgrnd = ColorWhite(); // panel
        w->col_alt = ColorBlack();        // text
        w->col_border = ColorBlack();     // border
    }
    else {
        // configure cold properties
        w->border = 1;
        w->col_bckgrnd = ColorWhite(); // panel
        w->col_alt = ColorBlack();        // text
        w->col_border = ColorBlack();     // border
    }

    // add into widget tree
    TreeSibling(w);

    return clicked;
}


void UI_Label(const char *text) {}
void UI_Panel(u64 key) {}
void UI_LayoutHoriz(u64 key) {}
void UI_LayoutVert(u64 key) {}
void UI_LayoutHorizC(u64 key) {}
void UI_LayoutVertC(u64 key) {}
void UI_Pop() {}


#endif
