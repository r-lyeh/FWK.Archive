// immediate ui framework
// - rlyeh, public domain
//
// @todo: logger

#ifndef UI_H
#define UI_H

int  ui_begin(const char *title, int flags);
    int  ui_label(const char *label);
    int  ui_labelb(const char *text, double value);
    int  ui_labelf(const char *text, double value);
    int  ui_labels(const char *text, const char *fmt, ...);
    int  ui_button(const char *label);
    int  ui_color3(const char *label, float *color3);
    int  ui_color4(const char *label, float *color4);
    int  ui_slider(const char *label, float *value);
    int  ui_list(const char *label, const char **items, int num_items, int *selector);
    int  ui_int(const char *label, int *v);
    int  ui_bool(const char *label, bool *value);
    int  ui_short(const char *label, short *v);
    int  ui_float(const char *label, float *v);
    int  ui_string(const char *label, char *buffer, int buflen);
    int  ui_separator();
    int  ui_dialog(const char *title, const char *text, int choices, bool *show); // @fixme: return
    int  ui_menu(); // @todo
void ui_end();

void ui_demo();
void ui_render();
// ui_create();
// ui_destroy();

#endif

#ifdef UI_C
#pragma once

enum theme {THEME_RED, THEME_DARK};

static void
set_style(struct nk_context *ctx, enum theme theme)
{
    struct nk_color table[NK_COLOR_COUNT];
    nk_style_default(ctx);
    if (theme == THEME_RED) {
        table[NK_COLOR_TEXT] = nk_rgba(190, 190, 190, 255);
        table[NK_COLOR_WINDOW] = nk_rgba(30, 33, 40, 215);
        table[NK_COLOR_HEADER] = nk_rgba(181, 45, 69, 220);
        table[NK_COLOR_BORDER] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_BUTTON] = nk_rgba(181, 45, 69, 255);
        table[NK_COLOR_BUTTON_HOVER] = nk_rgba(190, 50, 70, 255);
        table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(195, 55, 75, 255);
        table[NK_COLOR_TOGGLE] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(45, 60, 60, 255);
        table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(181, 45, 69, 255);
        table[NK_COLOR_SELECT] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(181, 45, 69, 255);
        table[NK_COLOR_SLIDER] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(181, 45, 69, 255);
        table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(186, 50, 74, 255);
        table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(191, 55, 79, 255);
        table[NK_COLOR_PROPERTY] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_EDIT] = nk_rgba(51, 55, 67, 225);
        table[NK_COLOR_EDIT_CURSOR] = nk_rgba(190, 190, 190, 255);
        table[NK_COLOR_COMBO] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_CHART] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_CHART_COLOR] = nk_rgba(170, 40, 60, 255);
        table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba( 255, 0, 0, 255);
        table[NK_COLOR_SCROLLBAR] = nk_rgba(30, 33, 40, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(64, 84, 95, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(70, 90, 100, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(75, 95, 105, 255);
        table[NK_COLOR_TAB_HEADER] = nk_rgba(181, 45, 69, 220);
        nk_style_from_table(ctx, table);
    } else if (theme == THEME_DARK) {
        table[NK_COLOR_TEXT] = nk_rgba(210, 210, 210, 255);
        table[NK_COLOR_WINDOW] = nk_rgba(57, 67, 71, 215);
        table[NK_COLOR_HEADER] = nk_rgba(51, 51, 56, 220);
        table[NK_COLOR_BORDER] = nk_rgba(46, 46, 46, 255);
        table[NK_COLOR_BUTTON] = nk_rgba(48, 83, 111, 255);
        table[NK_COLOR_BUTTON_HOVER] = nk_rgba(58, 93, 121, 255);
        table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(63, 98, 126, 255);
        table[NK_COLOR_TOGGLE] = nk_rgba(45/1.2, 53/1.2, 56/1.2, 255);
        table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(45*1.2, 53*1.2, 56*1.2, 255);
        table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(48, 83, 111, 255);
        table[NK_COLOR_SELECT] = nk_rgba(57, 67, 61, 255);
        table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(48, 83, 111, 255);
        table[NK_COLOR_SLIDER] = nk_rgba(50, 58, 61, 255);
        table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(48, 83, 111, 245);
        table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(53, 88, 116, 255);
        table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(58, 93, 121, 255);
        table[NK_COLOR_PROPERTY] = nk_rgba(50, 58, 61, 255);
        table[NK_COLOR_EDIT] = nk_rgba(50, 58, 61, 225);
        table[NK_COLOR_EDIT_CURSOR] = nk_rgba(210, 210, 210, 255);
        table[NK_COLOR_COMBO] = nk_rgba(50, 58, 61, 255);
        table[NK_COLOR_CHART] = nk_rgba(50, 58, 61, 255);
        table[NK_COLOR_CHART_COLOR] = nk_rgba(48, 83, 111, 255);
        table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(255, 0, 0, 255);
        table[NK_COLOR_SCROLLBAR] = nk_rgba(50, 58, 61, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(48, 83, 111, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(53, 88, 116, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(58, 93, 121, 255);
        table[NK_COLOR_TAB_HEADER] = nk_rgba(48, 83, 111, 255);
        nk_style_from_table(ctx, table);
    }
}

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
        {struct nk_font_atlas *atlas;
        struct nk_font *last = NULL;
        nk_glfw3_font_stash_begin(&ui_glfw, &atlas); // nk_sdl_font_stash_begin(&atlas);
        /*struct nk_font *droid = nk_font_atlas_add_from_file(atlas, "nuklear/extra_font/DroidSans.ttf", 14, 0); last = droid; */
        /*struct nk_font *roboto = nk_font_atlas_add_from_file(atlas, "nuklear/extra_font/Roboto-Regular.ttf", 16, 0); last = roboto; */
        /*struct nk_font *future = nk_font_atlas_add_from_file(atlas, "nuklear/extra_font/kenvector_future_thin.ttf", 13, 0); last = future; */
        /*struct nk_font *clean = nk_font_atlas_add_from_file(atlas, "nuklear/extra_font/ProggyClean.ttf", 12, 0); last = clean; */
        /*struct nk_font *tiny = nk_font_atlas_add_from_file(atlas, "nuklear/extra_font/ProggyTiny.ttf", 10, 0); last = tiny; */
        /*struct nk_font *cousine = nk_font_atlas_add_from_file(atlas, "nuklear/extra_font/Cousine-Regular.ttf", 13, 0); last = cousine; */
        nk_glfw3_font_stash_end(&ui_glfw); // nk_sdl_font_stash_end();
        /* nk_style_load_all_cursors(ctx, atlas->cursors); glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN); */
        if(last) nk_style_set_font(ui_ctx, &last->handle);}

        set_style(ui_ctx, THEME_DARK);  //_RED

        // enum { MAX_MEMORY = 2 * 1024 * 1024 };
        // nk_init_fixed(&ui_ctx, calloc(1, MAX_MEMORY), MAX_MEMORY, &font);
    }
}

static int ui_dirty = 1;
static int ui_popups_active = 0;

void ui_render() {
    /* IMPORTANT: `nk_sdl_render` modifies some global OpenGL state
     * with blending, scissor, face culling, depth test and viewport and
     * defaults everything back into a default state.
     * Make sure to either a.) save and restore or b.) reset your own state after
     * rendering the UI. */
    //nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
    nk_glfw3_render(&ui_glfw, NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
    ui_dirty = 1;
}


// ----------------------------------------------------------------------------

int ui_begin(const char *title, int flags) {

    ui_create();
    if(ui_dirty) {
        nk_glfw3_new_frame(&ui_glfw);
        ui_dirty = 0;
    }

#if 0
    static int rows = 0;
    static hashmap(const char*,int) map, *init = 0;
    if( !init ) { hashmap_create(init = &map, 128 /*maxwindows*/, str64); }
    int *found = hashmap_find(&map, title);
    if( !found ) hashmap_insert(&map, title, ++rows);
    int row = found ? *found : rows;
#else // fixme
    static const char *list[128] = {0};
    int row = -1, find = 0; while( find < 128 && list[find] ) {
        if( !strcmp(title, list[find]) ) { row = find; break; } else ++find;
    }
    if( row < 0 ) list[row = find] = strdup(title);
#endif

    int window_flags = nk_window_is_active(ui_ctx, title) ? 0 : NK_WINDOW_MINIMIZED; //NK_MINIMIZED;
    window_flags |= NK_WINDOW_BORDER;
    window_flags |= NK_WINDOW_SCALABLE;
    window_flags |= NK_WINDOW_MOVABLE;
    //window_flags |= NK_WINDOW_NO_SCROLLBAR;
    //window_flags |= NK_WINDOW_SCALE_LEFT;
    window_flags |= NK_WINDOW_MINIMIZABLE;

    vec2 offset = vec2(0, 32*row);
    float w = window_width() / 4, h = window_height() - offset.y * 2;
    if( nk_begin(ui_ctx, title, nk_rect(offset.x, offset.y, offset.x+w, offset.y+h), window_flags) ) {
        return 1;
    }

    nk_end(ui_ctx);
    return 0;
}

void ui_end() {
    nk_end(ui_ctx);
}

int ui_button(const char *label) {
    nk_layout_row_dynamic(ui_ctx, 0, 1);

    const char *split = strstr(label, "@"), *tooltip = split + 1;
    const struct nk_input *in = &ui_ctx->input;
    struct nk_rect bounds = nk_widget_bounds(ui_ctx);
    int ret = nk_button_label(ui_ctx, label); // nk_button_symbol_label(ui_ctx, NK_SYMBOL_TRIANGLE_RIGHT, label, NK_TEXT_RIGHT);
    //int ret = nk_button_symbol(ui_ctx, NK_SYMBOL_TRIANGLE_RIGHT); //
    if (split && nk_input_is_mouse_hovering_rect(in, bounds) && !ui_popups_active) {
        nk_tooltip(ui_ctx, tooltip);
    }
    return ret;
}

int ui_label_(const char *label, int alignment) {
    const char *split = strstr(label, "@"), *tooltip = split + 1;
    const struct nk_input *in = &ui_ctx->input;
    struct nk_rect bounds = nk_widget_bounds(ui_ctx);
    char buffer[128]; if( split ) label = (snprintf(buffer, 128, "%.*s", (int)(split-label), label), buffer); 
    nk_label(ui_ctx, label, alignment);
    if (split && nk_input_is_mouse_hovering_rect(in, bounds) && !ui_popups_active) {
        nk_tooltip(ui_ctx, tooltip);
    }
    return 1;
}

int ui_label_right(const char *label) {
    nk_layout_row_dynamic(ui_ctx, 0, 1);
    return ui_label_(label, NK_TEXT_RIGHT);
}

int ui_label(const char *label) {
    nk_layout_row_dynamic(ui_ctx, 0, 1);
    return ui_label_(label, NK_TEXT_LEFT);
}
int ui_labelb(const char *text, double value) {
    bool b = !!value;
    return ui_bool(text, &b), 0;
}
int ui_labelf(const char *text, double value) {
    char buf[128]; sprintf(buf, "%f", value);
    return ui_string(text, buf, 128), 0;
}
int ui_labels(const char *text, const char *fmt, ...) {
    char buf[128]; 
    va_list args;
    va_start(args,fmt);
    vsnprintf(buf, sizeof(buf)-1, fmt, args);
    va_end(args);
    return ui_string(text, buf, 128), 0;
}

int ui_color4(const char *label, float *color4) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    struct nk_colorf after = { color4[0]/255, color4[1]/255, color4[2]/255, color4[3]/255 }, before = after;
    if (nk_combo_begin_color(ui_ctx, nk_rgb_cf(after), nk_vec2(200,400))) {
        enum color_mode {COL_RGB, COL_HSV};
        static int col_mode = COL_RGB;
        #ifndef DEMO_DO_NOT_USE_COLOR_PICKER
        nk_layout_row_dynamic(ui_ctx, 120, 1);
        after = nk_color_picker(ui_ctx, after, NK_RGBA);
        #endif

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
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    size_t val = *slider * 100;
    int chg = nk_progress(ui_ctx, &val, 1000, NK_MODIFIABLE);
    *slider = val / 100.f;
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
    *v = nk_propertyi(ui_ctx, "#", 0, *v, INT_MAX, 1,1);
    return prev != *v;
}

int ui_short(const char *label, short *v) {
    int i = *v, ret = ui_int( label, &i );
    return *v = (short)i, ret;
}

int ui_float(const char *label, float *v) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    float prev = *v;
    *v = nk_propertyf(ui_ctx, "#", 0, *v, FLT_MAX, 0.01f,0.005f);
    return prev != *v;
}

int ui_string(const char *label, char *buffer, int buflen) {
    nk_layout_row_dynamic(ui_ctx, 0, 2);
    ui_label_(label, NK_TEXT_LEFT);

    int active = nk_edit_string_zero_terminated(ui_ctx, NK_EDIT_FIELD/*NK_EDIT_BOX*/|NK_EDIT_SIG_ENTER, buffer, buflen, nk_filter_default);
    return !!(active & NK_EDIT_COMMITED);
}

int ui_separator() {
    nk_layout_row_dynamic(ui_ctx, 1, 1);
//    nk_label(ui_ctx, "_______________________________________________", NK_TEXT_CENTERED);

            struct nk_command_buffer *canvas;
            struct nk_input *input = &ui_ctx->input;
            canvas = nk_window_get_canvas(ui_ctx);

            struct nk_rect space;
            enum nk_widget_layout_states state;
            state = nk_widget(&space, ui_ctx);
            if (state) nk_fill_rect(canvas, space, 0, ui_ctx->style.button.normal.data.color );

    return 1;
}

int ui_dialog(const char *title, const char *text, int choices, bool *show) {
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

int ui_menu() {
    #if 0 // @todo
    if( nk_begin(ui_ctx, "Menu", nk_rect(0, 0, window_width(), 32), NK_WINDOW_NO_SCROLLBAR|NK_WINDOW_BACKGROUND)){
        nk_menubar_begin(ui_ctx);
        nk_layout_row_begin(ui_ctx, NK_STATIC, 25, 2);
        nk_layout_row_push(ui_ctx, 45);
            if (nk_menu_begin_label(ui_ctx, "File", NK_TEXT_LEFT, nk_vec2(120, 200))) {
                nk_layout_row_dynamic(ui_ctx, 32, 1);
                nk_menu_item_label(ui_ctx, "Open", NK_TEXT_LEFT);
                nk_menu_item_label(ui_ctx, "Close", NK_TEXT_LEFT);
                nk_menu_end(ui_ctx);
            }
            nk_layout_row_push(ui_ctx, 45);
            if (nk_menu_begin_label(ui_ctx, "Edit", NK_TEXT_LEFT, nk_vec2(120, 200))) {
                nk_layout_row_dynamic(ui_ctx, 32, 1);
                nk_menu_item_label(ui_ctx, "Copy", NK_TEXT_LEFT);
                nk_menu_item_label(ui_ctx, "Cut", NK_TEXT_LEFT);
                nk_menu_item_label(ui_ctx, "Paste", NK_TEXT_LEFT);
                nk_menu_end(ui_ctx);
            }

        nk_layout_row_end(ui_ctx);
        nk_menubar_end(ui_ctx);
    }
    nk_end(ui_ctx);
    #endif
    return 0;
}

// ----------------------------------------------------------------------------

void ui_demo() {
    enum { dialog_buttons = 0 /*0,1,2*/ };
    static bool show_modal = 0;
    if( ui_begin("about", 0)) {
        if( ui_button("show details") ) show_modal = 1;
        if( ui_dialog("details", __FILE__ "\n" __DATE__ "\n" "Public Domain licensed.", dialog_buttons, &show_modal) ) {}
        ui_end();
    }
}

#endif
