#ifndef EVENT_H
#define EVENT_H

SDL_AppResult event_keydown(AppState *app, SDL_KeyboardEvent event);
SDL_AppResult event_mousedown(AppState *app, SDL_MouseButtonEvent event);
SDL_AppResult event_mouseup(AppState *app, SDL_MouseButtonEvent event);
SDL_AppResult event_mousewheel(AppState *app, SDL_MouseWheelEvent event);

#endif