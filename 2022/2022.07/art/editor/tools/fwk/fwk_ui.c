#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024

static struct nk_context *ui_ctx;
static struct nk_glfw nk_glfw = {0};

static void nk_config_custom_fonts() {
    #define ICON_FONTNAME "MaterialIconsSharp-Regular.otf" // "MaterialIconsOutlined-Regular.otf" "MaterialIcons-Regular.ttf" // 
    #define ICON_FONTSIZE 20
    #define ICON_MIN ICON_MIN_MD
    #define ICON_MED ICON_MAX_16_MD
    #define ICON_MAX ICON_MAX_MD
    #define ICON_SPACING_X 0
    #define ICON_SPACING_Y 0

    #define ICON_BARS        ICON_MD_MENU
    #define ICON_FILE        ICON_MD_INSERT_DRIVE_FILE
    #define ICON_TRASH       ICON_MD_DELETE

    struct nk_font *font = NULL;
    struct nk_font_atlas *atlas = NULL;
    nk_glfw3_font_stash_begin(&nk_glfw, &atlas); // nk_sdl_font_stash_begin(&atlas);

        // Default font(#1)...

        for( char *data = vfs_read("Carlito-Regular.ttf"); data; data = 0 ) {
            float font_size = 14.5f;
                struct nk_font_config cfg = nk_font_config(font_size);
                cfg.oversample_v = 2;
                cfg.pixel_snap = 0;
            // win32: struct nk_font *arial = nk_font_atlas_add_from_file(atlas, va("%s/fonts/arial.ttf",getenv("windir")), font_size, &cfg); font = arial ? arial : font;
            // struct nk_font *droid = nk_font_atlas_add_from_file(atlas, "nuklear/extra_font/DroidSans.ttf", font_size, &cfg); font = droid ? droid : font;
            struct nk_font *regular = nk_font_atlas_add_from_memory(atlas, data, vfs_size("Carlito-Regular.ttf"), font_size, &cfg); font = regular ? regular : font;
        }

        // ...with icons embedded on it.

        for( char *data = vfs_read(ICON_FONTNAME); data; data = 0 ) {
            static const nk_rune icon_range[] = {ICON_MIN, ICON_MED /*MAX*/, 0};

            struct nk_font_config cfg = nk_font_config(ICON_FONTSIZE);
            cfg.range = icon_range; // nk_font_default_glyph_ranges();
            cfg.merge_mode = 1;

            cfg.spacing.x += ICON_SPACING_X;
            cfg.spacing.y += ICON_SPACING_Y;
         // cfg.font->ascent += ICON_ASCENT;
         // cfg.font->height += ICON_HEIGHT;

            cfg.oversample_h = 1;
            cfg.oversample_v = 1;
            cfg.pixel_snap = 1;

            struct nk_font *icons = nk_font_atlas_add_from_memory(atlas, data, vfs_size(ICON_FONTNAME), ICON_FONTSIZE, &cfg);
        }

        // Monospaced font. Used in terminals or consoles.

        for( char *data = vfs_read("Inconsolata-Regular.ttf"); data; data = 0 ) {
            const float fontsize = 14.f; // 18.f;
            static const nk_rune icon_range[] = {32, 127, 0};

            struct nk_font_config cfg = nk_font_config(fontsize);
            cfg.range = icon_range;

            cfg.oversample_h = 1;
            cfg.oversample_v = 1;
            cfg.pixel_snap = 1;

            // struct nk_font *proggy = nk_font_atlas_add_default(atlas, fontsize, &cfg);
            struct nk_font *bold = nk_font_atlas_add_from_memory(atlas, data, vfs_size("Inconsolata-Regular.ttf"), fontsize, &cfg);
        }

        // Extra optional fonts from here...

        for( char *data = vfs_read("Carlito-BoldItalic.ttf"); data; data = 0 ) {
            struct nk_font *bold = nk_font_atlas_add_from_memory(atlas, data, vfs_size("Carlito-BoldItalic.ttf"), 16.f, 0); // font = bold ? bold : font;
        }

    nk_glfw3_font_stash_end(&nk_glfw); // nk_sdl_font_stash_end();
//  ASSERT(font);
    if(font) nk_style_set_font(ui_ctx, &font->handle);

    // Load Cursor: if you uncomment cursor loading please hide the cursor
    // nk_style_load_all_cursors(ctx, atlas->cursors); glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

static void nk_config_custom_style() {
    nk_style_default(ui_ctx);
    struct nk_color blue_gray = nk_rgba(53, 88, 156, 255);
    struct nk_color washed_cyan = nk_rgb_f(0.35,0.6,0.8);
    struct nk_color vivid_cyan = nk_rgb_f(0.0,0.75,1.0);
    struct nk_color main_color = blue_gray;
    struct nk_color hover_color = washed_cyan;
    struct nk_color lit_color = vivid_cyan;
    struct nk_color table[NK_COLOR_COUNT] = {0};
    table[NK_COLOR_TEXT] = nk_rgba(210, 210, 210, 255);
    table[NK_COLOR_WINDOW] = nk_rgba(42, 42, 42, 215);
    table[NK_COLOR_HEADER] = nk_rgba(51, 51, 56, 220);
    table[NK_COLOR_BORDER] = nk_rgba(46, 46, 46, 255);
    table[NK_COLOR_BUTTON] = main_color;
    table[NK_COLOR_BUTTON_HOVER] = hover_color;
    table[NK_COLOR_BUTTON_ACTIVE] = lit_color;
    // ok
    table[NK_COLOR_TOGGLE] = nk_rgba(45*1.2, 53*1.2, 56*1.2, 255); // table[NK_COLOR_WINDOW]; // nk_rgba(45/1.2, 53/1.2, 56/1.2, 255);
    table[NK_COLOR_TOGGLE_HOVER] = vivid_cyan;
    table[NK_COLOR_TOGGLE_CURSOR] = main_color; // vivid_blue;
    table[NK_COLOR_SCROLLBAR] = nk_rgba(50, 58, 61, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR] = main_color;
    table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = hover_color;
    table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = lit_color;
    table[NK_COLOR_SLIDER] = nk_rgba(50, 58, 61, 255);
    table[NK_COLOR_SLIDER_CURSOR] = main_color;
    table[NK_COLOR_SLIDER_CURSOR_HOVER] = hover_color;
    table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = lit_color;
    table[NK_COLOR_EDIT] = nk_rgba(50, 58, 61, 225);
    table[NK_COLOR_EDIT_CURSOR] = nk_rgba(210, 210, 210, 255);

    // table[NK_COLOR_COMBO] = nk_rgba(50, 58, 61, 255);

    // table[NK_COLOR_PROPERTY] = nk_rgba(50, 58, 61, 255);
table[NK_COLOR_CHART] = nk_rgba(50, 58, 61, 255);
table[NK_COLOR_CHART_COLOR] = main_color;
table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = hover_color; // nk_rgba(255, 0, 0, 255);
    // table[NK_COLOR_TAB_HEADER] = main_color;
    // table[NK_COLOR_SELECT] = nk_rgba(57, 67, 61, 255);
    // table[NK_COLOR_SELECT_ACTIVE] = main_color;

    nk_style_from_table(ui_ctx, table);

    struct nk_style *s = &ui_ctx->style;
    s->window.spacing = nk_vec2(4,0);
    s->window.combo_border = 0.f;
    s->window.scrollbar_size = nk_vec2(5,5);
    s->property.rounding = 0;
    s->combo.border = 0;
    s->button.border = 1;
    s->edit.border = 0;
}

static float ui_alpha = 1;
static array(float) ui_alphas;
static void ui_alpha_push(float alpha) {
    array_push(ui_alphas, ui_alpha);
    ui_alpha = alpha;

    struct nk_color c;
    struct nk_style *s = &ui_ctx->style;
    c = s->window.background;                  c.a = alpha * 255; nk_style_push_color(ui_ctx, &s->window.background, c);
    c = s->text.color;                         c.a = alpha * 255; nk_style_push_color(ui_ctx, &s->text.color, c);
    c = s->window.fixed_background.data.color; c.a = alpha * 255; nk_style_push_style_item(ui_ctx, &s->window.fixed_background, nk_style_item_color(c));
}
static void ui_alpha_pop() {
    if( array_count(ui_alphas) ) {
        nk_style_pop_style_item(ui_ctx);
        nk_style_pop_color(ui_ctx);
        nk_style_pop_color(ui_ctx);

        ui_alpha = *array_back(ui_alphas);
        array_pop(ui_alphas);
    }
}

// -----------------------------------------------------------------------------
// ui menu

static array(char*) ui_items; // queued menu names. to be evaluated during next frame
static vec2 ui_results = {0}; // clicked menu items from last frame

int ui_item() {
    return ui_items ? (ui_results.x == array_count(ui_items) ? ui_results.y : 0) : 0;
}

int ui_menu(const char *items) { // semicolon- or comma-separated items
    array_push(ui_items, STRDUP(items));
    return ui_item();
}

int ui_has_menubar() {
    return !!ui_items; // array_count(ui_items) > 0;
}

static
void ui_separator_line() {
    struct nk_rect space; nk_layout_peek(&space, ui_ctx); // bounds.w *= 0.95f;
    struct nk_command_buffer *canvas = nk_window_get_canvas(ui_ctx);
    nk_stroke_line(canvas, space.x+0,space.y+0,space.x+space.w,space.y+0, 3.0, nk_rgb(128,128,128));
}

NK_API nk_bool
nk_menu_begin_text_styled(struct nk_context *ctx, const char *title, int len,
    nk_flags align, struct nk_vec2 size, struct nk_style_button *style_button) //< @r-lyeh: added style_button param
{
    struct nk_window *win;
    const struct nk_input *in;
    struct nk_rect header;
    int is_clicked = nk_false;
    nk_flags state;

    NK_ASSERT(ctx);
    NK_ASSERT(ctx->current);
    NK_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout)
        return 0;

    win = ctx->current;
    state = nk_widget(&header, ctx);
    if (!state) return 0;
    in = (state == NK_WIDGET_ROM || win->flags & NK_WINDOW_ROM) ? 0 : &ctx->input;
    if (nk_do_button_text(&ctx->last_widget_state, &win->buffer, header,
        title, len, align, NK_BUTTON_DEFAULT, style_button, in, ctx->style.font))
        is_clicked = nk_true;
    return nk_menu_begin(ctx, win, title, is_clicked, header, size);
}


