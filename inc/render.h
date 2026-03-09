#ifndef RENDER_H
#define RENDER_H

void render_frame(AppState *app);

Vec2 render_to_pos(AppState *app, Vec2 pos);
Vec2 pos_to_render(AppState *app, Vec2 render);

#endif