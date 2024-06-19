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

    WF_DRAW_BACKGROUND_AND_BORDER = 1 << 0,
    WF_DRAW_TEXT = 1 << 1,

    WF_LAYOUT_H = 1 << 10,
    WF_LAYOUT_V = 1 << 11,
    WF_LAYOUT_CX = 1 << 12,
    WF_LAYOUT_CY = 1 << 13,
};
bool WidgetIsLayout(u32 features) {
    bool result =
        features & WF_LAYOUT_H ||
        features & WF_LAYOUT_V ||
        features & WF_LAYOUT_CX ||
        features & WF_LAYOUT_CY ||
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
    s32 x;
    s32 y;

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
    if (w_layout->first != NULL) {
        Widget *sib = w_layout->first;
        while (sib->next != NULL) {
            sib = sib->next;
        }
        sib->next = w;
    }
    else {
        w_layout->first = w;
    }
    w->parent = w_layout;
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


void WidgetWrap_Rec(Widget *w, s32 *w_sum, s32 *h_sum, s32 *w_max, s32 *h_max) {
    //  -   returns widget size if set
    //  -   otherwise; recurses child widgets to accumulate / max sizes

    *w_sum = 0;
    *h_sum = 0;
    *w_max = 0;
    *h_max = 0;

    Widget *ch = w->first;
    while (ch != NULL) { // iterate child widgets
        s32 w_sum_ch;
        s32 h_sum_ch;
        s32 w_max_ch;
        s32 h_max_ch;

        WidgetWrap_Rec(ch, &w_sum_ch, &h_sum_ch, &w_max_ch, &h_max_ch);

        *w_sum += ch->w;
        *h_sum += ch->h;
        *w_max = MaxS32(*w_max, ch->w);
        *h_max = MaxS32(*h_max, ch->h);

        ch = ch->next;
    }

    // assign sizes to current widget 
    if (w->w == 0 || w->h == 0) {
        if (w->features & WF_LAYOUT_H) {
            w->w = *w_sum;
            w->h = *h_max;
        }
        if (w->features & WF_LAYOUT_V) {
            w->w = *w_max;
            w->h = *h_sum;
        }
        if ((w->features & WF_LAYOUT_CX) || (w->features & WF_LAYOUT_CX)) {
            w->w = *w_max;
            w->h = *h_max;
        }
    }
    // or keep pre-set sizes
    else {
        *w_sum = w->w;
        *h_sum = w->h;
        *w_max = w->w;
        *h_max = w->h;
    }

    //printf("w: %d h: %d\n", w->w, w->h);

}
void WidgetWrap_Tree(Widget *root) {
    s32 w_sum_ch;
    s32 h_sum_ch;
    s32 w_max_ch;
    s32 h_max_ch;

    WidgetWrap_Rec(root, &w_sum_ch, &h_sum_ch, &w_max_ch, &h_max_ch);
}


void UI_FrameEnd(MArena *a_tmp) {
    if (ml == false) {
        w_active = NULL;
    }

    // collect widgets during layout pass
    List<Widget*> all_widgets = InitList<Widget*>(a_tmp, 0);
    Widget *w = &_w_root;

    // layout pass: sizing
    WidgetWrap_Tree(w);

    // layout pass: positioning
    while (w != NULL) {
        ArenaAlloc(a_tmp, sizeof(Widget*));
        all_widgets.Add(w);

        s32 pt_x = 0;
        s32 pt_y = 0;

        // since all sizes are known, each widget can lay out its children according to its settings
        Widget *ch = w->first;
        while (ch != NULL) { // iterate child widgets
            ch->x0 = w->x0;
            ch->y0 = w->y0;

            // iterate the layout (will not work with nested layouts)
            if (w->features & WF_LAYOUT_H) {
                ch->x0 = w->x0 + pt_x;
                pt_x += ch->w;
            }
            else if (w->features & WF_LAYOUT_CX) {
                ch->x0 = w->x0 + (w->w - ch->w) / 2;
            }

            if (w->features & WF_LAYOUT_V) {
                ch->y0 = w->y0 + pt_y;
                pt_y += ch->h;
            }
            else if (w->features & WF_LAYOUT_CY) {
                ch->y0 = w->y0 + (w->h - ch->h) / 2;
            }

            // set the collision rect for next frame code-interleaved mouse collision
            ch->SetCollisionRectUsingX0Y0WH();

            // iter
            ch = ch->next;
        }


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

        if (w->features & WF_DRAW_BACKGROUND_AND_BORDER) {
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
            w->x0 = 0;
            w->y0 = 0;
            w->x = 0;
            w->y = 0;
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
        w->features |= WF_DRAW_BACKGROUND_AND_BORDER;

        w->w = 100;
        w->h = 50;
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
    w->frame_touched = 0;
    w->features |= WF_DRAW_BACKGROUND_AND_BORDER;
    w->features |= WF_LAYOUT_CX;
    w->features |= WF_LAYOUT_V;
    w->w = width;
    w->h = height;
    w->sz_border = 20;
    w->col_bckgrnd = ColorGray(0.9f);
    w->col_border = ColorGray(0.7f);

    TreeBranch(w);
}


void UI_SpacePanelH(u32 width) {
    // no frame persistence

    Widget *w = p_widgets->Alloc();
    w->frame_touched = 0;
    w->w = width;

    TreeSibling(w);
}


void UI_SpacePanelV(u32 height) {
    // no frame persistence

    Widget *w = p_widgets->Alloc();
    w->frame_touched = 0;
    w->h = height;

    TreeSibling(w);
}


void UI_Label(const char *text) {
    // no frame persistence
    Widget *w = p_widgets->Alloc();
    w->frame_touched = 0;
    w->features |= WF_DRAW_TEXT;

    w->text = Str { (char*) text, _strlen( (char*) text) };
    w->sz_font = FS_48;
    w->col_bckgrnd = ColorGray(0.9f);
    w->col_border = ColorBlack();
    w->col_text = ColorBlack();

    FontSize fs = GetFontSize();
    SetFontAndSize(w->sz_font);
    w->w = TextLineWidth(g_text_plotter, w->text);;
    w->h = g_text_plotter->ln_measured;
    SetFontAndSize(fs);

    TreeSibling(w);
}


void UI_LayoutHoriz() {
    Widget *w = p_widgets->Alloc();
    w->frame_touched = 0;
    w->features |= WF_LAYOUT_H;

    TreeBranch(w);
}


void UI_LayoutVert() {
    Widget *w = p_widgets->Alloc();
    w->frame_touched = 0;
    w->features |= WF_LAYOUT_V;

    TreeBranch(w);
}


void UI_LayoutHorizC() {}
void UI_LayoutVertC() {}


void UI_Pop() {
    TreePop();
}


#endif