static
vec2 ui_toolbar_(array(char*) ui_items, vec2 ui_results, float min_width) {
//    nk_layout_row_dynamic(ui_ctx, 0, 1);

#if 0
    nk_layout_row_dynamic(ui_ctx, 25/*h*/, array_count(ui_items));
#else
    // adjust size for all the upcoming UI elements
    {
        const struct nk_style *style = &ui_ctx->style;
        const struct nk_user_font *f = style->font;

        nk_layout_row_template_begin(ui_ctx, 25/*h*/);
        for(int i = 0; i < array_count(ui_items); ++i) {
            char first_token[512];
            sscanf(ui_items[i], "%[^,;|]", first_token); // @fixme: vsnscanf

            char *tooltip = strchr(first_token, '@');
            int len = tooltip ? (int)(tooltip - first_token /*- 1*/) : strlen(first_token);

            float pixels_width = f->width(f->userdata, f->height, first_token, len);
            if( min_width ) pixels_width += ( style->window.header.label_padding.x + style->window.header.padding.x ) * 2;
            if( pixels_width < min_width ) pixels_width = min_width;
            nk_layout_row_template_push_static(ui_ctx, pixels_width);
        }
        nk_layout_row_template_end(ui_ctx);
    }
#endif

    // display the UI elements
    bool has_popups = ui_popups();
    for( int i = 0, end = array_count(ui_items); i < end; ++i ) {
        array(char*) ids = strsplit(ui_items[i], ",;|");

        // transparent style
        static struct nk_style_button transparent_style;
        do_once transparent_style = ui_ctx->style.button;
        do_once transparent_style.normal.data.color = nk_rgba(0,0,0,0);
        do_once transparent_style.border_color = nk_rgba(0,0,0,0);
        do_once transparent_style.active = transparent_style.normal;
        do_once transparent_style.hover = transparent_style.normal;
        do_once transparent_style.hover.data.color = nk_rgba(0,0,0,127);
        transparent_style.text_alignment = NK_TEXT_ALIGN_CENTERED|NK_TEXT_ALIGN_MIDDLE; // array_count(ids) > 1 ? NK_TEXT_ALIGN_LEFT : NK_TEXT_ALIGN_CENTERED;

        char *tooltip = strchr(ids[0], '@');
        int len = tooltip ? (int)(tooltip -  ids[0]) : strlen(ids[0]);

        // single button
        if( array_count(ids) == 1 ) {
            // tooltip
            if( tooltip && !has_popups ) {
                struct nk_rect bounds = nk_widget_bounds(ui_ctx);
                if (nk_input_is_mouse_hovering_rect(&ui_ctx->input, bounds) && nk_window_has_focus(ui_ctx)) {
                    nk_tooltip(ui_ctx, tooltip+1);
                }
            }
            // text
            if( nk_button_text_styled(ui_ctx, &transparent_style, ids[0], len) ) {
                ui_results = vec2(i+1, 0+1);
            }
        }
        else {
            struct nk_vec2 dims = {120, array_count(ids)* 32};
            const struct nk_style *style = &ui_ctx->style;
            const struct nk_user_font *f = style->font;
            static array(float) lens = 0; array_resize(lens, array_count(ids));
            lens[0] = len;
            for( int j = 1; j < array_count(ids); ++j ) {
                lens[j] = strlen(ids[j]);
                float width_px = f->width(f->userdata, f->height, ids[j], lens[j]);
                dims.x = maxf(dims.x, width_px);
            }
            dims.x += 2 * style->window.header.label_padding.x;

            // dropdown menu
            if( nk_menu_begin_text_styled(ui_ctx, ids[0], lens[0], NK_TEXT_ALIGN_CENTERED|NK_TEXT_ALIGN_MIDDLE, dims, &transparent_style) ) {
                nk_layout_row_dynamic(ui_ctx, 32, 1);

                for( int j = 1; j < array_count(ids); ++j ) {
                    char *item = ids[j];
                    if( *item == '-' ) {
                        while(*item == '-') ++item, --lens[j];
                        //nk_menu_item_label(ui_ctx, "---", NK_TEXT_LEFT);
                        ui_separator_line();
                    }

                    if( nk_menu_item_text(ui_ctx, item, lens[j], NK_TEXT_LEFT) ) {
                        ui_results = vec2(i+1, j+1-1);
                    }
                }                    

                nk_menu_end(ui_ctx);
            }            
        }
    }

    return ui_results;
}

int ui_toolbar(const char *icons) { // usage: int clicked_icon = ui_toolbar( ICON_1 ";" ICON_2 ";" ICON_3 ";" ICON_4 );
    vec2 results = {0};
    array(char*) items = strsplit(icons, ",;|");
    return ui_toolbar_(items, results, 5).x;
}


// UI Windows handlers. These are not OS Windows but UI Windows instead. For OS Windows check window_*() API.

#ifndef WINDOWS_INI
#define WINDOWS_INI editor_path("windows.ini")
#endif

static map(char*,unsigned) ui_windows = 0;
static int ui_window_register(const char *panel_or_window_title) {
    if(!ui_windows) map_init(ui_windows, less_str, hash_str);
    unsigned *state = map_find_or_add_allocated_key(ui_windows, STRDUP(panel_or_window_title), 0);

    // check for visibility flag on first call
    int visible = 0;
    if( *state == 0 ) {
        static ini_t i = 0;
        do_once i = ini(WINDOWS_INI); // @leak
        char **found = i ? map_find(i, va("%s.visible", panel_or_window_title)) : NULL;
        if( found ) visible = (*found)[0] == '1';
    }

    *state |= 2;
    return visible;
}
int ui_visible(const char *panel_or_window_title) {
    if(!ui_windows) map_init(ui_windows, less_str, hash_str);
    return *map_find_or_add_allocated_key(ui_windows, STRDUP(panel_or_window_title), 0) & 1;
}
int ui_show(const char *panel_or_window_title, int enabled) {
    if(!ui_windows) map_init(ui_windows, less_str, hash_str);
    unsigned *found = map_find_or_add_allocated_key(ui_windows, STRDUP(panel_or_window_title), 0);
    if( enabled ) {
        *found |= 1;
        nk_window_collapse(ui_ctx, panel_or_window_title, NK_MAXIMIZED); // in case windows was previously collapsed
    } else {
        *found &= ~1;
    }
    return !!enabled;
}
static char *ui_build_window_list() {
    if(!ui_windows) map_init(ui_windows, less_str, hash_str);
    char *build_windows_menu = 0;
    strcatf(&build_windows_menu, "%s;", ICON_MD_VIEW_QUILT); // "Windows");
    for each_map_sorted_ptr(ui_windows, char*, k, unsigned, v) {
        strcatf(&build_windows_menu, "%s%s;", ui_visible(*k) ? ICON_MD_TOGGLE_ON : ICON_MD_TOGGLE_OFF, *k);
    }
    strcatf(&build_windows_menu, "-%s;%s", ICON_MD_RECYCLING "Reset layout", ICON_MD_SAVE_AS "Save layout");
    return build_windows_menu; // @leak if discarded
}
static int ui_layout_all_reset(const char *mask);
static int ui_layout_all_save_disk(const char *mask);
static int ui_layout_all_load_disk(const char *mask);


static
void ui_menu_render() {
    // clean up from past frame
    ui_results = vec2(0,0);
    if( !ui_items ) return;
    if( !array_count(ui_items) ) return;

// artificially inject Windows menu on the first icon
bool show_window_menu = !!array_count(ui_items);
if( show_window_menu ) {
    array_push_front(ui_items, ui_build_window_list());
}

    // process menus
    if( nk_begin(ui_ctx, "Menu", nk_rect(0, 0, window_width(), 32), NK_WINDOW_NO_SCROLLBAR/*|NK_WINDOW_BACKGROUND*/)) {
        if( ui_ctx->current ) {
            nk_menubar_begin(ui_ctx);

            ui_results = ui_toolbar_(ui_items, ui_results, 5);

            //nk_layout_row_end(ui_ctx);
            nk_menubar_end(ui_ctx);
        }
    }
    nk_end(ui_ctx);

if( show_window_menu ) {
    // if clicked on first menu (Windows)
    if( ui_results.x == 1 ) {
        array(char*) split = strsplit(ui_items[0],";"); // *array_back(ui_items), ";");
        const char *title = split[(int)ui_results.y]; title += title[0] == '-'; title += 3 * (title[0] == '\xee'); title += title[0] == ' '; /*skip separator+emoji+space*/
        // toggle window unless clicked on lasts items {"reset layout", "save layout"}
        bool clicked_reset_layout = ui_results.y == array_count(split) - 2;
        bool clicked_save_layout = ui_results.y == array_count(split) - 1;
        /**/ if( clicked_reset_layout ) ui_layout_all_reset("*");
        else if( clicked_save_layout ) file_delete(WINDOWS_INI), ui_layout_all_save_disk("*");
        else ui_show(title, ui_visible(title) ^ true);
        // reset value so developers don't catch this click
        ui_results = vec2(0,0);
    }
    // restore state prior to previously injected Windows menu
    else
    ui_results.x = ui_results.x > 0 ? ui_results.x - 1 : 0;
}

    // clean up for next frame
    for( int i = 0; i < array_count(ui_items); ++i ) {
        FREE(ui_items[i]);
    }
    array_resize(ui_items, 0);
}

