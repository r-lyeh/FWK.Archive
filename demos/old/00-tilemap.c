// tilemap demo
// - rlyeh, public domain.

#include "fwk.h"

int main() {
    window_create(0.75, 0);

    camera_t cam = camera();
    cam.position = vec3(window_width()/2,window_height()/2,3); // at(center) zoom(x3)
    camera_enable(&cam);

    texture_t sheet = texture("castle-tileset-by-RottingPixels-(CC0)", 0);
    tileset_t set = tileset(sheet, 16,16, 8,16); 
    tilemap_t map = tilemap("\1\2\3\n\4\5\6", ' ','\n');

    while( window_swap() ) {
        // camera panning (x,y) & zooming (z)
        camera_t cam = *camera_get_active();
        if( !ui_hover() && !ui_active() ) {
            if( input(MOUSE_L) ) cam.position.x += input_diff(MOUSE_X);
            if( input(MOUSE_L) ) cam.position.y += input_diff(MOUSE_Y);
            cam.position.z += input_diff(MOUSE_W) * 0.1; // cam.p.z += 0.001f; for tests
        }
        camera_enable(&cam);

        // update map position and render
        map.position = camera_get_active()->position;
        tilemap_render(map, set);

        // ui
        if( ui_panel("tileset", 0) ) {
            set.selected = tileset_ui(set);
            ui_panel_end();        
        }
    }
}
