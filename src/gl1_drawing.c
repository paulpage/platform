typedef struct State {
    int screen_width;
    int screen_height;
} State;
State state = {};

void draw_rect(Rect rect, Color color) {
    float x = (float)rect.x * 2.0f / (float)state.screen_width - 1.0f;
    float y = 1.0f - (float)rect.y * 2.0f / (float)state.screen_height;
    float width = (float)rect.w * 2.0f / (float)state.screen_width;
    float height = -1.0f * (float)rect.h * 2.0f / (float)state.screen_height;
    float c[4] = {
        (float)color.r / 255.0f,
        (float)color.g / 255.0f,
        (float)color.b / 255.0f,
        (float)color.a / 255.0f,
    };

    GLfloat vertices[36] = {
        x, y, c[0], c[1], c[2], c[3],
        x + width, y, c[0], c[1], c[2], c[3],
        x + width, y + height, c[0], c[1], c[2], c[3],
        x, y, c[0], c[1], c[2], c[3],
        x + width, y + height, c[0], c[1], c[2], c[3],
        x, y + height, c[0], c[1], c[2], c[3], 
    };

    glColor3f(c[0], c[1], c[2]);
    glRectf(x, y, x + width, y + height);
}

void draw_texture(Texture texture, Rect rect) {
    Rect src_rect = {0, 0, texture.width, texture.height};
    draw_partial_texture(texture, src_rect, rect);
}

void draw_partial_texture(Texture texture, Rect src_rect, Rect dest_rect) {

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    float x0 = (float)dest_rect.x * 2.0f / (float)state.screen_width - 1.0f;
    float x1 = (float)(dest_rect.x + dest_rect.w) * 2.0f / (float)state.screen_width - 1.0f;
    float y0 = -1.0f * ((float)dest_rect.y * 2.0f / (float)state.screen_height - 1.0f);
    float y1 = -1.0f * ((float)(dest_rect.y + dest_rect.h) * 2.0f / (float)state.screen_height - 1.0f);
    float tx0 = (float)src_rect.x / (float)texture.width;
    float tx1 = (float)(src_rect.x + src_rect.w) / (float)texture.width;
    float ty0 = (float)src_rect.y / (float)texture.height;
    float ty1 = (float)(src_rect.y + src_rect.h) / (float)texture.height;

    glBindTexture(GL_TEXTURE_2D, texture.id);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glTexCoord2f(tx0, ty0); glVertex2f(x0, y0);
    glTexCoord2f(tx0, ty1); glVertex2f(x0, y1);
    glTexCoord2f(tx1, ty1); glVertex2f(x1, y1);
    glTexCoord2f(tx1, ty0); glVertex2f(x1, y0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFlush(); //don't need this with GLUT_DOUBLE and glutSwapBuffers
}

Texture create_texture(int width, int height, unsigned char *data) {
    GLuint id = 0;
    glGenTextures(1, &id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    Texture texture = { id, width, height };
    return texture;
}

Texture load_texture(char *filename) {
    int x, y, n;
    unsigned char *data = stbi_load(filename, &x, &y, &n, 4);
    Texture texture = create_texture(x, y, data);
    stbi_image_free(data);
    return texture;
}

// Window
void resize_window(int width, int height) {
    state.screen_width = width;
    state.screen_height = height;
}

int get_screen_width() {
    return state.screen_width;
}

int get_screen_height() {
    return state.screen_height;
}