// -----------------------------------------------------------------------------

static int ui_dirty = 1;
static int ui_has_active_popups = 0;
static float ui_hue = 0; // hue
static int ui_is_hover = 0;
static int ui_is_active = 0;
static uint64_t ui_active_mask = 0;

int ui_popups() {
    return ui_has_active_popups;
}
int ui_hover() {
    return ui_is_hover;
}
int ui_active() {
    return ui_is_active; //window_has_cursor() && nk_window_is_any_hovered(ui_ctx) && nk_item_is_any_active(ui_ctx);
}

static
void ui_destroy(void) {
    if(ui_ctx) {
        nk_glfw3_shutdown(&nk_glfw); // nk_sdl_shutdown();
        ui_ctx = 0;
    }
}
static
void ui_create() {
    do_once atexit(ui_destroy);

    if( ui_dirty ) {
        nk_glfw3_new_frame(&nk_glfw); //g->nk_glfw);
        ui_dirty = 0;
    }
}

enum {
    UI_NOTIFICATION = 1,  // sets panel as notification. used by ui_notify()
};

struct ui_notify {
    char *title;
    char *body; // char *icon;
    float timeout;
    float alpha;
    int   used;
};

static array(struct ui_notify) ui_notifications; // format=("%d*%s\n%s", timeout, title, body)

static
void ui_notify_render() {
    // draw queued notifications
    if( array_count(ui_notifications) ) {
        struct ui_notify *n = array_back(ui_notifications);

        static double timeout = 0;
        timeout += 1/60.f; // window_delta(); // @fixme: use editor_time() instead

        ui_alpha_push( timeout >= n->timeout ? 1 - clampf(timeout - n->timeout,0,1) : 1 );

            if( timeout < (n->timeout + 1) ) { // N secs display + 1s fadeout
                if(n->used++ < 3) nk_window_set_focus(ui_ctx, "!notify");

                if( ui_panel( "!notify", UI_NOTIFICATION ) ) {
                    if(n->title) ui_label(n->title);
                    if(n->body)  ui_label(n->body);
                    ui_panel_end();
                }
            }

            if( timeout >= (n->timeout + 2) ) { // 1s fadeout + 1s idle
                timeout = 0;

                if(n->title) FREE(n->title);
                if(n->body)  FREE(n->body);
                array_pop(ui_notifications);
            }

        ui_alpha_pop();
    }
}

