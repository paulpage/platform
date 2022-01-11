#include "glad/glad.h"
#include <GL/gl.h>
#include <GL/glx.h>
#include <stdio.h>
#include <stdlib.h>

#define Font XFontUnused
#include "platform.h"

// Timer
#include <time.h>
#include <unistd.h>
#include <stdint.h>

uint64_t get_performance_counter() {
    uint64_t t;
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC_RAW, &now);
    t = now.tv_sec;
    t *= 1000000000;
    t += now.tv_nsec;
    return t;
}

uint64_t get_performance_frequency() {
    return 1000000000;
}

// TODO This is from gl_drawing.c - maybe should be a header file?
void gl_drawing_init();

typedef struct State {
    // Input 
    int mouse_x;
    int mouse_y;
    float mouse_wheel_x;
    float mouse_wheel_y;
    int mouse_pressed[MOUSE_BUTTON_COUNT];
    int mouse_down[MOUSE_BUTTON_COUNT];
    char keys_down[KEY_COUNT];
    char keys_pressed[KEY_COUNT];
    // X11
    Display *display;
    Window window;
    // Window
    int screen_width;
    int screen_height;
    // App
    int should_quit;
    // Internal
    int should_draw_next_frame;
} State;
State state = {};


// Screen
int get_screen_width() {
    return state.screen_width;
}

int get_screen_height() {
    return state.screen_height;
}

// Control
void app_init(int screen_width, int screen_height, char *name) {

    state.mouse_x = 0;
    state.mouse_y = 0;
    state.mouse_wheel_x = 0.0f;
    state.mouse_wheel_y = 0.0f;
    for (int i = 0; i < MOUSE_BUTTON_COUNT; i++) {
        state.mouse_down[i] = 0;
        state.mouse_pressed[i] = 0;
    }
    for (int i = 0; i < KEY_COUNT; i++) {
        state.keys_down[i] = 0;
        state.keys_pressed[i] = 0;
    }

    state.should_quit = 0;
    state.screen_width = screen_width;
    state.screen_height = screen_height;
    state.display = XOpenDisplay(NULL);

    state.should_draw_next_frame = TRUE;


    int attrib[] = { GLX_RGBA,
        GLX_RED_SIZE, 1,
        GLX_GREEN_SIZE, 1,
        GLX_BLUE_SIZE, 1,
        GLX_DOUBLEBUFFER,
        None };

    int scrnum = DefaultScreen(state.display);
    Window root = RootWindow(state.display, scrnum);
    XVisualInfo *visinfo = glXChooseVisual(state.display, scrnum, attrib);
    if (!visinfo) {
        printf("Error: couldn't get an RGB, Double-buffered visual\n");
        exit(1);
    }

    // window attributes
    XSetWindowAttributes attr;
    attr.background_pixel = 0;
    attr.border_pixel = 0;
    attr.colormap = XCreateColormap(state.display, root, visinfo->visual, AllocNone);
    attr.event_mask = StructureNotifyMask | ExposureMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask;
    unsigned long mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

    state.window = XCreateWindow(state.display, root, 0, 0, state.screen_width, state.screen_height,
            0, visinfo->depth, InputOutput,
            visinfo->visual, mask, &attr);

    GLXContext ctx = glXCreateContext(state.display, visinfo, NULL, True);
    if (!ctx) {
        printf("Error: glXCreateContext failed\n");
        exit(1);
    }

    glXMakeCurrent(state.display, state.window, ctx);

    if (!gladLoadGLLoader((GLADloadproc)glXGetProcAddress)) {
        printf("Error: Failed to intitialize GLAD\n");
        exit(1);
    }

    /* glShadeModel(GL_FLAT); */
    XMapWindow(state.display, state.window);

    gl_drawing_init();
}

void app_quit() {
    // TODO cleanup
}

void process_events() {

    for (int i = 0; i < MOUSE_BUTTON_COUNT; i++) {
        state.mouse_pressed[i] = 0;
    }
    for (int i = 0; i < KEY_COUNT; i++) {
        state.keys_pressed[i] = 0;
    }
    state.mouse_wheel_x = 0.0f;
    state.mouse_wheel_y = 0.0f;

    XEvent event;
    while (XPending(state.display) > 0) {
        XNextEvent(state.display, &event);

        switch (event.type) {
            case Expose:
                // Redraw
                break;
            case ButtonPress:
                switch (event.xbutton.button) {
                    case Button1:
                        state.mouse_down[MOUSE_LEFT] = 1;
                        state.mouse_pressed[MOUSE_LEFT] = 1;
                        break;
                    case Button2:
                        state.mouse_down[MOUSE_RIGHT] = 1;
                        state.mouse_pressed[MOUSE_RIGHT] = 1;
                        break;
                    case Button3:
                        state.mouse_down[MOUSE_MIDDLE] = 1;
                        state.mouse_pressed[MOUSE_MIDDLE] = 1;
                        break;
                }
                break;
            case ButtonRelease:
                switch (event.xbutton.button) {
                    case Button1:
                        state.mouse_down[MOUSE_LEFT] = 0;
                        state.mouse_pressed[MOUSE_LEFT] = 0;
                        break;
                    case Button2:
                        state.mouse_down[MOUSE_RIGHT] = 0;
                        state.mouse_pressed[MOUSE_RIGHT] = 0;
                        break;
                    case Button3:
                        state.mouse_down[MOUSE_MIDDLE] = 0;
                        state.mouse_pressed[MOUSE_MIDDLE] = 0;
                        break;
                    case Button4:
                        state.mouse_wheel_y += 1.0f;
                        break;
                    case Button5:
                        state.mouse_wheel_y -= 1.0f;
                        break;
                }
                break;
            case MotionNotify:
                state.mouse_x = event.xmotion.x;
                state.mouse_y = event.xmotion.y;
                break;
            case ConfigureNotify:
                state.screen_width = event.xconfigure.width;
                state.screen_height = event.xconfigure.height;
                glViewport(0, 0, state.screen_width, state.screen_height);
                // Avoid flickering screen
                state.should_draw_next_frame = FALSE;
                /* glMatrixMode(GL_PROJECTION); */
                /* glLoadIdentity(); */
                /* glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0); */
                break;
            case KeyPress:
                // TODO
                break;

        }
    }
}

int app_should_quit() {
    return state.should_quit;
}

/* void set_clear_color(Color color) { */
/*     float r = (float)color.r / 255.0f; */
/*     float g = (float)color.g / 255.0f; */
/*     float b = (float)color.b / 255.0f; */
/*     float a = (float)color.a / 255.0f; */
/*     glClearColor(r, g, b, a); */
/* } */

/* void clear_screen() { */
/*     glClear(GL_COLOR_BUFFER_BIT); */
/* } */

void app_present() {
    /* if (state.should_draw_next_frame) { */
        glXSwapBuffers(state.display, state.window);
    /* } else { */
    /*     state.should_draw_next_frame = TRUE; */
    /* } */
}


// Input
int mouse_button_down(MouseButton button) {
    return state.mouse_down[button];
}

int mouse_button_pressed(MouseButton button) {
    return state.mouse_pressed[button];
}

int get_mouse_x() {
    return state.mouse_x;
}

int get_mouse_y() {
    return state.mouse_y;
}

float get_wheel_x() {
    return state.mouse_wheel_x;
}

float get_wheel_y() {
    return state.mouse_wheel_y;
}

int key_down(int key) {
    // TODO
}

int key_pressed(int key) {
    // TODO
}
