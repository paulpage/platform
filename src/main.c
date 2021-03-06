#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "platform.h"

size_t file_length(FILE *f) {
	long len, pos;
	pos = ftell(f);
	fseek(f, 0, SEEK_END);
	len = ftell(f);
	fseek(f, pos, SEEK_SET);
	return (size_t) len;
}

char **read_file_lines(char *filename, int *plen) {
    FILE *f = fopen(filename, "rb");
    char *buffer, **list=NULL, *s;
    size_t len, count, i;

    if (!f) return NULL;
    len = file_length(f);
    buffer = (char *) malloc(len+1);
    len = fread(buffer, 1, len, f);
    buffer[len] = 0;
    fclose(f);

    // two passes through: first time count lines, second time set them
    for (i = 0; i < 2; i++) {
        s = buffer;
        if (i == 1) {
            list[0] = s;
        }
        count = 1;
        while (*s) {
            if (*s == '\n' || *s == '\r') {
                // detect if both cr & lf are together
                int crlf = (s[0] + s[1]) == ('\n' + '\r');
                if (i == 1) *s = 0;
                if (crlf) s++;
                if (s[1]) { // it's not over yet
                    if (i == 1) list[count] = s+1;
                    count++;
                }
            }
            s++;
        }
        if (i == 0) {
            list = (char **) malloc(sizeof(*list) * (count+1) + len+1);
            if (!list) return NULL;
            list[count] = 0;
            // recopy the file so there's just a single allocation to free
            memcpy(&list[count+1], buffer, len + 1);
            free(buffer);
            buffer = (char *) &list[count + 1];
            if (plen) *plen = (int) count;
        }
    }
    return list;
}

unsigned char *read_file(char *filename, size_t *plen) {
    FILE *f = fopen(filename, "rb");
    size_t len = file_length(f);
    char *buffer = (char*)malloc(len);
    len = fread(buffer, 1, len, f);
    if (plen) *plen = len;
    return buffer;
}

int main(int argc, char *argv[]) {

    struct {
        int font_size;
    } config = {
        .font_size = 32,
    };

    int font_size = 32;

    app_init(800, 600, "Window");
    set_clear_color((Color){255, 0, 0, 255});
    Font font = load_font("/usr/share/fonts/TTF/DejaVuSans.ttf", config.font_size);

    int line_count;
    char **file = read_file_lines("src/main.c", &line_count);

    int scroll_offset = 0;

    while (!app_should_quit()) {
        process_events();

        if (mouse_button_pressed(MOUSE_LEFT)) {
            printf("Left Pressed\n");
        }
        if (mouse_button_down(MOUSE_LEFT)) {
            printf("Left down\n");
        }
        scroll_offset += (int)(get_wheel_y() * (float)config.font_size * 3.0f);

        clear_screen();

        draw_rect((Rect){get_mouse_x() - 25, get_mouse_y() - 25, 50, 50}, (Color){255, 255, 255, 255});

        draw_rect((Rect){5, 5, 100, 100}, (Color){255, 0, 255, 255});
        draw_rect((Rect){get_screen_width() - 105, 5, 100, 100}, (Color){255, 255, 255, 255});
        draw_rect((Rect){5, get_screen_height() - 105, 100, 100}, (Color){255, 255, 255, 255});
        draw_rect((Rect){get_screen_width() - 105, get_screen_height() - 105, 100, 100}, (Color){255, 255, 255, 255});
        for (int i = 0; i < line_count; i++) {
            draw_text(file[i], 5, scroll_offset + i * 25, font, (Color){0, 0, 0, 255});
        }

        app_present();
    }

    free_font(font);
    app_quit();
    return 0;
}
