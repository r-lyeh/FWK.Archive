// -----------------------------------------------------------------------------
// in-game editor
// - rlyeh, public domain.
//
// when editor is on:
// - [ ] ESC, toggles editor on/off
// - [ ] ESC (long press), sends ESC key to game
// - [ ] TAB, toggles console/log and debugdraw on/off
// - [ ] TAB (long press), sends TAB key to game
// - [ ] F1, isolated view of selected entity on/off.
// - [ ] F1 (long press), sends F1 key to game
// - [ ] F5, plays/restarts the game
// - [ ] F5 (long press), sends F5 key to game
// - [ ] F12, toggles fullscreen on/off
// - [ ] F12 (long press), sends F12 key to game
// - [ ] PAUSE, toggles game pause on/off
// - [ ] PAUSE (long press), sends PAUSE key to game
// - [ ] LMB, select item, display hierarchy @ left sidebar, status @ right sidebar. right click to unselect
// - [ ] RMB, on selected item, display contextual menu: edit, edit script...
// - [ ] RMB, camera move if no gameobj is selected + WASDEC + wheel
// - [ ] SPC, on selected item, cycle transform: locate, rotate, scale
// - [ ] CTRL+S save, CTRL+L load restore point
// - [ ] IMPR save capture bug/info
//
// roadmap
// - [x] ray/object picking
// - [x] basic gizmos (@todo: fixed screen size, snapping)
// - [ ] introspect properties on selected object
// - [ ] add/rem entities, add/rem components, add/rem/pause/resume systems
// - [ ] widgets for (bool,str,flt,int,vec2,vec3,vec4,range,mesh,texture,audio,any other asset,combo of anything)
// - [ ] osc server for properties and editor behavior
// - [ ] multiple selections/select all
// - [ ] undo/redo
// - [ ] cut/copy/paste (ctrl-c to serialize)
// - [ ] menu: open, save, save as, save all, reload

API void  editor();
API bool  editor_active();
API vec3  editor_pick(float mouse_x, float mouse_y);

API char* dialog_load();
API char* dialog_save();

API int   gizmo(vec3 *pos, vec3 *rot, vec3 *sca);
API bool  gizmo_active();