static
void ui_render() {

    // draw queued menus
    ui_notify_render();
    ui_menu_render();

    /* IMPORTANT: `nk_sdl_render` modifies some global OpenGL state
     * with blending, scissor, face culling, depth test and viewport and
     * defaults everything back into a default state.
     * Make sure to either a.) save and restore or b.) reset your own state after
     * rendering the UI. */
    //nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
    nk_glfw3_render(&nk_glfw, NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
#if is(ems)
    glFinish();
#endif
    ui_dirty = 1;
    ui_hue = 0;

    ui_is_hover = nk_window_is_any_hovered(ui_ctx) && window_has_cursor();

    if(input_down(MOUSE_L))
        ui_is_active = (ui_is_hover && nk_item_is_any_active(ui_ctx));
    if(input_up(MOUSE_L))
        ui_is_active = 0;
}


// -----------------------------------------------------------------------------
// save/restore all window layouts on every framebuffer resize

#define UI_SNAP_PX      1 /*treshold of pixels when snapping panels/windows to the application borders [1..N]*/
#define UI_ANIM_ALPHA 0.9 /*animation alpha used when restoring panels/windows state from application resizing events: [0..1]*/
//#define UI_MENUBAR_Y     32 // menubar and row

typedef struct ui_layout {
    const char *title;

    bool is_panel;

    vec2   desktop;
    vec2     p0,p1;
    float    l0,l1;

    float alpha;

} ui_layout;

static ui_layout ui_layouts[2][64] = {0};

static
int ui_layout_find(const char *title, bool is_panel) {
    int i, found = 0;
    for( i = 0; i < 64 && !found; ++i) {
        if (!ui_layouts[is_panel][i].title) break;
        if( strcmp(title, ui_layouts[is_panel][i].title)) continue;
        found = 1;
        break;
    }
    if( !found ) {  
        ui_layouts[is_panel][i].is_panel = is_panel;
        ui_layouts[is_panel][i].title = STRDUP(title);
    }
    return i;
}

static
void ui_layout_save_mem(int idx, vec2 desktop, float workarea_h, struct nk_rect xywh, bool is_panel) {
    ui_layout *s = &ui_layouts[is_panel][idx];

    s->desktop = desktop;

struct nk_window *win = nk_window_find(ui_ctx, s->title);
float excess = 0;
if( win && (win->flags & NK_WINDOW_MINIMIZED)) {
    excess = xywh.h - 32;
    xywh.h = 32;
}

    // sanity checks
    if(xywh.x<0)                               xywh.x = 0;
    if(xywh.w>desktop.w-UI_SNAP_PX)            xywh.w = desktop.w-UI_SNAP_PX-1;

    if(xywh.y<workarea_h)                      xywh.y = workarea_h;
    if(xywh.h>desktop.h-workarea_h-UI_SNAP_PX) xywh.h = desktop.h-workarea_h-UI_SNAP_PX-1;

    if((xywh.x+xywh.w)>desktop.w)              xywh.x-= xywh.x+xywh.w-desktop.w;
    if((xywh.y+xywh.h)>desktop.h)              xywh.y-= xywh.y+xywh.h-desktop.h;

if( win && (win->flags & NK_WINDOW_MINIMIZED)) {
    xywh.h += excess;
}

    // build reconstruction vectors from bottom-right corner
    s->p0 = vec2(xywh.x/s->desktop.x,xywh.y/s->desktop.y);
    s->p1 = vec2(xywh.w/s->desktop.x,xywh.h/s->desktop.y);
    s->p0 = sub2(s->p0, vec2(1,1)); s->l0 = len2(s->p0);
    s->p1 = sub2(s->p1, vec2(1,1)); s->l1 = len2(s->p1);
}

static
struct nk_rect ui_layout_load_mem(int idx, vec2 desktop, bool is_panel) {
    ui_layout *s = &ui_layouts[is_panel][idx];

    // extract reconstruction coords from bottom-right corner
    vec2 p0 = mul2(add2(vec2(1,1), scale2(norm2(s->p0), s->l0)), desktop);
    vec2 p1 = mul2(add2(vec2(1,1), scale2(norm2(s->p1), s->l1)), desktop);

    return (struct nk_rect){ p0.x, p0.y, p1.x, p1.y };
}

static int ui_layout_reset_num_frames;

static
int ui_layout_all_reset(const char *mask) {
    ui_layout z = {0};

    vec2 desktop = vec2(window_width(), window_height());
    float workarea_h = ui_has_menubar()*32; // @fixme workarea -> reserved_area

    for( int is_panel = 0; is_panel < 2; ++is_panel ) {
        for( int j = 0; j < 64; ++j ) {
            if( ui_layouts[is_panel][j].title ) {
                struct nk_rect xywh = { 0, workarea_h + j * 32, desktop.w / 3.333, 32 };
                if( is_panel ) {
                    xywh.x = 0;
                    xywh.y = workarea_h + j * 32;
                    xywh.w = desktop.w / 4;
                    xywh.h = desktop.h / 3;
                } else {
                    xywh.x = desktop.w / 3.00 + j * 32;
                    xywh.y = workarea_h + j * 32;
                    xywh.w = desktop.w / 4;
                    xywh.h = desktop.h / 3;
                }
                nk_window_set_focus(ui_ctx, ui_layouts[is_panel][j].title );
                nk_window_collapse(ui_ctx, ui_layouts[is_panel][j].title, is_panel ? 0 : 1);
                ui_layout_save_mem(j, desktop, workarea_h, xywh, is_panel);
            }
        }
    }

    ui_layout_reset_num_frames = maxf(desktop.w, desktop.h);
    return 1;
}

static
int ui_layout_all_save_disk(const char *mask) {
    float w = window_width(), h = window_height();
    for each_map_sorted_ptr(ui_windows, char*, k, unsigned, v) {
        struct nk_window *win = nk_window_find(ui_ctx, *k);
        if( win && strmatchi(*k, mask) ) {
            ini_write(WINDOWS_INI, *k, "x", va("%f", win->bounds.x / w ));
            ini_write(WINDOWS_INI, *k, "y", va("%f", win->bounds.y / h ));
            ini_write(WINDOWS_INI, *k, "w", va("%f", win->bounds.w / w ));
            ini_write(WINDOWS_INI, *k, "h", va("%f", win->bounds.h / h ));
            ini_write(WINDOWS_INI, *k, "visible", ui_visible(*k) ? "1":"0");
        }
    }
    return 1;
}

static
const char *ui_layout_load_disk(const char *title, const char *mask, ini_t i, struct nk_rect *r) {
    if(!i) return 0;

    const char *dot = strrchr(title, '.');
    if( dot ) title = va("%.*s", (int)(dot - title), title);
    if( !strmatchi(title, mask) ) return 0;

    char **x = map_find(i, va("%s.x", title));
    char **y = map_find(i, va("%s.y", title));
    char **w = map_find(i, va("%s.w", title));
    char **h = map_find(i, va("%s.h", title));
    if( x && y && w && h ) {
        float ww = window_width(), wh = window_height();
        r->x = atof(*x) * ww;
        r->y = atof(*y) * wh;
        r->w = atof(*w) * ww;
        r->h = atof(*h) * wh;

        char **on = map_find(i, va("%s.visible", title));

        return title;
    }
    return 0;
}

static
int ui_layout_all_load_disk(const char *mask) {
    ini_t i = ini(WINDOWS_INI); // @leak
    if( !i ) return 0;
    for each_map(i, char*, k, char*, v) {
        struct nk_rect out = {0};
        const char *title = ui_layout_load_disk(k, mask, i, &out);
        if( title ) {
            struct nk_window *win = nk_window_find(ui_ctx, title);
            if( win ) {            
                win->bounds.x = out.x;
                win->bounds.y = out.y;
                win->bounds.w = out.w;
                win->bounds.h = out.h;
            }
        }
    }
    return 1;
}


// -----------------------------------------------------------------------------
// shared code for both panels and windows. really messy.

static
int ui_begin_panel_or_window_(const char *title, int flags, bool is_window) {

struct nk_window *win = nk_window_find(ui_ctx, title);

int is_panel = !is_window;
bool starts_minimized = is_panel;
bool is_closable = is_window;
bool is_scalable = true;
bool is_movable = true;
bool is_auto_minimizes = false;
bool is_pinned = win && (win->flags & NK_WINDOW_PINNED);

if( is_pinned ) {
//    is_closable = false;
    is_auto_minimizes = false;
//    is_scalable = false;
    is_movable = false;
}

    ui_create();

    uint64_t hash = 14695981039346656037ULL, mult = 0x100000001b3ULL;
    for(int i = 0; title[i]; ++i) hash = (hash ^ title[i]) * mult;
    ui_hue = (hash & 0x3F) / (float)0x3F; ui_hue += !ui_hue;

    int idx = ui_layout_find(title, is_panel);
    ui_layout *s = &ui_layouts[is_panel][idx];

vec2 desktop = vec2(window_width(), window_height());
float workarea_h = ui_has_menubar()*32;

    int row = idx + !!ui_has_menubar(); // add 1 to skip menu
    vec2 offset = vec2(is_panel ? 0 : 32*row + 200, 32*row + is_window * 75);
    float w = desktop.w / 3.33, h = flags & UI_NOTIFICATION ? 64 : desktop.h - offset.y * 2 - 1; // h = desktop.h * 0.66; // 
    struct nk_rect start_coords = {offset.x, offset.y, offset.x+w, offset.y+h};

if(is_window) {
    w = desktop.w / 1.5;
    h = desktop.h / 1.5;
    start_coords.x = (desktop.w-w)/2;
    start_coords.y = (desktop.h-h)/2 + workarea_h/2;
    start_coords.w = w;
    start_coords.h = h;
}

    static vec2 edge = {0}; static int edge_type = 0; // [off],L,R,U,D
    do_once edge = vec2(desktop.w * 0.33, desktop.h * 0.66);

// do not snap windows and/or save windows when using may be interacting with UI
int is_active = 0;
int mouse_pressed = !!input(MOUSE_L) && ui_ctx->active == win;
if( win ) {
    // update global window activity bitmask
    is_active = ui_ctx->active == win;
    ui_active_mask = is_active ? ui_active_mask | (1ull << idx) : ui_active_mask & ~(1ull << idx);
}

//  struct nk_style *s = &ui_ctx->style;
//  nk_style_push_color(ui_ctx, &s->window.header.normal.data.color, nk_hsv_f(ui_hue,0.6,0.8));

// adjust inactive edges automatically
if( win ) {
    bool group1_any             = !is_active; // && !input(MOUSE_L);
    bool group2_not_resizing    =  is_active && !win->is_window_resizing;
    bool group2_interacting     =  is_active && input(MOUSE_L);

#if 0
    if( group1_any ) {
        // cancel self-adjust if this window is not overlapping the active one that is being resized at the moment
        struct nk_window *parent = ui_ctx->active;

        struct nk_rect a = win->bounds, b = parent->bounds;
        bool overlap = a.x <= (b.x+b.w) && b.x <= (a.x+a.w) && a.y <= (b.y+b.h) && b.y <= (a.y+a.h);

        group1_any = overlap;
    }
#else
    if( group1_any )
        group1_any = !(win->flags & NK_WINDOW_PINNED);
#endif

    if( group1_any ) {
        float mouse_x = clampf(input(MOUSE_X), 0, desktop.w);
        float mouse_y = clampf(input(MOUSE_Y), 0, desktop.h);
        float distance_x = absf(mouse_x - win->bounds.x) / desktop.w;
        float distance_y = absf(mouse_y - win->bounds.y) / desktop.h;
        float alpha_x = sqrt(sqrt(distance_x)); // amplify signals a little bit: 0.1->0.56,0.5->0.84,0.98->0.99,etc
        float alpha_y = sqrt(sqrt(distance_y));

        /**/ if( (edge_type & 1) && win->bounds.x <= UI_SNAP_PX ) {
            win->bounds.w = win->bounds.w * alpha_y + edge.w * (1-alpha_y);
        }
        else if( (edge_type & 2) && (win->bounds.x + win->bounds.w) >= (desktop.w-UI_SNAP_PX) ) {
            win->bounds.w = win->bounds.w * alpha_y + edge.w * (1-alpha_y);
            win->bounds.x = desktop.w - win->bounds.w;
        }
        if( (edge_type & 8) && (win->bounds.y + (win->flags & NK_WINDOW_MINIMIZED ? 32 : win->bounds.h)) >= (desktop.h-UI_SNAP_PX) ) {
            win->bounds.h = win->bounds.h * alpha_x + edge.h * (1-alpha_x);
            win->bounds.y = desktop.h - (win->flags & NK_WINDOW_MINIMIZED ? 32 : win->bounds.h);
        }        
    }

    if( group1_any || !group2_interacting || (ui_layout_reset_num_frames > 0)) {
        struct nk_rect target = ui_layout_load_mem(idx, desktop, is_panel);
        float alpha = len2sq(sub2(s->desktop, desktop)) ? 0 : UI_ANIM_ALPHA; // smooth unless we're restoring a desktop change 
        win->bounds = (struct nk_rect){ 
            win->bounds.x * alpha + target.x * (1 - alpha),
            win->bounds.y * alpha + target.y * (1 - alpha),
            win->bounds.w * alpha + target.w * (1 - alpha),
            win->bounds.h * alpha + target.h * (1 - alpha)
        };
    }
    int skip_save = ui_layout_reset_num_frames > 0;
    if( ui_layout_reset_num_frames > 0 ) ui_layout_reset_num_frames--;
    if( !skip_save ) { // group1_any || group2_interacting ) {
        ui_layout_save_mem(idx, desktop, workarea_h, win->bounds, is_panel);
    }
} else {
    ui_layout_save_mem(idx, desktop, workarea_h, start_coords, is_panel);    
}


    int window_flags = NK_WINDOW_PINNABLE | NK_WINDOW_MINIMIZABLE | NK_WINDOW_NO_SCROLLBAR_X;
    if( starts_minimized ) window_flags |= (win ? 0 : NK_WINDOW_MINIMIZED);
    if( is_auto_minimizes ) window_flags |= is_active ? 0 : NK_WINDOW_MINIMIZED;
    if( is_movable )  window_flags |= NK_WINDOW_MOVABLE;
    if( is_closable ) window_flags |= NK_WINDOW_CLOSABLE;
    if( is_scalable ) {
        window_flags |= NK_WINDOW_SCALABLE;
        if(win) window_flags |= input(MOUSE_X) < (win->bounds.x + win->bounds.w/2) ? NK_WINDOW_SCALE_LEFT : 0;
        if(win) window_flags |= input(MOUSE_Y) < (win->bounds.y + win->bounds.h/2) ? NK_WINDOW_SCALE_TOP : 0;
    }

//    if( is_pinned )
        window_flags |= NK_WINDOW_BORDER;

if( is_panel && win && !is_active ) {
    if( !is_pinned ) {
        window_flags |= NK_WINDOW_MINIMIZED;
    }
}

// if( is_modal ) window_flags &= ~(NK_WINDOW_MINIMIZED | NK_WINDOW_MINIMIZABLE);
if( is_panel && win ) {
//    if( win->bounds.x > 0 && (win->bounds.x+win->bounds.w) < desktop.w-1 ) window_flags &= ~NK_WINDOW_MINIMIZED;
}

if(!win) { // if newly created window (!win)
    // first time, try to restore from WINDOWS_INI file
    static ini_t i; do_once i = ini(WINDOWS_INI); // @leak
    ui_layout_load_disk(title, "*", i, &start_coords);
    ui_layout_save_mem(idx, desktop, workarea_h, start_coords, is_panel);
}

bool is_notify = flags & UI_NOTIFICATION;
if( is_notify ) {
    window_flags = NK_WINDOW_MOVABLE | NK_WINDOW_NOT_INTERACTIVE | NK_WINDOW_NO_SCROLLBAR;
    start_coords = (struct nk_rect){desktop.w / 2 - w / 2, -h, w, h};
}

    if( nk_begin(ui_ctx, title, start_coords, window_flags) ) {

// set width for all inactive panels
struct nk_rect bounds = nk_window_get_bounds(ui_ctx); 
if( mouse_pressed && win && win->is_window_resizing ) {
    edge = vec2(bounds.w, bounds.h);

    // push direction
    int top  = !!(win->is_window_resizing & NK_WINDOW_SCALE_TOP);
    int left = !!(win->is_window_resizing & NK_WINDOW_SCALE_LEFT), right = !left;

    edge_type = 0;
    /**/ if( right && (win->bounds.x <= UI_SNAP_PX) ) edge_type |= 1;
    else if(  left && (win->bounds.x + win->bounds.w) >= (desktop.w-UI_SNAP_PX) ) edge_type |= 2;
    /**/ if(   top && (win->bounds.y + win->bounds.h) >= (desktop.h-UI_SNAP_PX) ) edge_type |= 8;

    // @fixme
    // - if window is in a corner (sharing 2 edges), do not allow for multi edges. either vert or horiz depending on the clicked scaler
    // - or maybe, only propagate edge changes to the other windows that overlapping our window.
}

        return 1;
    } else {

if(is_panel) {
   ui_panel_end(); 
} else ui_window_end();

        return 0;
    }
}

static const char *ui_last_title = 0;
static int *ui_last_enabled = 0;
static int ui_has_window = 0;
static int ui_window_has_menubar = 0;
int ui_window(const char *title, int *enabled) {
    if( window_width() <= 0 ) return 0;
    if( window_height() <= 0 ) return 0;
    if( !ui_ctx || !ui_ctx->style.font ) return 0;

    bool forced_creation = ( enabled && *enabled );
    forced_creation |= ui_window_register(title);
    if(!ui_visible(title)) {
        if( !forced_creation ) return 0;
        ui_show(title, forced_creation);
    }

    ui_last_enabled = enabled;
    ui_last_title = title;
    ui_has_window = 1;
    return ui_begin_panel_or_window_(title, /*flags*/0, true);
}
int ui_window_end() {
    if(ui_window_has_menubar) nk_menubar_end(ui_ctx), ui_window_has_menubar = 0;
    nk_end(ui_ctx), ui_has_window = 0;

    if( nk_window_is_hidden(ui_ctx, ui_last_title) ) {
        nk_window_close(ui_ctx, ui_last_title);
        ui_show(ui_last_title, false);
        if( ui_last_enabled ) *ui_last_enabled = 0; // clear developers' flag
    }
    return 0;
}

int ui_panel(const char *title, int flags) {
    if( window_width() <= 0 ) return 0;
    if( window_height() <= 0 ) return 0;
    if( !ui_ctx || !ui_ctx->style.font ) return 0;

    if( ui_has_window ) {
        // transparent style
        static struct nk_style_button transparent_style;
        do_once transparent_style = ui_ctx->style.button;
        do_once transparent_style.normal.data.color = nk_rgba(0,0,0,0);
        do_once transparent_style.border_color = nk_rgba(0,0,0,0);
        do_once transparent_style.active = transparent_style.normal;
        do_once transparent_style.hover = transparent_style.normal;
        do_once transparent_style.hover.data.color = nk_rgba(0,0,0,127);
        transparent_style.text_alignment = NK_TEXT_ALIGN_CENTERED|NK_TEXT_ALIGN_MIDDLE;

        if(!ui_window_has_menubar) nk_menubar_begin(ui_ctx);
        if(!ui_window_has_menubar) nk_layout_row_begin(ui_ctx, NK_STATIC, 25, 4);
        if(!ui_window_has_menubar) nk_layout_row_push(ui_ctx, 70);
        ui_window_has_menubar = 1;

        return nk_menu_begin_text_styled(ui_ctx, title, strlen(title), NK_TEXT_ALIGN_CENTERED|NK_TEXT_ALIGN_MIDDLE, nk_vec2(220, 200), &transparent_style);    
    }

    return ui_begin_panel_or_window_(title, flags, false);
}
int ui_panel_end() {
    if( ui_has_window ) {
        nk_menu_end(ui_ctx);
        return 0;
    }
    nk_end(ui_ctx);
//  nk_style_pop_color(ui_ctx);
    return 0;
}

static unsigned ui_collapse_state = 0;
int ui_collapse(const char *label, const char *id) { // mask: 0(closed),1(open),2(created)

    uint64_t hash = 14695981039346656037ULL, mult = 0x100000001b3ULL;
    for(int i = 0; id[i]; ++i) hash = (hash ^ id[i]) * mult;
    ui_hue = (hash & 0x3F) / (float)0x3F; ui_hue += !ui_hue;

    ui_collapse_state = nk_tree_base_(ui_ctx, NK_TREE_NODE, 0, label, NK_MINIMIZED, id, strlen(id), 0);
    return ui_collapse_state & 1; // |1 open, |2 clicked, |4 toggled
}
int ui_collapse_clicked() {
    return ui_collapse_state >> 1; // |1 clicked, |2 toggled
}
int ui_collapse_end() {
    return nk_tree_pop(ui_ctx), 1;
}


int ui_context() {
    struct nk_rect bounds = nk_widget_bounds(ui_ctx);
    bounds.y -= 25;
    return ui_popups() ? 0 : nk_contextual_begin(ui_ctx, 0, nk_vec2(150, 300), bounds);
}
int ui_context_end() {
    nk_contextual_end(ui_ctx);
    return 1;
}
int ui_submenu(const char *options) {
    int choice = 0;
    if( ui_context() ) {
        array(char*) tokens = strsplit(options, ";");
        for( int i = 0; i < array_count(tokens) ; ++i ) {
            if( ui_button_transparent(tokens[i]) ) choice = i + 1;
        }
        ui_context_end();
    }
    return choice;
}

// -----------------------------------------------------------------------------
// code for all the widgets

static
int nk_button_transparent(struct nk_context *ctx, const char *text) {
    static struct nk_style_button transparent_style;
    do_once transparent_style = ctx->style.button;
    do_once transparent_style.normal.data.color = nk_rgba(0,0,0,0);
    do_once transparent_style.border_color = nk_rgba(0,0,0,0);
    do_once transparent_style.active = transparent_style.normal;
    do_once transparent_style.hover = transparent_style.normal;
    do_once transparent_style.hover.data.color = nk_rgba(0,0,0,127);
    transparent_style.text_alignment = NK_TEXT_ALIGN_CENTERED|NK_TEXT_ALIGN_MIDDLE;

    return nk_button_label_styled(ctx, &transparent_style, text);
}

static
int ui_label_(const char *label, int alignment) {
    struct nk_rect bounds = nk_widget_bounds(ui_ctx);
    const struct nk_input *input = &ui_ctx->input;
    int is_hovering = nk_input_is_mouse_hovering_rect(input, bounds) && !ui_has_active_popups;
    if( is_hovering ) {
        struct nk_rect winbounds = nk_window_get_bounds(ui_ctx);
        is_hovering &= nk_input_is_mouse_hovering_rect(input, winbounds);
        is_hovering &= nk_window_has_focus(ui_ctx);
    }

    int do_color_tab = label[0] != '!';
    if(!do_color_tab) label++;

    int indent = 8;
    struct nk_window *win = ui_ctx->current;
    struct nk_panel *layout = win->layout;
    layout->at_x += indent;
    layout->bounds.w -= indent;
    if( do_color_tab ) {
        bounds.w = is_hovering ? indent*3/4 : indent/2-1;
        bounds.h -= 1;
        struct nk_command_buffer *canvas = nk_window_get_canvas(ui_ctx);
        nk_fill_rect(canvas, bounds, 0, nk_hsva_f(ui_hue, 0.6f, 0.8f, ui_alpha) );
    }

        const char *split = strchr(label, '@');
            char buffer[128]; if( split ) label = (snprintf(buffer, 128, "%.*s", (int)(split-label), label), buffer);

struct nk_style *style = &ui_ctx->style;
bool emphasis = label[0] == '*'; label += emphasis;
struct nk_font *font = emphasis ? nk_glfw.atlas.fonts->next->next /*3rd font*/ : NULL; // list
if( font && nk_style_push_font(ui_ctx, &font->handle) ) {} else font = 0;
if( font )  nk_style_push_color(ui_ctx, &style->text.color, nk_rgba(255, 255, 255, 255 * ui_alpha));

            nk_label(ui_ctx, label, alignment);

if( font )  nk_style_pop_color(ui_ctx);
if( font )  nk_style_pop_font(ui_ctx);

            if (split && is_hovering && !ui_has_active_popups && nk_window_has_focus(ui_ctx)) {
                nk_tooltip(ui_ctx, split + 1);
            }

    layout->at_x -= indent;
    layout->bounds.w += indent;

    return is_hovering ? nk_input_has_mouse_click_down_in_rect(input, NK_BUTTON_LEFT, layout->bounds, nk_true) : 0;
}

int ui_notify(const char *title, const char *body) {
    struct ui_notify n = {0};
    n.title = title ? stringf("*%s", title) : 0;
    n.body = body ? STRDUP(body) : 0;
    n.timeout = 2; // 4s = 2s timeout (+ 1s fade + 1s idle)
    n.alpha = 1;
    array_push_front(ui_notifications, n);
    return 1;
}

int ui_button_transparent(const char *text) {
    nk_layout_row_dynamic(ui_ctx, 0, 1);
    int align = text[0] == '>' ? (text++, NK_TEXT_RIGHT) : text[0] == '=' ? (text++, NK_TEXT_CENTERED) : text[0] == '<' ? (text++, NK_TEXT_LEFT) : NK_TEXT_CENTERED;
    return !!nk_contextual_item_label(ui_ctx, text, align);
}

int ui_button(const char *s) {
    nk_layout_row_dynamic(ui_ctx, 0, 1);

    struct nk_rect bounds = nk_widget_bounds(ui_ctx);

    const char *split = strchr(s, '@'), *tooltip = split + 1;
    int ret = split ? nk_button_text(ui_ctx, s, (int)(split - s)) : nk_button_label(ui_ctx, s);

    const struct nk_input *in = &ui_ctx->input;
    if (split && nk_input_is_mouse_hovering_rect(in, bounds) && !ui_has_active_popups && nk_window_has_focus(ui_ctx)) {
        nk_tooltip(ui_ctx, tooltip);
    }
    return ret;
}

int ui_buttons(int buttons, ...) {
    nk_layout_row_dynamic(ui_ctx, 0, buttons);

    va_list list;
    va_start(list, buttons);
    int rc = 0;
    for( int i = 0; i < buttons; ++i ) {
        if( nk_button_label(ui_ctx, va_arg(list, const char*) ) ) rc = i+1;
    }
    va_end(list);

    return rc;
}

int ui_label(const char *text) {
    int align = text[0] == '>' ? (text++, NK_TEXT_RIGHT) : text[0] == '=' ? (text++, NK_TEXT_CENTERED) : text[0] == '<' ? (text++, NK_TEXT_LEFT) : NK_TEXT_LEFT;
    nk_layout_row_dynamic(ui_ctx, 0, 1);
    return ui_label_(text, align);
}
int ui_label2(const char *head, const char *text) {
    int align1 = head[0] == '>' ? (head++, NK_TEXT_RIGHT) : head[0] == '=' ? (head++, NK_TEXT_CENTERED) : head[0] == '<' ? (head++, NK_TEXT_LEFT) : NK_TEXT_LEFT;
    int align2 = text[0] == '>' ? (text++, NK_TEXT_RIGHT) : text[0] == '=' ? (text++, NK_TEXT_CENTERED) : text[0] == '<' ? (text++, NK_TEXT_LEFT) : NK_TEXT_LEFT;
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(head, align1);
    return nk_label(ui_ctx, text, align2), 0;
}
int ui_const_bool(const char *text, const double value) {
    bool b = !!value;
    return ui_bool(text, &b), 0;
}
int ui_const_float(const char *text, const double value) {
    float f = (float)value;
    return ui_float(text, &f), 0;
}
int ui_const_string(const char *label, const char *text) {
    return ui_label2(label, text);
}

int ui_toggle(const char *label, bool *value) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    // nk_label(ui_ctx, label, alignment);
    int rc = nk_button_transparent(ui_ctx, *value ? ICON_MD_TOGGLE_ON : ICON_MD_TOGGLE_OFF);
    return rc ? (*value ^= 1), rc : rc;
}

