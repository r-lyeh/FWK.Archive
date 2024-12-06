#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024

static struct nk_context *ui_ctx;
static struct nk_glfw ui_glfw = {0};

void ui_destroy(void) {
    if(ui_ctx) {
        nk_glfw3_shutdown(&ui_glfw); // nk_sdl_shutdown();
        ui_ctx = 0;
    }
}

void ui_create() {
    if(!ui_ctx) {
        ui_ctx = nk_glfw3_init(&ui_glfw, window_handle(), NK_GLFW3_INSTALL_CALLBACKS); // nk_sdl_init(window_handle());
        atexit(ui_destroy);

        /* Load Fonts: if none of these are loaded a default font will be used  */
        /* Load Cursor: if you uncomment cursor loading please hide the cursor */
        //{struct nk_font_atlas *atlas;

#define ICON_FONTNAME "fontawesome-webfont.ttf"
#define ICON_FONTSIZE 15
#define ICON_MIN 0xF000
#define ICON_MAX 0xF2E0
#define ICON_FILE_O "\xef\x80\x96" // U+f016
#define ICON_BARS "\xef\x83\x89" // U+f0c9
#define ICON_TEST_GLYPH ICON_FILE_O

        {struct nk_font_atlas *atlas;
        struct nk_font_config cfg = nk_font_config(ICON_FONTSIZE+1);
        static const nk_rune icon_range[] = {ICON_MIN, ICON_MAX, 0};
        cfg.range = icon_range;
        cfg.merge_mode = 1;

cfg.spacing.x += 8.f;
//    cfg.spacing.y += 0.f;
//    cfg.font->ascent ++;
//    cfg.font->height += 4;

        struct nk_font *last = NULL;
        nk_glfw3_font_stash_begin(&ui_glfw, &atlas); // nk_sdl_font_stash_begin(&atlas);
#if 0 // is(win32)
        struct nk_font *arial = nk_font_atlas_add_from_file(atlas, va("%s/fonts/arial.ttf",getenv("windir")), 15.f, 0); last = arial ? arial : last;
#else
        struct nk_font *roboto = nk_font_atlas_add_from_file(atlas, vfs_find("Carlito-Regular.ttf"), 14.5f, 0); last = roboto ? roboto : last;
#endif
        //struct nk_font *droid = nk_font_atlas_add_from_file(atlas, "nuklear/extra_font/DroidSans.ttf", 14, 0); last = droid ? droid : last;
        struct nk_font *icons = nk_font_atlas_add_from_file(atlas, vfs_find(ICON_FONTNAME), ICON_FONTSIZE, &cfg);
        nk_glfw3_font_stash_end(&ui_glfw); // nk_sdl_font_stash_end();
        /* nk_style_load_all_cursors(ctx, atlas->cursors); glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN); */
        if(last) nk_style_set_font(ui_ctx, &last->handle);}

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
        // table[NK_COLOR_CHART] = nk_rgba(50, 58, 61, 255);
        // table[NK_COLOR_CHART_COLOR] = main_color;
        // table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(255, 0, 0, 255);
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

        // enum { MAX_MEMORY = 2 * 1024 * 1024 };
        // nk_init_fixed(&ui_ctx, calloc(1, MAX_MEMORY), MAX_MEMORY, &font);
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

static
int ui_menu_active() {
    return !!ui_items; // array_count(ui_items) > 0;
}

static
void ui_separator_line() {
    struct nk_rect space; nk_layout_peek(&space, ui_ctx); // bounds.w *= 0.95f;
    struct nk_command_buffer *canvas = nk_window_get_canvas(ui_ctx);
    nk_stroke_line(canvas, space.x+0,space.y+0,space.x+space.w,space.y+0, 3.0, nk_rgb(128,128,128));
}

static
void ui_menu_render() {
    // clean up from past frame
    ui_results = vec2(0,0);
    if( !ui_items ) return;

    // process menus
    if( nk_begin(ui_ctx, "Menu", nk_rect(0, 0, window_width(), 32), NK_WINDOW_NO_SCROLLBAR/*|NK_WINDOW_BACKGROUND*/)) {
        nk_menubar_begin(ui_ctx);
        nk_layout_row_begin(ui_ctx, NK_STATIC, 25, array_count(ui_items));

        for( int i = 0, end = array_count(ui_items); i < end; ++i ) {
            char id[64]; sscanf(ui_items[i], "%[^,;|]", id);

            nk_layout_row_push(ui_ctx, 45);
            if (nk_menu_begin_label(ui_ctx, id, NK_TEXT_LEFT, nk_vec2(120, 200))) {
                nk_layout_row_dynamic(ui_ctx, 32, 1);

                int j = 0;
                for each_substring(ui_items[i] + strlen(id) + 1, ",;|", item) {
                    if( *item == '-' ) {
                        while(*item == '-') ++item;
                        //nk_menu_item_label(ui_ctx, "---", NK_TEXT_LEFT);
                        ui_separator_line();
                    }

                    if( nk_menu_item_label(ui_ctx, item, NK_TEXT_LEFT) ) {
                        ui_results = vec2(i+1, j+1);
                    }
                    ++j;
                }
                nk_menu_end(ui_ctx);
            }
        }

        nk_layout_row_end(ui_ctx);
        nk_menubar_end(ui_ctx);
    }
    nk_end(ui_ctx);

    // clean up for next frame
    for( int i = 0; i < array_count(ui_items); ++i ) {
        FREE(ui_items[i]);
    }
    array_resize(ui_items, 0);
}

// -----------------------------------------------------------------------------

static int ui_dirty = 1;
static int ui_popups_active = 0;
static float ui_hue = 0; // hue
static int ui_hovering = 0;
static int ui_actived = 0;

int ui_hover() {
    return ui_hovering;
}
int ui_active() {
    return ui_actived; //window_has_cursor() && nk_window_is_any_hovered(ui_ctx) && nk_item_is_any_active(ui_ctx);
}

void ui_render() {
    // draw queued menus
    ui_menu_render();

    /* IMPORTANT: `nk_sdl_render` modifies some global OpenGL state
     * with blending, scissor, face culling, depth test and viewport and
     * defaults everything back into a default state.
     * Make sure to either a.) save and restore or b.) reset your own state after
     * rendering the UI. */
    //nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
    nk_glfw3_render(&ui_glfw, NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
    ui_dirty = 1;
    ui_hue = 0;

    ui_hovering = nk_window_is_any_hovered(ui_ctx) && window_has_cursor();

    if(input_down(MOUSE_L))
        ui_actived = (ui_hovering && nk_item_is_any_active(ui_ctx));
    if(input_up(MOUSE_L))
        ui_actived = 0;
}


// ----------------------------------------------------------------------------

int ui_begin(const char *title, int flags) {
    if( window_width() <= 0 ) return 0;
    if( window_height() <= 0 ) return 0;

    ui_create();
    if(ui_dirty) {
        nk_glfw3_new_frame(&ui_glfw);
        ui_dirty = 0;
    }

    uint64_t hash = 14695981039346656037ULL, mult = 0x100000001b3ULL;
    for(int i = 0; title[i]; ++i) hash = (hash ^ title[i]) * mult;
    ui_hue = (hash & 0x3F) / (float)0x3F; ui_hue += !ui_hue;

#if 0
    static int rows = 0;
    static hashmap(const char*,int) map, *init = 0;
    if( !init ) { hashmap_create(init = &map, 128 /*maxwindows*/, str64); }
    int *found = hashmap_find(&map, title);
    if( !found ) hashmap_insert(&map, title, ++rows);
    int row = found ? *found : rows;
#else // fixme
    static const char *list[128] = {0};
    static int         posx[128] = {0};
    int row = -1, find = 0; while( find < 128 && list[find] ) {
        if( !strcmp(title, list[find]) ) { row = find; break; } else ++find;
    }
    if( row < 0 ) list[row = find] = STRDUP(title);
    if( ui_menu_active() ) ++row; // add 1 to skip menu
#endif

    int window_flags = nk_window_is_active(ui_ctx, title) ? 0 : NK_WINDOW_MINIMIZED; //NK_MINIMIZED;
    window_flags |= NK_WINDOW_BORDER;
    window_flags |= NK_WINDOW_SCALABLE;
    window_flags |= NK_WINDOW_MOVABLE;
    //window_flags |= NK_WINDOW_NO_SCROLLBAR;
    //window_flags |= NK_WINDOW_SCALE_LEFT;
    window_flags |= NK_WINDOW_MINIMIZABLE;

//  struct nk_style *s = &ui_ctx->style;
//  nk_style_push_color(ui_ctx, &s->window.header.normal.data.color, nk_hsv_f(ui_hue,0.6,0.8));
if( posx[row] > 0 ) window_flags &= ~NK_WINDOW_MINIMIZED;

    int ui_open;
    vec2 offset = vec2(0, 32*row);
    float w = window_width() / 3.33, h = window_height() - offset.y * 2;
    if( nk_begin(ui_ctx, title, nk_rect(offset.x, offset.y, offset.x+w, offset.y+h), window_flags) ) {

posx[row] = nk_window_get_position(ui_ctx).x;

        ui_open = 1;
    } else {
        ui_end();
        ui_open = 0;
    }

    return ui_open;
}

void ui_end() {
    nk_end(ui_ctx);
//  nk_style_pop_color(ui_ctx);
}


int ui_button(const char *s) {
    nk_layout_row_dynamic(ui_ctx, 0, 1);

    struct nk_rect bounds = nk_widget_bounds(ui_ctx);

    const char *split = strchr(s, '@'), *tooltip = split + 1;
    int ret = split ? nk_button_text(ui_ctx, s, (int)(split - s)) : nk_button_label(ui_ctx, s);

    const struct nk_input *in = &ui_ctx->input;
    if (split && nk_input_is_mouse_hovering_rect(in, bounds) && !ui_popups_active) {
        nk_tooltip(ui_ctx, tooltip);
    }
    return ret;
}

int ui_label_(const char *label, int alignment) {
    struct nk_rect bounds = nk_widget_bounds(ui_ctx);
    const struct nk_input *in = &ui_ctx->input;
    int is_hovering = nk_input_is_mouse_hovering_rect(in, bounds);

    int indent = 8;
    struct nk_window *win = ui_ctx->current;
    struct nk_panel *layout = win->layout;
    layout->at_x += indent;
    layout->bounds.w -= indent;
        bounds.w = is_hovering ? indent*3/4 : indent/2-1;
        bounds.h -= 1;
        struct nk_command_buffer *canvas = nk_window_get_canvas(ui_ctx);
        struct nk_input *input = &ui_ctx->input;
        nk_fill_rect(canvas, bounds, 0, nk_hsv_f(ui_hue, 0.6f, 0.8f) );

        const char *split = strchr(label, '@'), *tooltip = split + 1;
            char buffer[128]; if( split ) label = (snprintf(buffer, 128, "%.*s", (int)(split-label), label), buffer);
            nk_label(ui_ctx, label, alignment);
            if (split && is_hovering && !ui_popups_active) {
                nk_tooltip(ui_ctx, split ? tooltip : label);
            }

    layout->at_x -= indent;
    layout->bounds.w += indent;

    return is_hovering ? nk_input_has_mouse_click_down_in_rect(input, NK_BUTTON_LEFT, layout->bounds, nk_true) : 0;
}

int ui_label(const char *label) {
    nk_layout_row_dynamic(ui_ctx, 0, 1);
    return ui_label_(label, NK_TEXT_LEFT);
}
int ui_label2(const char *label, const char *caption) {
    char buffer[2048]; snprintf(buffer, 2048, "%s@%s", label, caption);
    return ui_label(buffer);
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
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);
    return nk_label(ui_ctx, text, NK_TEXT_LEFT), 0;
}



int ui_toggle(const char *label, bool *value) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    struct nk_style_button button = ui_ctx->style.button; if(!*value) {
    ui_ctx->style.button.normal =
    ui_ctx->style.button.hover =
    ui_ctx->style.button.active = nk_style_item_color(nk_rgba(50, 58, 61, 225)); // same as table[NK_COLOR_EDIT]
    ui_ctx->style.button.border_color =
    ui_ctx->style.button.text_background =
    ui_ctx->style.button.text_normal =
    ui_ctx->style.button.text_hover =
    ui_ctx->style.button.text_active = nk_rgb(120,120,120);
    }

    int rc = nk_button_label(ui_ctx, *value ? "on":"off");
    ui_ctx->style.button = button;
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
    int chg = nk_checkbox_label(ui_ctx, "", &val);
    *enabled = !!val;
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

int ui_dialog(const char *title, const char *text, int choices, bool *show) { // @fixme: return
    ui_popups_active = 0;
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
        ui_popups_active = *show;
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
            nk_layout_row_push(ui_ctx, 10); \
            /* bit */ \
            int val = (*enabled >> i) & 1; \
            int chg = nk_checkbox_label(ui_ctx, "", &val); \
            *enabled = (*enabled & ~(1 << i)) | ((!!val) << i); \
            /* tooltip */ \
            struct nk_rect bb = { offset + 10 + i * 14, bounds.y, 14, 30 }; /* 10:padding,14:width,30:height */ \
            if (nk_input_is_mouse_hovering_rect(&ui_ctx->input, bb) && !ui_popups_active) { \
                nk_tooltipf(ui_ctx, "Bit %d", i); \
            } \
        } \
\
    nk_layout_row_end(ui_ctx); \
    return copy ^ *enabled; \
}

ui_bits_template(8);
ui_bits_template(16);
//ui_bits_template(32);

// ----------------------------------------------------------------------------

void ui_demo() {
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
    static uint8_t bitmask = 0x55;

    // if( ui_menu("File;Open;Save;Quit") ) printf("menu option %d\n", ui_item());
    // if( ui_menu("Edit;Cut;Copy;Paste") ) printf("menu option %d\n", ui_item());

    if( ui_begin("UI", 0)) {
        if( ui_label("my label")) {}
        if( ui_label("my label with tooltip@built on " __DATE__ " " __TIME__)) {}
        if( ui_separator() ) {}
        if( ui_bool("my bool", &boolean) ) puts("bool changed");
        if( ui_int("my int", &integer) ) puts("int changed");
        if( ui_float("my float", &floating) ) puts("float changed");
        if( ui_string("my string", string, 64) ) puts("string changed");
        if( ui_separator() ) {}
        if( ui_slider("my slider", &slider)) puts("slider changed");
        if( ui_slider2("my slider 2", &slider2, va("%.2f", slider2))) puts("slider2 changed");
        if( ui_separator() ) {}
        if( ui_list("my list", list, 3, &item ) ) puts("list changed");
        if( ui_separator() ) {}
        if( ui_color3f("my color3", rgb) ) puts("color3 changed");
        if( ui_color4f("my color4@this is a tooltip", rgba) ) puts("color4 changed");
        if( ui_separator() ) {}
        if( ui_float2("my float2", float2) ) puts("float2 changed");
        if( ui_float3("my float3", float3) ) puts("float3 changed");
        if( ui_float4("my float4", float4) ) puts("float4 changed");
        if( ui_bits8("my bitmask", &bitmask) ) puts("bitmask changed");
        if( ui_separator() ) {}
        if( ui_toggle("my toggle", &toggle) ) printf("toggle %s\n", toggle ? "on":"off");
        if( ui_separator() ) {}
        if( ui_image("my image", texture_checker().id, 0, 0) ) { puts("image clicked"); }
        if( ui_button("my button") ) { puts("button clicked"); show_dialog = true; }
        if( ui_dialog("my dialog", __FILE__ "\n" __DATE__ "\n" "Public Domain.", 2/*two buttons*/, &show_dialog) ) {}

        if(0) { // plot
            nk_layout_row_dynamic(ui_ctx, 100, 1);
            static float values[1000] = {0}; static int offset = 0;
            values[offset=(offset+1)%1000] = window_fps();
            nk_plot(ui_ctx, NK_CHART_LINES, values,1000, 0);
        }
        if(0) { // logger/console
            static char box_buffer[512] = {0};
            static int box_len;

            nk_layout_row_dynamic(ui_ctx, 100, 1); // height(100), horiz_slots(1)
            nk_edit_string(ui_ctx, NK_EDIT_BOX/*_EDITOR*/|NK_EDIT_GOTO_END_ON_ACTIVATE/*|NK_EDIT_READ_ONLY*/, box_buffer, &box_len, 512, nk_filter_default);

            ui_separator();

            static char text[64] = "demo";
            static int text_len = 4;

            nk_layout_row_dynamic(ui_ctx, 0, 1); // height(min), horiz_slots(2)
            int active = nk_edit_string(ui_ctx, NK_EDIT_FIELD|NK_EDIT_SIG_ENTER|NK_EDIT_NO_CURSOR, text, &text_len, 64, nk_filter_ascii);
            if (active & NK_EDIT_COMMITED) {
                text[text_len++] = '\n';
                memcpy(&box_buffer[box_len], &text, text_len);
                box_len += text_len;
                text_len = 0;
            }

            if(text_len > 0 && text[0] != '>') { snprintf(text, 64, va(">%s", text)); text_len++; }
        }

        ui_end();
    }
}
