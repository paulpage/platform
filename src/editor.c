#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

typedef struct Config {
    int font_size;
    Color background_color;
    Color text_color;
} Config;

typedef struct State {
    Font font;
    int scroll_offset;
} State;

typedef struct File {
    size_t size;
    size_t line_count;
    char *name;
    char **lines;
} File;

size_t file_length(FILE *f) {
	long len, pos;
	pos = ftell(f);
	fseek(f, 0, SEEK_END);
	len = ftell(f);
	fseek(f, pos, SEEK_SET);
	return (size_t) len;
}

char **read_file_lines(char *filename, size_t *file_size, size_t *line_count) {
    FILE *f = fopen(filename, "rb");
    char *buffer, **list=NULL, *s;
    size_t len, count, i;

    if (!f) return NULL;
    len = file_length(f);
    buffer = (char *) malloc(len+1);
    len = fread(buffer, 1, len, f);
    buffer[len] = 0;
    fclose(f);
    if (file_size) *file_size = len;

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
            buffer = (char*)&list[count + 1];
            if (line_count) *line_count = (int)count;
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

File open_file(char *filename) {
    File file = {};

    int name_size = strlen(filename) + 1;
    file.name = malloc(name_size);
    memcpy(file.name, filename, name_size);

    file.lines = read_file_lines(filename, &file.size, &file.line_count);

    return file;
}

int main(int argc, char *argv[]) {
    app_init(800, 600, "Window");
    Config config = {
        .font_size = 16,
        .background_color = (Color){10, 10, 10, 255},
        .text_color = (Color){240, 240, 240, 255},
    };
    State state = {
        .font = load_font("/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf", config.font_size),
        .scroll_offset = 0,
    };

    set_clear_color(config.background_color);

    File file = open_file("/home/paul/s/platform/src/editor.c");

    while (!app_should_quit()) {
        process_events();

        state.scroll_offset += (int)(get_wheel_y() * (float)config.font_size * 3.0f);

        clear_screen();
        for (int i = 0; i < file.line_count; i++) {
            draw_text(file.lines[i], 5, state.scroll_offset + i * config.font_size, state.font, config.text_color);
        }
        app_present();
    }
}