int ui_color4f(const char *label, float *color4) {
    float c[4] = { color4[0]*255, color4[1]*255, color4[2]*255, color4[3]*255 };
    int ret = ui_color4(label, c);
    for( int i = 0; i < 4; ++i ) color4[i] = c[i] / 255.0f;
    return ret;
}

int ui_color4(const char *label, float *color4) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    struct nk_colorf after = { color4[0]/255, color4[1]/255, color4[2]/255, color4[3]/255 }, before = after;
    if (nk_combo_begin_color(ui_ctx, nk_rgb_cf(after), nk_vec2(200,400))) {
        enum color_mode {COL_RGB, COL_HSV};
        static int col_mode = COL_RGB;
        nk_layout_row_dynamic(ui_ctx, 120, 1);
        after = nk_color_picker(ui_ctx, after, NK_RGBA);

        nk_layout_row_dynamic(ui_ctx, 25, 2);
        col_mode = nk_option_label(ui_ctx, "RGB", col_mode == COL_RGB) ? COL_RGB : col_mode;
        col_mode = nk_option_label(ui_ctx, "HSV", col_mode == COL_HSV) ? COL_HSV : col_mode;

        nk_layout_row_dynamic(ui_ctx, 25, 1);
        if (col_mode == COL_RGB) {
            after.r = nk_propertyf(ui_ctx, "#R:", 0, after.r, 1.0f, 0.01f,0.005f);
            after.g = nk_propertyf(ui_ctx, "#G:", 0, after.g, 1.0f, 0.01f,0.005f);
            after.b = nk_propertyf(ui_ctx, "#B:", 0, after.b, 1.0f, 0.01f,0.005f);
            after.a = nk_propertyf(ui_ctx, "#A:", 0, after.a, 1.0f, 0.01f,0.005f);
        } else {
            float hsva[4];
            nk_colorf_hsva_fv(hsva, after);
            hsva[0] = nk_propertyf(ui_ctx, "#H:", 0, hsva[0], 1.0f, 0.01f,0.05f);
            hsva[1] = nk_propertyf(ui_ctx, "#S:", 0, hsva[1], 1.0f, 0.01f,0.05f);
            hsva[2] = nk_propertyf(ui_ctx, "#V:", 0, hsva[2], 1.0f, 0.01f,0.05f);
            hsva[3] = nk_propertyf(ui_ctx, "#A:", 0, hsva[3], 1.0f, 0.01f,0.05f);
            after = nk_hsva_colorfv(hsva);
        }

        color4[0] = after.r * 255;
        color4[1] = after.g * 255;
        color4[2] = after.b * 255;
        color4[3] = after.a * 255;

        nk_combo_end(ui_ctx);
    }
    return !!memcmp(&before.r, &after.r, sizeof(struct nk_colorf));
}

