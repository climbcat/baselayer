#ifndef __IMUI_H__
#define __IMUI_H__


#include "ui.h"


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

    DrawCall dc = {};
    dc.tpe = DCT_SOLID;
    dc.texture_key = 0;
    dc.quads = InitList<QuadHexaVertex>(a_dest, 2);
    dc.quads.Add(QuadCookSolid(w, h, l, t, col_border));
    dc.quads.Add(QuadCookSolid(w - 2*thic_border, h - 2*thic_border, l + thic_border, t + thic_border, col_pnl));

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

    WF_LAYOUT_HORIZONTAL = 1 << 10,
    WF_LAYOUT_VERTICAL = 1 << 11,
    WF_LAYOUT_HORIZONTAL_CENTERING = 1 << 12,
    WF_LAYOUT_VERTICAL_CENTERING = 1 << 13,

    WF_EXPAND_HORIZONTAL = 1 << 15,
    WF_EXPAND_VERTICAL = 1 << 16,

    WF_ABSOLUTE_POSITION = 1 << 20
};
bool WidgetIsLayout(u32 features) {
    bool result =
        features & WF_LAYOUT_HORIZONTAL ||
        features & WF_LAYOUT_VERTICAL ||
        features & WF_LAYOUT_HORIZONTAL_CENTERING ||
        features & WF_LAYOUT_VERTICAL_CENTERING ||
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
    void SetFeature(WidgetFlags f) {
        features = features |= f;
    }
};


//
//  Core


static MArena _g_a_imui;
static MArena *g_a_imui;
static MPoolT<Widget> _g_p_widgets;
static MPoolT<Widget> *g_p_widgets;
static Stack<Widget*> _g_s_widgets;
static Stack<Widget*> *g_s_widgets;

static HashMap _g_m_widgets;
static HashMap *g_m_widgets;

static Widget _g_w_root;
static Widget *g_w_layout;
static Widget *g_w_hot;
static Widget *g_w_active;

static u64 *g_frameno_imui;
static MouseTrap *g_mouse_imui;


void TreeSibling(Widget *w) {
    if (g_w_layout->first != NULL) {
        Widget *sib = g_w_layout->first;
        while (sib->next != NULL) {
            sib = sib->next;
        }
        sib->next = w;
        w->parent = sib->parent;
    }
    else {
        g_w_layout->first = w;
        w->parent = g_w_layout;
    }
}
void TreeBranch(Widget *w) {
    if (g_w_layout->first != NULL) {
        Widget *sib = g_w_layout->first;
        while (sib->next != NULL) {
            sib = sib->next;
        }
        sib->next = w;
    }
    else {
        g_w_layout->first = w;
    }
    w->parent = g_w_layout;
    g_w_layout = w;
}
void TreePop() {
    Widget *parent = g_w_layout->parent;
    if (parent != NULL) {
        g_w_layout = parent;
    }
}


void InitImUi(u32 width, u32 height, MouseTrap *mouse, u64 *frameno) {
    if (g_a_imui != NULL) {
        printf("WARN: imui re-initialize\nd");

        // TODO: reset / clear
    }
    else {
        assert(mouse != NULL);
        assert(frameno != NULL);
        g_mouse_imui = mouse;
        g_frameno_imui = frameno;

        MArena _g_a_imui = ArenaCreate();
        g_a_imui = &_g_a_imui;

        u32 max_widgets = 1000;
        _g_p_widgets = PoolCreate<Widget>(max_widgets);
        g_p_widgets = &_g_p_widgets;

        _g_s_widgets = InitStack<Widget*>(g_a_imui, max_widgets);
        g_s_widgets = &_g_s_widgets;

        // TODO: It seems we do need to remove from the hash-map, thus impl. MapDelete()
        _g_m_widgets = InitMap(g_a_imui, max_widgets);
        g_m_widgets = &_g_m_widgets;

        _g_w_root = {};
        _g_w_root.features |= WF_LAYOUT_HORIZONTAL;
        _g_w_root.w_max = width;
        _g_w_root.h_max = height,
        _g_w_root.x0 = 0;
        _g_w_root.y0 = 0;

        g_w_layout = &_g_w_root;
    }
}


