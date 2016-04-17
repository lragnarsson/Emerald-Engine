#ifndef STATE_H
#define STATE_H

enum render_mode {
    FORWARD,
    DEFERRED
};

struct State
{
    bool running;
    render_mode current_render_mode;
};

#endif