int ui_color3f(const char *label, float *color3) {
    float c[3] = { color3[0]*255, color3[1]*255, color3[2]*255 };
    int ret = ui_color3(label, c);
    for( int i = 0; i < 3; ++i ) color3[i] = c[i] / 255.0f;
    return ret;
}

int ui_color3(const char *label, float *color3) {
    float color4[] = { color3[0], color3[1], color3[2], 255 };
    int ret = ui_color4(label, color4);
    color3[0] = color4[0];
    color3[1] = color4[1];
    color3[2] = color4[2];
    return ret;
}

int ui_list(const char *label, const char **items, int num_items, int *selector) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    int val = nk_combo(ui_ctx, items, num_items, *selector, 25, nk_vec2(200,200));
    int chg = val != *selector;
    *selector = val;
    return chg;
}

int ui_slider(const char *label, float *slider) {
    // return ui_slider2(label, slider, va("%.2f ", *slider));
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    nk_size val = *slider * 1000;
    int chg = nk_progress(ui_ctx, &val, 1000, NK_MODIFIABLE);
    *slider = val / 1000.f;
    return chg;
}
int ui_slider2(const char *label, float *slider, const char *caption) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    struct nk_window *win = ui_ctx->current;
    const struct nk_style *style = &ui_ctx->style;
    struct nk_rect bounds; nk_layout_peek(&bounds, ui_ctx); bounds.w -= 10; // bounds.w *= 0.95f;
    struct nk_vec2 item_padding = style->text.padding;
    struct nk_text text;
    text.padding.x = item_padding.x;
    text.padding.y = item_padding.y;
    text.background = style->window.background;
    text.text = nk_rgb(255,255,255);

        nk_size val = *slider * 1000;
        int chg = nk_progress(ui_ctx, &val, 1000, NK_MODIFIABLE);
        *slider = val / 1000.f;

    nk_widget_text(&win->buffer, bounds, caption, strlen(caption), &text, NK_TEXT_RIGHT, style->font);
    return chg;
}

int ui_bool(const char *label, bool *enabled ) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    int val = *enabled;
#if 0
    int chg = !!nk_checkbox_label(ui_ctx, "", &val);
#else
    int chg = !!nk_button_transparent(ui_ctx, val ? ICON_MD_CHECK_BOX : ICON_MD_CHECK_BOX_OUTLINE_BLANK );
#endif
    *enabled ^= chg;
    return chg;
}

int ui_int(const char *label, int *v) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    int prev = *v;
    *v = nk_propertyi(ui_ctx, "#", INT_MIN, *v, INT_MAX, 1,1);
    return prev != *v;
}

int ui_short(const char *label, short *v) {
    int i = *v, ret = ui_int( label, &i );
    return *v = (short)i, ret;
}

int ui_float(const char *label, float *v) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    float prev = v[0]; v[0] = nk_propertyf(ui_ctx, "#", -FLT_MAX, v[0], FLT_MAX, 0.01f,0.005f);
    return prev != v[0];
}

int ui_clampf(const char *label, float *v, float minf, float maxf) {
    if( minf > maxf ) return ui_clampf(label, v, maxf, minf);

    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    float prev = v[0]; v[0] = nk_propertyf(ui_ctx, "#", minf, v[0], maxf, 0.1f,0.05f);
    return prev != v[0];
}

int ui_float2(const char *label, float *v) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    char *buffer = va("%.2f, %.2f", v[0], v[1]);
    if (nk_combo_begin_label(ui_ctx, buffer, nk_vec2(200,200))) {
        nk_layout_row_dynamic(ui_ctx, 25, 1);
        float prev0 = v[0]; nk_property_float(ui_ctx, "#X:", -FLT_MAX, &v[0], FLT_MAX, 1,0.5f);
        float prev1 = v[1]; nk_property_float(ui_ctx, "#Y:", -FLT_MAX, &v[1], FLT_MAX, 1,0.5f);
        nk_combo_end(ui_ctx);
        return prev0 != v[0] || prev1 != v[1];
    }
    return 0;
}

int ui_float3(const char *label, float *v) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    char *buffer = va("%.2f, %.2f, %.2f", v[0], v[1], v[2]);
    if (nk_combo_begin_label(ui_ctx, buffer, nk_vec2(200,200))) {
        nk_layout_row_dynamic(ui_ctx, 25, 1);
        float prev0 = v[0]; nk_property_float(ui_ctx, "#X:", -FLT_MAX, &v[0], FLT_MAX, 1,0.5f);
        float prev1 = v[1]; nk_property_float(ui_ctx, "#Y:", -FLT_MAX, &v[1], FLT_MAX, 1,0.5f);
        float prev2 = v[2]; nk_property_float(ui_ctx, "#Z:", -FLT_MAX, &v[2], FLT_MAX, 1,0.5f);
        nk_combo_end(ui_ctx);
        return prev0 != v[0] || prev1 != v[1] || prev2 != v[2];
    }
    return 0;
}