void WidgetTreeSizeWrap_Rec(Widget *w, s32 *w_sum, s32 *h_sum, s32 *w_max, s32 *h_max) {
    // Recursively determines widget sizes by wrapping in child widgets. 
    // Sizes will be the minimal, and expander sizes will be expanded elsewhere.

    // There is an accumulated child size and a max child size.
    // Depending on the layou of the current widget, its actual size
    // is set to either the maximum child widget.
    // Or, if a panel has the WF_LAYOUT_H or WR_LAYOUT_V features, the sum of
    // each child's actual size.
    //
    // max & sum sizes are determined on descent, actual sizes are set on ascent.


    //
    // Descent: determine child_max and child_sum sizes


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

        WidgetTreeSizeWrap_Rec(ch, &w_sum_ch, &h_sum_ch, &w_max_ch, &h_max_ch);

        *w_sum += ch->w;
        *h_sum += ch->h;
        *w_max = MaxS32(*w_max, ch->w);
        *h_max = MaxS32(*h_max, ch->h);

        ch = ch->next;
    }


    //
    // Ascent: Assign actual size to current widget (where undefined)


    if (w->w == 0 && w->h == 0) {
        if (w->features & WF_LAYOUT_HORIZONTAL) {
            w->w = *w_sum;
            w->h = *h_max;
        }
        if (w->features & WF_LAYOUT_VERTICAL) {
            w->w = *w_max;
            w->h = *h_sum;
        }
        if ((w->features & WF_LAYOUT_HORIZONTAL_CENTERING) || (w->features & WF_LAYOUT_VERTICAL_CENTERING)) {
            w->w = *w_max;
            w->h = *h_max;
        }
    }
    // or keep pre-sets
    else {
        *w_sum = w->w;
        *h_sum = w->h;
        *w_max = w->w;
        *h_max = w->h;
    }
}


void WidgetTreeExpand_Rec(Widget *w) {
    // expands one sub-widget using own dimensions

    Widget *ch = w->first;
    if (ch == NULL) {
        return;
    }

    // extract info
    bool expander_vert_found = false;
    bool expander_horiz_found = false;
    u32 width_other = 0;
    u32 height_other = 0;
    while (ch) {
        if ( ! (ch->features & WF_EXPAND_VERTICAL)) {
            height_other += ch->h;
        }
        else {
            assert(w->features & WF_LAYOUT_HORIZONTAL || expander_vert_found == false && "Expander not allowed (WF_EXPAND_VERTICAL)");
            expander_vert_found = true;
        }

        if ( ! (ch->features & WF_EXPAND_HORIZONTAL)) {
            width_other += ch->w;
        }
        else {
            assert(w->features & WF_LAYOUT_VERTICAL || expander_horiz_found == false && "Expander not allowed (WF_EXPAND_HORIZONTAL)");
            expander_horiz_found = true;
        }

        ch = ch->next;
    }

    // assign max possible size to the expander(s)
    ch = w->first;
    while (ch) {
        if (ch->features & WF_EXPAND_VERTICAL) {
            ch->h = w->h - height_other;
        }
        if (ch->features & WF_EXPAND_HORIZONTAL) {
            ch->w = w->w - width_other;
        }
        ch = ch->next;
    }

    // descend
    ch = w->first;
    while (ch) {
        WidgetTreeExpand_Rec(ch);

        ch = ch->next;
    }
}


List<Widget*> WidgetTreePositioning(MArena *a_tmp, Widget *w_root) {
    List<Widget*> all_widgets = InitList<Widget*>(a_tmp, 0);
    Widget *w = w_root;


    while (w != NULL) {
        ArenaAlloc(a_tmp, sizeof(Widget*));
        all_widgets.Add(w);

        s32 pt_x = 0;
        s32 pt_y = 0;

        // with all widget sizes known, widgets can position their children
        Widget *ch = w->first;
        while (ch != NULL) { // iterate child widgets

            // set child position - skip completely if absolutely positioned
            if ((ch->features & WF_ABSOLUTE_POSITION) == false) {
                ch->x0 = w->x0;
                ch->y0 = w->y0;

                if (w->features & WF_LAYOUT_HORIZONTAL) {
                    ch->x0 = w->x0 + pt_x;
                    pt_x += ch->w;
                }
                else if (w->features & WF_LAYOUT_HORIZONTAL_CENTERING) {
                    ch->x0 = w->x0 + (w->w - ch->w) / 2;
                }

                if (w->features & WF_LAYOUT_VERTICAL) {
                    ch->y0 = w->y0 + pt_y;
                    pt_y += ch->h;
                }
                else if (w->features & WF_LAYOUT_VERTICAL_CENTERING) {
                    ch->y0 = w->y0 + (w->h - ch->h) / 2;
                }
            }

            // set the collision rect for next frame code-interleaved mouse collision
            ch->SetCollisionRectUsingX0Y0WH();

            // iter
            ch = ch->next;
        }

        // iter
        if (w->first != NULL) {
            if (w->next) {
                g_s_widgets->Push(w->next);
            }
            w = w->first;
        }
        else if (w->next) {
            w = w->next;
        }
        else {
            w = g_s_widgets->Pop();
        }
    }

    return all_widgets;
}


