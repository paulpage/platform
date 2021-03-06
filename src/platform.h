#define TRUE 1
#define FALSE 0

typedef enum MouseButton {
    MOUSE_LEFT = 0,
    MOUSE_RIGHT = 1,
    MOUSE_MIDDLE = 2,
} MouseButton;

typedef struct Keymap {
    int dummy; // TODO
} Keymap;

typedef struct Key {
    int dummy; // TODO
} Key;

typedef struct MousemMap {
    char left;
    char right;
    char middle;
} MouseMap;

typedef struct AppState {
    int screen_height;
    int screen_width;
    struct {
        int x;
        int y;
        struct {
            char left;
            char right;
            char middle;
        } pressed;
        int down;
        int wheel_dx;
        int wheel_dy;
    } mouse;
    struct {
        Keymap down;
        Keymap pressed;
    } keys;
    int last_frame_time;
    int should_quit;
} AppState;

typedef struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} Color;

typedef struct Rect {
    int x, y, w, h;
} Rect;

typedef struct Texture {
    int width, height;
    unsigned char *buffer;
} Texture;

typedef struct Font {
    int id;
    int size;
} Font;

// Screen
int get_screen_width();
int get_screen_height();

// Control
void app_init();
void app_quit();
void process_events();
int app_should_quit();

// Input
int mouse_button_down(MouseButton button);
int mouse_button_pressed(MouseButton button);
int get_mouse_x();
int get_mouse_y();
float get_wheel_x();
float get_wheel_y();
int key_down(Key key);
int key_pressed(Key key);

// Drawing
void draw_rect(Rect rect, Color color);
void free_font(Font font_handle);
Font load_font(const char* filename, int size);
void draw_text(const char *text, int x, int y, Font font_handle, Color color);
int measure_text(const char *text, Font font_handle);
Texture create_texture(int width, int height);
Texture create_texture_from_bitmap(int width, int height, unsigned char *buffer);
Texture load_texture(char *filename);
void free_texture(Texture *texture);
void draw_texture(Texture texture, int x, int y);
void draw_texture_rect(Texture texture, Rect src_rect, Rect dest_rect);
void set_clear_color(Color color);
void clear_screen();
void app_present();

// Time
int get_last_frame_time();