int ui_float4(const char *label, float *v) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    char *buffer = va("%.2f, %.2f, %.2f, %.2f", v[0], v[1], v[2], v[3]);
    if (nk_combo_begin_label(ui_ctx, buffer, nk_vec2(200,200))) {
        nk_layout_row_dynamic(ui_ctx, 25, 1);
        float prev0 = v[0]; nk_property_float(ui_ctx, "#X:", -FLT_MAX, &v[0], FLT_MAX, 1,0.5f);
        float prev1 = v[1]; nk_property_float(ui_ctx, "#Y:", -FLT_MAX, &v[1], FLT_MAX, 1,0.5f);
        float prev2 = v[2]; nk_property_float(ui_ctx, "#Z:", -FLT_MAX, &v[2], FLT_MAX, 1,0.5f);
        float prev3 = v[3]; nk_property_float(ui_ctx, "#W:", -FLT_MAX, &v[3], FLT_MAX, 1,0.5f);
        nk_combo_end(ui_ctx);
        return prev0 != v[0] || prev1 != v[1] || prev2 != v[2] || prev3 != v[3];
    }
    return 0;
}

int ui_string(const char *label, char *buffer, int buflen) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    int active = nk_edit_string_zero_terminated(ui_ctx, NK_EDIT_AUTO_SELECT|NK_EDIT_CLIPBOARD|NK_EDIT_FIELD/*NK_EDIT_BOX*/|NK_EDIT_SIG_ENTER, buffer, buflen, nk_filter_default);
    return !!(active & NK_EDIT_COMMITED);
}

int ui_separator() {
    nk_layout_row_dynamic(ui_ctx, 10, 1);

    // cycle color (phi ratio)
    //ui_hue = (ui_hue+0.61803f)*1.61803f; while(ui_hue > 1) ui_hue -= 1;
    ui_hue *= 1.61803f / 1.85f; while(ui_hue > 1) ui_hue -= 1;

    struct nk_command_buffer *canvas;
    struct nk_input *input = &ui_ctx->input;
    canvas = nk_window_get_canvas(ui_ctx);
    struct nk_rect space;
    enum nk_widget_layout_states state;
    state = nk_widget(&space, ui_ctx);
    if (state) nk_fill_rect(canvas, space, 0, ui_ctx->style.window.header.normal.data.color );

    return 1;
}

int ui_image(const char *label, handle id, unsigned w, unsigned h) {
    //nk_layout_row_dynamic(ui_ctx, 1, 1);
    //struct nk_rect bounds = nk_widget_bounds(ui_ctx);

    nk_layout_row_dynamic(ui_ctx, h < 30 || id == texture_checker().id ? 0 : h, 2);
    ui_label_(label, NK_TEXT_LEFT);

    int ret = nk_button_image(ui_ctx, nk_image_id((int)id));
    return !!ret;
}

int ui_colormap( const char *map_name, colormap_t *cm ) {
    int ret = 0;
    if( !cm->texture ) {
        const char *title = va("%s (no image)", map_name);
        if( ui_image( title, texture_checker().id, 0,0 ) ) {
            ret = 2;
        }
    } else {
        unsigned w = cm->texture->w, h = cm->texture->h;
        ui_label(va("%s (%s)", map_name, cm->texture->filename) ); // @fixme: labelf would crash?

        const char *fmt[] = { "", "R", "RG", "RGB", "RGBA" };
        const char *title = va("%s %dx%d %s", map_name, w, h, fmt[cm->texture->n]);
        if( ui_image( title, cm->texture->id, 0, 128 ) ) {
            ret = 2;
        }
    }

    if( ui_color4f( va("%s Color", map_name), (float *) &cm->color ) ) {
        ret = 1;
    }
    return ret;
}

int ui_radio(const char *label, const char **items, int num_items, int *selector) {
    int ret = 0;
    if( label && label[0] ) ui_label(label);
    for( int i = 0; i < num_items; i++ ) {
        bool enabled = *selector == i;
        if( ui_bool( va("%s%s", label && label[0] ? "  ":"", items[i]), &enabled ) ) {
            *selector = i;
            ret |= 1;
        }
    }
    return ret;
}

int ui_section(const char *section) {
    ui_separator();
    return ui_label(va("*%s", section));
}

int ui_dialog(const char *title, const char *text, int choices, bool *show) { // @fixme: return
    ui_has_active_popups = 0;
    if(*show) {
        static struct nk_rect s = {0, 0, 300, 190};
        if (nk_popup_begin(ui_ctx, NK_POPUP_STATIC, title, NK_WINDOW_BORDER|NK_WINDOW_CLOSABLE, s)) {
            nk_layout_row_dynamic(ui_ctx, 20, 1);
            for( char label[1024]; *text && sscanf(text, "%[^\r\n]", label); ) {
                nk_label(ui_ctx, label, NK_TEXT_LEFT);
                text += strlen(label); while(*text && *text < 32) text++;
            }

            if( choices ) {
                nk_layout_row_dynamic(ui_ctx, 25, choices > 1 ? 2 : 1);
                if (nk_button_label(ui_ctx, "OK")) {
                    *show = 0;
                    nk_popup_close(ui_ctx);
                }
                if (choices > 1 && nk_button_label(ui_ctx, "Cancel")) {
                    *show = 0;
                    nk_popup_close(ui_ctx);
                }
            }

            nk_popup_end(ui_ctx);
        } else {
            *show = nk_false;
        }
        ui_has_active_popups = *show;
    }
    return *show;
}

#define ui_bits_template(X) \
int ui_bits##X(const char *label, uint##X##_t *enabled) { \
    /* @fixme: better way to retrieve widget width? nk_layout_row_dynamic() seems excessive */ \
    nk_layout_row_dynamic(ui_ctx, 1, 1); \
    struct nk_rect bounds = nk_widget_bounds(ui_ctx); \
\
    /* actual widget: label + X checkboxes */ \
    nk_layout_row_begin(ui_ctx, NK_STATIC, 0, 1+X); \
\
        int offset = bounds.w > (15*X) ? bounds.w - (15*X) : 0; /* bits widget below needs at least 118px wide */ \
        nk_layout_row_push(ui_ctx, offset); \
        ui_label_(label, NK_TEXT_LEFT); \
\
        uint8_t copy = *enabled; \
        for( int i = 0; i < X; ++i ) { \
            int b = (X-1-i); \
            nk_layout_row_push(ui_ctx, 10); \
            /* bit */ \
            int val = (*enabled >> b) & 1; \
            int chg = nk_checkbox_label(ui_ctx, "", &val); \
            *enabled = (*enabled & ~(1 << b)) | ((!!val) << b); \
            /* tooltip */ \
            struct nk_rect bb = { offset + 10 + i * 14, bounds.y, 14, 30 }; /* 10:padding,14:width,30:height */ \
            if (nk_input_is_mouse_hovering_rect(&ui_ctx->input, bb) && !ui_has_active_popups && nk_window_has_focus(ui_ctx)) { \
                nk_tooltipf(ui_ctx, "Bit %d", b); \
            } \
        } \
\
    nk_layout_row_end(ui_ctx); \
    return copy ^ *enabled; \
}

ui_bits_template(8);
ui_bits_template(16);
//ui_bits_template(32);

int ui_console() { // @fixme: buggy. also separate input from output buffer?
    struct nk_font *font = nk_glfw.atlas.fonts->next /*2nd font*/;
    if( font && nk_style_push_font(ui_ctx, &font->handle) ) {} else font = 0;

    static int box_len = 0;
    static char box_buffer[512] = {0};

    // @fixme: better way to retrieve widget width? nk_layout_row_dynamic() seems excessive
    nk_layout_row_dynamic(ui_ctx, 1, 1);
    struct nk_rect bounds = nk_widget_bounds(ui_ctx);

    nk_layout_row_static(ui_ctx, 360, bounds.w, 1);
    int flags = NK_EDIT_BOX|NK_EDIT_GOTO_END_ON_ACTIVATE|NK_EDIT_CLIPBOARD; // |NK_EDIT_SIG_ENTER|NK_EDIT_AUTO_SELECT|
    nk_edit_focus(ui_ctx, flags);
    int active = nk_edit_string(ui_ctx, flags, box_buffer, &box_len, 512-1, nk_filter_default);
    if (active && input_down(KEY_ENTER)) { // == 0x11) { // & NK_EDIT_COMMITED) {

//              hexdump(box_buffer, box_len);

        box_buffer[box_len] = '\0';
        array(char*) lines = strsplit(box_buffer, "\n"); // @fixme: new strsplit2 should not include terminator imo?

        if( array_count(lines) > 0 ) {
            int first_line = 0;
            char *last_line = *array_back(lines);
#if 0
            os_exec(last_line);
            const char *output = os_exec_output();
            box_len += snprintf(box_buffer + box_len, (512-1) - box_len, "%s\n", output);
#else
            // puts(last_line);
            for( FILE *fp = popen(last_line, "r"); fp; pclose(fp), fp = 0) {
                char buffer[511];
                while( fgets(buffer, 511, fp) ) {
                    int len = 0; while(buffer[len] != '\r' && buffer[len] != '\n') { if(buffer[len] < 0) buffer[len] = '?'; len++; }

                    while( (box_len+len) >= 512-1 && array_count(lines) ) {
                        int first_line_len = strlen(lines[0]);
                        first_line_len += box_buffer[first_line_len] == '\n';
                        memmove(box_buffer, box_buffer + first_line_len, box_len -= first_line_len);
                        //box_buffer[box_len] = '\0';
                        lines = strsplit(box_buffer, "\n");
                    }

                    if((box_len+len) > 512-1)
                        hexdump(box_buffer, 511);

                    ASSERT( (box_len+len) <= 512-1, "box_len %d+%d %d", box_len, len, array_count(lines));
                    box_len += snprintf(box_buffer + box_len, (512-1) - box_len, "%.*s\n", len, buffer);
                }
            }
#endif
        }
//              hexdump(box_buffer, box_len);
    } else {
        active = 0;
    }

    if( font )  nk_style_pop_font(ui_ctx);
    return active;
}