void WidgetTreeRenderToDrawcalls(List<Widget*> all_widgets) {


    // TODO: This should iterate the tree, and do something meaningful to figure out
    //      what should be rendered on top of what - proper interleaving of calls.


    for (u32 i = 0; i < all_widgets.len; ++i) {
        Widget *w = all_widgets.lst[i];

        if (w->features & WF_DRAW_BACKGROUND_AND_BORDER) {
            LayoutPanel(w->x0, w->y0, w->w, w->h, w->sz_border, w->col_border, w->col_bckgrnd);
        }

        if (w->features & WF_DRAW_TEXT) {
            SetFontSize(w->sz_font);
            s32 w_out;
            s32 h_out;
            List<QuadHexaVertex> txt_quads = LayoutTextLine(w->text, w->x0, w->y0, &w_out, &h_out, w->col_text);

            // position text at widget center
            s32 w_center_x = w->x0 + w->w / 2;
            s32 w_center_y = w->y0 + w->h / 2;

            s32 offset_x = (w->w - w_out) / 2;
            s32 offset_y = (w->h - h_out) / 2;
            for (u32 i = 0; i < txt_quads.len; ++i) {
                QuadOffset(txt_quads.lst + i, (f32) offset_x, (f32) offset_y);
            }
        }
    }
}


