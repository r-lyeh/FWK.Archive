// -----------------------------------------------------------------------------
// immediate ui framework
// - rlyeh, public domain
//
// @todo: logger/console

API int  ui_begin(const char *title, int flags);
API int    ui_int(const char *label, int *value);
API int    ui_bool(const char *label, bool *value);
API int    ui_short(const char *label, short *value);
API int    ui_float(const char *label, float *value);
API int    ui_float2(const char *label, float value[2]);
API int    ui_float3(const char *label, float value[3]);
API int    ui_float4(const char *label, float value[4]);
API int    ui_string(const char *label, char *buffer, int buflen);
API int    ui_color3(const char *label, float *color3); //[0..255]
API int    ui_color3f(const char *label, float *color3); //[0..1]
API int    ui_color4(const char *label, float *color4); //[0..255]
API int    ui_color4f(const char *label, float *color4); //[0..1]
API int    ui_button(const char *label);
API int    ui_toggle(const char *label, bool *value);
API int    ui_dialog(const char *title, const char *text, int choices, bool *show); // @fixme: return
API int    ui_list(const char *label, const char **items, int num_items, int *selector);
API int    ui_radio(const char *label, const char **items, int num_items, int *selector);
API int    ui_image(const char *label, handle id, unsigned w, unsigned h); //(w,h) can be 0
API int    ui_colormap(const char *map_name, colormap_t *cm); // returns num member changed: 1 for color, 2 for texture map
API int    ui_separator();
API int    ui_bits8(const char *label, uint8_t *bits);
API int    ui_bits16(const char *label, uint16_t *bits);
API int    ui_clampf(const char *label, float *value, float minf, float maxf);
API int    ui_label(const char *label);
#define    ui_labelf(...) ui_label(va(__VA_ARGS__))
API int    ui_label2(const char *label, const char *caption);
API int    ui_slider(const char *label, float *value);
API int    ui_slider2(const char *label, float *value, const char *caption);
API int    ui_const_bool(const char *label, const double value);
API int    ui_const_float(const char *label, const double value);
API int    ui_const_string(const char *label, const char *value);
#define    ui_const_stringf(label, ...) ui_const_string(label, va(__VA_ARGS__))
API void ui_end();

API int  ui_menu(const char *items); // semicolon- or comma-separated items
API int  ui_item();

API int  ui_hover(); // ui_is_hover()?
API int  ui_active(); // ui_is_active()?
API void ui_demo();