int ui_browse(const char **output, bool *inlined) {
    static struct browser_media media = {0};
    static struct browser browser = {0};
    do_once {
        const int W = 96, H = 96; // 2048x481 px, 21x5 cells
        texture_t i = texture("icons/suru.png", TEXTURE_RGBA|TEXTURE_MIPMAPS);
        browser_config_dir(icon_load_rect(i.id, i.w, i.h, W, H, 16, 3), BROWSER_FOLDER); // default group
        browser_config_dir(icon_load_rect(i.id, i.w, i.h, W, H,  2, 4), BROWSER_HOME); 
        browser_config_dir(icon_load_rect(i.id, i.w, i.h, W, H, 17, 3), BROWSER_COMPUTER);
        browser_config_dir(icon_load_rect(i.id, i.w, i.h, W, H,  1, 4), BROWSER_PROJECT);
        browser_config_dir(icon_load_rect(i.id, i.w, i.h, W, H,  0, 4), BROWSER_DESKTOP);

        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  8, 0), "");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H, 10, 2), ".txt.md.license" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  8, 1), ".ini" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  8, 3), ".xlsx" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  9, 0), ".c" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  4, 1), ".h.hpp.hh" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  4, 2), ".fs.vs.gs.fx.glsl.shader" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H, 12, 0), ".cpp.cc" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H, 15, 0), ".json" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H, 15, 2), ".bat.sh" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  6, 1), ".htm.html" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H, 20, 1), ".xml" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H, 12, 1), ".js" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  0, 3), ".ts" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  6, 2), ".py" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H, 16, 1), ".lua" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H, 10, 0), ".css.doc" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  6, 0), ".wav.flac.ogg.mp1.mp3.mod.xm.s3m.it.sfxr.mid" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  1, 3), ".ttf.ttc.otf" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  7, 1), ".jpg.jpeg.png.bmp.psd.pic.pnm.ico.ktx.pvr.dds.astc.basis.hdr.tga.gif" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  4, 3), ".mp4.mpg.ogv.mkv.wmv.avi" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  2, 1), ".iqm.iqe.gltf.gltf2.glb.fbx.obj.dae.blend.md3.md5.ms3d.smd.x.3ds.bvh.dxf.lwo" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H,  7, 0), ".exe.bin.dSYM.pdb.o.lib.dll" ".");
        browser_config_type(icon_load_rect(i.id, i.w, i.h, W, H, 15, 3), ".zip.rar.7z.pak" ".");

        browser_init(&browser);
    }
    // at_exit: browser_free(&browser);

    int clicked = 0;
    bool windowed = !inlined;
    if( windowed || (!windowed && *inlined) ) {

        struct nk_rect bounds = {0,0,400,300}; // @fixme: how to retrieve inlined region below? (inlined)
        if( windowed ) bounds = nk_window_get_content_region(ui_ctx);
        else { struct nk_rect b; nk_layout_peek(&b, ui_ctx); bounds.w = b.w; }

        clicked = browser_run(ui_ctx, &browser, windowed, bounds);
        if( clicked ) {
            static char *ui_browse_result = 0;
            strcatf(&ui_browse_result, "%d", 0);
            ui_browse_result[0] = '\0';
            strcatf(&ui_browse_result, "%s", browser.file);
            if( inlined ) *inlined = 0;

            const char *target = ifdef(win32, "/", "\\"), *replace = ifdef(win32, "\\", "/");
            strswap(ui_browse_result, target, replace);

            if( output ) *output = ui_browse_result;
        }
    }

    return clicked;
}

/*
//  demo:
    static const char *file;
    if( ui_panel("inlined", 0)) {
        static bool show_browser = 0;
        if( ui_button("my button") ) { show_browser = true; }
        if( ui_browse(&file, &show_browser) ) puts(file);
        ui_panel_end();
    }
    if( ui_window("windowed", 0) ) {
        if( ui_browse(&file, NULL) ) puts(file);
        ui_window_end();
    }
*/

// ----------------------------------------------------------------------------

int ui_demo() {
    static int integer = 42;
    static bool toggle = true;
    static bool boolean = true;
    static float floating = 3.14159;
    static float float2[2] = {1,2};
    static float float3[3] = {1,2,3};
    static float float4[4] = {1,2,3,4};
    static float rgb[3] = {0.84,0.67,0.17};
    static float rgba[4] = {0.67,0.90,0.12,1};
    static float slider = 0.5f;
    static float slider2 = 0.5f;
    static char string[64] = "hello world 123";
    static int item = 0; const char *list[] = {"one","two","three"};
    static bool show_dialog = false;
    static bool show_browser = false;
    static const char* browsed_file = "";
    static uint8_t bitmask = 0x55;
    static int hits = 0;

    if( ui_panel("UI", 0) ) {

        switch( ui_toolbar("Browser;Toast@Notify") ) {
            default: break;
            case 1: show_browser = true; break;
            case 2: ui_notify(va("My random toast (%d)", rand()), va("This is notification #%d", ++hits));
        }
        if( ui_browse(&browsed_file, &show_browser) ) puts(browsed_file);

        if( ui_section("Labels")) {}
        if( ui_label("my label")) {}
        if( ui_label("my label with tooltip@built on " __DATE__ " " __TIME__)) {}

        if( ui_section("Types")) {}
        if( ui_bool("my bool", &boolean) ) puts("bool changed");
        if( ui_int("my int", &integer) ) puts("int changed");
        if( ui_float("my float", &floating) ) puts("float changed");
        if( ui_string("my string", string, 64) ) puts("string changed");

        if( ui_section("Vectors") ) {}
        if( ui_float2("my float2", float2) ) puts("float2 changed");
        if( ui_float3("my float3", float3) ) puts("float3 changed");
        if( ui_float4("my float4", float4) ) puts("float4 changed");

        if( ui_section("Lists")) {}
        if( ui_list("my list", list, 3, &item ) ) puts("list changed");

        if( ui_section("Colors")) {}
        if( ui_color3f("my color3", rgb) ) puts("color3 changed");
        if( ui_color4f("my color4@this is a tooltip", rgba) ) puts("color4 changed");

        if( ui_section("Sliders")) {}
        if( ui_slider("my slider", &slider)) puts("slider changed");
        if( ui_slider2("my slider 2", &slider2, va("%.2f", slider2))) puts("slider2 changed");

        if( ui_section("Others")) {}
        if( ui_bits8("my bitmask", &bitmask) ) printf("bitmask changed %x\n", bitmask);
        if( ui_toggle("my toggle", &toggle) ) printf("toggle %s\n", toggle ? "on":"off");
        if( ui_image("my image", texture_checker().id, 0, 0) ) { puts("image clicked"); }
        if( ui_separator() ) {}
        if( ui_buttons(3, "yes", "no", "maybe") ) { puts("button clicked"); }
        if( ui_buttons(2, "yes", "no") ) { puts("button clicked"); }
        if( ui_button("my button") ) { puts("button clicked"); show_dialog = true; }
        if( ui_dialog("my dialog", __FILE__ "\n" __DATE__ "\n" "Public Domain.", 2/*two buttons*/, &show_dialog) ) {}

        ui_panel_end();
    }

    // window api showcasing

    if( ui_window("UI Demo #1", 0) ) {
        ui_label("label #1");
        ui_window_end();
    }

    if( ui_window("UI Demo #2", 0) ) {
        if( ui_panel("panel #2", 0) ) {
            ui_label("label #2");
            ui_panel_end();
        }
        ui_window_end();
    }

    if( ui_window("UI Demo #3", 0) ) {
        if( ui_panel("panel #3a", 0) ) {
            if( ui_bool("my bool", &boolean) ) puts("bool changed");
            if( ui_int("my int", &integer) ) puts("int changed");
            if( ui_float("my float", &floating) ) puts("float changed");
            if( ui_string("my string", string, 64) ) puts("string changed");
            if( ui_separator() ) {}
            if( ui_slider("my slider", &slider)) puts("slider changed");
            if( ui_slider2("my slider 2", &slider2, va("%.2f", slider2))) puts("slider2 changed");
            ui_panel_end();
        }
        if( ui_panel("panel #3b", 0) ) {
            if( ui_bool("my bool", &boolean) ) puts("bool changed");
            if( ui_int("my int", &integer) ) puts("int changed");
            if( ui_float("my float", &floating) ) puts("float changed");
            if( ui_string("my string", string, 64) ) puts("string changed");
            if( ui_separator() ) {}
            if( ui_slider("my slider", &slider)) puts("slider changed");
            if( ui_slider2("my slider 2", &slider2, va("%.2f", slider2))) puts("slider2 changed");
            ui_panel_end();
        }

        const char *title = "UI Demo #3";
        struct nk_window *win = nk_window_find(ui_ctx, title);
        if( win ) {
            enum { menubar_height = 65 }; // title bar (~32) + menu bounds (~25)
            struct nk_rect bounds = win->bounds; bounds.y += menubar_height; bounds.h -= menubar_height; 
#if 0
            ddraw_flush();

            // @fixme: this is breaking rendering when post-fxs are in use
            static texture_t tx = {0};
            if( texture_rec_begin(&tx, bounds.w, bounds.h )) {
                glClearColor(0.15,0.15,0.15,1);
                glClear(GL_COLOR_BUFFER_BIT);
                ddraw_grid(10);
                ddraw_flush();
                texture_rec_end(&tx);
            }
            struct nk_image image = nk_image_id((int)tx.id);
            nk_draw_image_flipped(nk_window_get_canvas(ui_ctx), bounds, &image, nk_white);
#else
            static video_t *v = NULL;
            do_once v = video( "bjork-all-is-full-of-love.mp4", VIDEO_RGB );

            texture_t *textures = video_decode( v );

            struct nk_image image = nk_image_id((int)textures[0].id);
            nk_draw_image(nk_window_get_canvas(ui_ctx), bounds, &image, nk_white);
#endif
        }

        ui_window_end();
    }
    return 0;
}