void UI_FrameEnd(MArena *a_tmp) {
    if (g_mouse_imui->l == false) {
        g_w_active = NULL;
    }

    Widget *w = &_g_w_root;
    w->w = w->w_max;
    w->h = w->h_max;


    // size widgets to wrap tightly
    s32 w_sum_ch;
    s32 h_sum_ch;
    s32 w_max_ch;
    s32 h_max_ch;
    WidgetTreeSizeWrap_Rec(w, &w_sum_ch, &h_sum_ch, &w_max_ch, &h_max_ch);


    // size expanders to max possible sizes
    WidgetTreeExpand_Rec(w);


    // position the now wrapped and expanded widgets
    List<Widget*> all_widgets = WidgetTreePositioning(a_tmp, w);


    // render pass
    WidgetTreeRenderToDrawcalls(all_widgets);


    // clean up pass
    _g_w_root.frame_touched = *g_frameno_imui;
    g_w_layout = &_g_w_root;
    for (u32 i = 0; i < all_widgets.len; ++i) {
        Widget *w = all_widgets.lst[i];

        // prune
        if (w->frame_touched < *g_frameno_imui) {
            MapRemove(g_m_widgets, w->hash_key, w);
            g_p_widgets->Free(w);
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


bool UI_Button(const char *text, Widget **w_out = NULL) {
    u64 key = HashStringValue(text);

    Widget *w = (Widget*) MapGet(g_m_widgets, key);
    if (w == NULL) {
        w = g_p_widgets->Alloc();
        w->features |= WF_DRAW_TEXT;
        w->features |= WF_DRAW_BACKGROUND_AND_BORDER;

        w->w = 120;
        w->h = 50;
        w->text = Str { (char*) text, _strlen( (char*) text) };
        w->sz_font = FS_24;

        MapPut(g_m_widgets, key, w);
    }
    w->frame_touched = *g_frameno_imui;

    bool hot = w->rect.DidCollide( g_mouse_imui->x, g_mouse_imui->y ) && (g_w_active == NULL || g_w_active == w);
    if (hot) {
        if (g_mouse_imui->l) {
            g_w_active = w;
        }
    }
    bool active = (g_w_active == w);
    bool clicked = active && hot && (g_mouse_imui->dl == 1 || g_mouse_imui->ClickedRecently());

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
        g_w_hot = w;

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

    TreeSibling(w);

    if (w_out != NULL) {
        *w_out = w;
    }
    return clicked;
}


bool UI_PushButton(const char *text, bool *pushed, Widget **w_out = NULL) {
    u64 key = HashStringValue(text);

    Widget *w = (Widget*) MapGet(g_m_widgets, key);
    if (w == NULL) {
        w = g_p_widgets->Alloc();
        w->features |= WF_DRAW_TEXT;
        w->features |= WF_DRAW_BACKGROUND_AND_BORDER;

        w->w = 120;
        w->h = 50;
        w->text = Str { (char*) text, _strlen( (char*) text) };
        w->sz_font = FS_24;

        MapPut(g_m_widgets, key, w);
    }
    w->frame_touched = *g_frameno_imui;

    bool hot = w->rect.DidCollide( g_mouse_imui->x, g_mouse_imui->y ) && (g_w_active == NULL || g_w_active == w);
    if (hot) {
        if (g_mouse_imui->l) {
            g_w_active = w;
        }
    }
    bool active = (g_w_active == w) || *pushed;
    bool clicked = active && hot && (g_mouse_imui->dl == 1 || g_mouse_imui->ClickedRecently());

    if (clicked) {
        *pushed = !(*pushed);
        clicked = *pushed;
    }

    if (active) {
        // ACTIVE: mouse-down was engaged on this element

        // configure active properties
        w->sz_border = 1;
        w->col_bckgrnd = ColorGray(0.8f); // panel
        w->col_text = ColorBlack();        // text
        w->col_border = ColorBlack();     // border

        if (hot) {
            w->sz_border = 3;
        }
    }
    else if (hot) {
        // HOT: currently hovering the mouse
        g_w_hot = w;

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

    TreeSibling(w);

    if (w_out != NULL) {
        *w_out = w;
    }
    return clicked;
}


Widget *UI_CoolPanel(s32 width, s32 height) {
    // no frame persistence

    Widget *w = g_p_widgets->Alloc();
    w->frame_touched = 0;
    w->features |= WF_DRAW_BACKGROUND_AND_BORDER;
    w->features |= WF_LAYOUT_HORIZONTAL_CENTERING;
    w->features |= WF_LAYOUT_VERTICAL;
    w->w = width;
    w->h = height;
    w->sz_border = 20;
    w->col_bckgrnd = ColorGray(0.9f);
    w->col_border = ColorGray(0.7f);

    TreeBranch(w);

    return w;
}


void UI_SpaceH(u32 width) {
    // no frame persistence

    Widget *w = g_p_widgets->Alloc();
    w->frame_touched = 0;
    w->w = width;

    TreeSibling(w);
}


void UI_SpaceV(u32 height) {
    // no frame persistence

    Widget *w = g_p_widgets->Alloc();
    w->frame_touched = 0;
    w->h = height;

    TreeSibling(w);
}


Widget *UI_Label(const char *text, Color color = Color { RGBA_BLACK }) {
    // no frame persistence

    Widget *w = g_p_widgets->Alloc();
    w->frame_touched = 0;
    w->features |= WF_DRAW_TEXT;

    w->text = Str { (char*) text, _strlen( (char*) text) };
    w->sz_font = GetFontSize();
    w->col_bckgrnd = ColorGray(0.9f);
    w->col_border = ColorBlack();
    w->col_text = color;

    FontSize fs = GetFontSize();
    SetFontSize(w->sz_font);
    w->w = TextLineWidth(g_text_plotter, w->text);;
    w->h = g_text_plotter->ln_measured;
    SetFontSize(fs);

    TreeSibling(w);
    return w;
}


Widget *UI_LayoutHorizontal() {
    Widget *w = g_p_widgets->Alloc();
    w->frame_touched = 0;
    w->features |= WF_LAYOUT_HORIZONTAL;

    TreeBranch(w);
    return w;
}


Widget *UI_LayoutVertical() {
    Widget *w = g_p_widgets->Alloc();
    w->frame_touched = 0;
    w->features |= WF_LAYOUT_VERTICAL;

    TreeBranch(w);
    return w;
}


Widget *UI_LayoutVerticalCenterX() {
    // no frame persistence

    Widget *w = g_p_widgets->Alloc();
    w->frame_touched = 0;
    w->features |= WF_LAYOUT_HORIZONTAL_CENTERING;
    w->features |= WF_LAYOUT_VERTICAL;

    TreeBranch(w);
    return w;
}


Widget *UI_LayoutHorizontalCenterY() {
    // no frame persistence

    Widget *w = g_p_widgets->Alloc();
    w->frame_touched = 0;
    w->features |= WF_LAYOUT_VERTICAL_CENTERING;
    w->features |= WF_LAYOUT_HORIZONTAL;

    TreeBranch(w);
    return w;
}


void UI_Pop() {
    TreePop();
}


#endif
