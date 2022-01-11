#include <stddef.h>

#include "glad/glad.h"
#include "platform.h"

#define COLOR_DEPTH 4
#define MAX_FONTS 8

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
static size_t file_length(FILE *f) {
	long len, pos;
	pos = ftell(f);
	fseek(f, 0, SEEK_END);
	len = ftell(f);
	fseek(f, pos, SEEK_SET);
	return (size_t) len;
}

static unsigned char *read_file(const char *filename, size_t *plen) {
    FILE *f = fopen(filename, "rb");
    size_t len = file_length(f);
    char *buffer = (char*)malloc(len);
    len = fread(buffer, 1, len, f);
    if (plen) *plen = len;
    return buffer;
}


typedef struct StbFont {
    int id;
    stbtt_fontinfo* info;
    stbtt_packedchar *cdata;
    Texture atlas;
    GLuint tex;
    int texture_size;
    float size;
    float scale;
    int ascent;
    int baseline;
} StbFont;

int font_count = 0;
StbFont *fonts[MAX_FONTS] = {0};
GLuint program_2d;
GLuint program_3d;
GLuint program_text;
GLuint program_texture;


const GLchar *VERT_SRC_2D =
"#version 330 core\n"
"layout (location = 0) in vec2 position;\n"
"layout (location = 1) in vec4 color;\n"
"out vec4 v_color;\n"
"void main() {\n"
"    v_color = color;\n"
"    gl_Position = vec4(position, 0.0, 1.0);\n"
"}\n";

const GLchar *FRAG_SRC_2D =
"#version 330 core\n"
"in vec4 v_color;\n"
"void main() {\n"
"    gl_FragColor = v_color;\n"
"}\n";

const GLchar *VERT_SRC_3D =
"#version 330 core\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec3 normal;\n"
"layout (location = 2) in vec4 color;\n"
"uniform mat4 world;\n"
"uniform mat4 view;\n"
"uniform mat4 proj;\n"
"out vec3 v_normal;\n"
"out vec4 v_color;\n"
"out vec3 fragment_position;\n"
"void main() {\n"
"   mat4 worldview = view * world;\n"
"    v_normal = transpose(inverse(mat3(worldview))) * normal;\n"
"    v_color = color;\n"
"    fragment_position = vec3(worldview * vec4(position, 1.0));\n"
"   gl_Position = proj * worldview * vec4(position, 1.0);\n"
"}\n";

const GLchar *FRAG_SRC_3D =
"#version 330 core\n"
"in vec3 v_normal;\n"
"in vec4 v_color;\n"
"in vec3 fragment_position;\n"
"struct Light {\n"
"    vec3 position;\n"
"    vec3 direction;\n"
"    vec3 ambient;\n"
"    vec3 diffuse;\n"
"    vec3 specular;\n"
"};\n"
"uniform vec3 view_position;\n"
"uniform Light light;\n"
"void main() {\n"
"    vec3 N = normalize(v_normal);\n"
"    vec3 L = normalize(light.position - fragment_position);\n"
"    vec3 E = normalize(-fragment_position);\n"
"    vec3 R = normalize(-reflect(L, N));\n"
"    vec3 ambient = light.ambient; \n"
"    vec3 diffuse = light.diffuse * max(dot(N, L), 0.0);\n"
"    diffuse = clamp(diffuse, 0.0, 1.0);\n"
"    vec3 specular = light.specular * pow(max(dot(R, E), 0.0), 32);\n"
"    specular = clamp(specular, 0.0, 1.0);\n"
"\n"
"    gl_FragColor = vec4((ambient + diffuse + specular), 1.0) * v_color;\n"
"}\n";

const GLchar *VERT_SRC_TEXT =
"#version 330 core\n"
"layout (location = 0) in vec2 position;\n"
"layout (location = 1) in vec2 tex_coords;\n"
"layout (location = 2) in vec4 color;\n"
"out vec2 v_tex_coords;\n"
"out vec4 v_color;\n"
"void main() {\n"
"    gl_Position = vec4(position, 0.0, 1.0);\n"
"    v_tex_coords = tex_coords;\n"
"    v_color = color;\n"
"}\n";

const GLchar *FRAG_SRC_TEXT =
"#version 330 core\n"
"uniform sampler2D tex;\n"
"in vec2 v_tex_coords;\n"
"in vec4 v_color;\n"
"out vec4 f_color;\n"
"void main() {\n"
"    f_color = v_color * vec4(1.0, 1.0, 1.0, texture(tex, v_tex_coords).r);\n"
"}\n";

const GLchar *VERT_SRC_2D_TEXTURE =
"#version 330 core\n"
"layout (location = 0) in vec2 position;\n"
"layout (location = 1) in vec2 tex_coords;\n"
"out vec2 v_tex_coords;\n"
"void main() {\n"
"    gl_Position = vec4(position, 0.0, 1.0);\n"
"    v_tex_coords = tex_coords;\n"
"}\n";

const GLchar *FRAG_SRC_2D_TEXTURE =
"#version 330 core\n"
"uniform sampler2D tex;\n"
"in vec2 v_tex_coords;\n"
"out vec4 f_color;\n"
"void main() {\n"
"    f_color = texture(tex, v_tex_coords);\n"
"}\n";

static GLuint create_shader(GLenum type, const GLchar *src) {
    GLuint id = glCreateShader(type);
    glShaderSource(id, 1, &src, NULL);
    glCompileShader(id);
    GLint success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        int len = 0;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
        char buffer[len]; // TODO do we have to zero this?
        glGetShaderInfoLog(id, len, NULL, buffer);
        printf("BAD: %s\n", buffer);
        glDeleteShader(id);
        return 0;
    }
    return id;
}

static GLuint create_program(const char *vert_src, const char *frag_src) {
    GLuint vert = create_shader(GL_VERTEX_SHADER, vert_src);
    GLuint frag = create_shader(GL_FRAGMENT_SHADER, frag_src);
    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);
    glDeleteShader(vert);
    glDeleteShader(frag);
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        int len = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
        char buffer[len]; // TODO do we have to zero this?
        glGetProgramInfoLog(program, len, NULL, buffer);
        printf("BAD PROGRAM: %s\n", buffer);
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

void gl_drawing_init() {
    program_2d = create_program(VERT_SRC_2D, FRAG_SRC_2D);
    program_3d = create_program(VERT_SRC_3D, FRAG_SRC_3D);
    program_text = create_program(VERT_SRC_TEXT, FRAG_SRC_TEXT);
    program_texture = create_program(VERT_SRC_2D_TEXTURE, FRAG_SRC_2D_TEXTURE);
}


void draw_rect(Rect rect, Color color) {
    float x = (float)rect.x * 2.0f / (float)get_screen_width() - 1.0f;
    float y = 1.0f - (float)rect.y * 2.0f / (float)get_screen_height();
    float width = (float)rect.w * 2.0f / (float)get_screen_width();
    float height = -1.0f * (float)rect.h * 2.0f / (float)get_screen_height();
    float c[4] = {
        (float)color.r / 255.0f,
        (float)color.g / 255.0f,
        (float)color.b / 255.0f,
        (float)color.a / 255.0f,
    };
    GLuint vao;
    GLuint vbo;
    GLfloat vertices[36] = {
        x, y, c[0], c[1], c[2], c[3],
        x + width, y, c[0], c[1], c[2], c[3],
        x + width, y + height, c[0], c[1], c[2], c[3],
        x, y, c[0], c[1], c[2], c[3],
        x + width, y + height, c[0], c[1], c[2], c[3],
        x, y + height, c[0], c[1], c[2], c[3], 
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
            GL_ARRAY_BUFFER,
            36 * sizeof(GLfloat),
            &vertices,
            GL_STATIC_DRAW);
    glBindVertexArray(vao);
    GLsizei stride = 6 * sizeof(GLfloat);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, NULL);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(2 * sizeof(GLfloat)));

    glUseProgram(program_2d);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}

Texture create_texture(int width, int height, unsigned char *buffer) {
    GLuint gl_id = 0;
    glGenTextures(1, &gl_id);
    glBindTexture(GL_TEXTURE_2D, gl_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

    Texture texture = {
        .id = gl_id,
        .width = width,
        .height = height,
    };
    return texture;
}

Texture load_texture(char *filename) {
    int x, y, n;
    unsigned char *data = stbi_load(filename, &x, &y, &n, 4);
    Texture texture = create_texture(x, y, data);
    stbi_image_free(data);
    return texture;
}

void free_texture(Texture *texture) {
    // TODO
}

void draw_texture(Texture texture, int x, int y) {
    Rect src_rect = {0, 0, texture.width, texture.height};
    Rect dst_rect = {x, y, texture.width, texture.height};
    draw_partial_texture(texture, src_rect, dst_rect);
}

void draw_partial_texture(Texture texture, Rect src_rect, Rect dest_rect) {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    float x0 = (float)dest_rect.x * 2.0f / (float)get_screen_width() - 1.0f;
    float x1 = (float)(dest_rect.x + dest_rect.w) * 2.0f / (float)get_screen_width() - 1.0f;
    float y0 = -1.0f * ((float)dest_rect.y * 2.0f / (float)get_screen_height() - 1.0f);
    float y1 = -1.0f * ((float)(dest_rect.y + dest_rect.h) * 2.0f / (float)get_screen_height() - 1.0f);
    float tx0 = (float)src_rect.x / (float)texture.width;
    float tx1 = (float)(src_rect.x + src_rect.w) / (float)texture.width;
    float ty0 = (float)src_rect.y / (float)texture.height;
    float ty1 = (float)(src_rect.y + src_rect.h) / (float)texture.height;

    GLfloat vertices[24] = {
        x0, y0, tx0, ty0,
        x1, y0, tx1, ty0,
        x1, y1, tx1, ty1,
        x0, y0, tx0, ty0,
        x1, y1, tx1, ty1,
        x0, y1, tx0, ty1,
    };

    GLuint vao = 0;
    GLuint vbo = 0;
    GLint uniform = glGetUniformLocation(program_texture, "tex");
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(GLfloat), &vertices, GL_STATIC_DRAW);
    glBindVertexArray(vao);
    GLsizei stride = 4 * sizeof(GLfloat);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glUniform1i(uniform, 0);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, NULL);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(2 * sizeof(GLfloat)));

    glUseProgram(program_texture);
    glDrawArrays(GL_TRIANGLES, 0, 24);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

Font load_font(const char *filename, int size) {
    unsigned char *buffer = read_file(filename, NULL);

	StbFont *font = calloc(sizeof(StbFont), 1);
	font->info = malloc(sizeof(stbtt_fontinfo));
	font->cdata = malloc(sizeof(stbtt_packedchar) * 96);

    Font font_handle = {font_count, size};
    fonts[font_count] = font;
    font_count++;

	if (stbtt_InitFont(font->info, buffer, 0) == 0) {
		free(buffer);
		free_font(font_handle);
		return (Font){-1, 0};
	}

	// fill bitmap atlas with packed characters
	unsigned char* bitmap = NULL;
	font->texture_size = 256;
    int count = 0;
	while(1) {
		bitmap = malloc(font->texture_size * font->texture_size);
		stbtt_pack_context pack_context;
		stbtt_PackBegin(&pack_context, bitmap, font->texture_size, font->texture_size, 0, 1, 0);
		stbtt_PackSetOversampling(&pack_context, 1, 1);
		if (!stbtt_PackFontRange(&pack_context, buffer, 0, size, 32, 95, font->cdata)) {
			// too small
			free(bitmap);
			stbtt_PackEnd(&pack_context);
			font->texture_size *= 2;
		} else {
			stbtt_PackEnd(&pack_context);
			break;
		}
	}

    glGenTextures(1, &font->tex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font->tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, font->texture_size, font->texture_size, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap);

	free(bitmap);

	// setup additional info
  font->scale = stbtt_ScaleForPixelHeight(font->info, size);
	stbtt_GetFontVMetrics(font->info, &font->ascent, 0, 0);
  font->baseline = (int) (font->ascent * font->scale);

	free(buffer);

	return font_handle;
}

void free_font(Font font) {
    // TODO
}


void draw_text(const char *text, int x, int y, Font font_handle, Color color) {

    StbFont *font = fonts[font_handle.id];

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    float fx = (float)x;
    float fy = (float)y;

    int count;
    for (count = 0; text[count]; count++) {}

    GLfloat *vertices = (GLfloat*)malloc(sizeof(GLfloat) * 48 * count);
    for (int i = 0; i < 48 * count; i++) {
        vertices[i] = 0.0f;
    }

    for (int i = 0; i < count; i++) {
        if (text[i] >= 32 && text[i] < 128) {
            stbtt_aligned_quad q;
            stbtt_GetPackedQuad(font->cdata, font->texture_size, font->texture_size, text[i] - 32, &fx, &fy, &q, 1);

            GLfloat x0 = (float)q.x0 * 2.0f / (float)get_screen_width() - 1.0f;
            GLfloat y0 = -1.0f * (((float)q.y0 + 16.0f) * 2.0f / (float)get_screen_height() - 1.0f);
            GLfloat x1 = (float)q.x1 * 2.0f / (float)get_screen_width() - 1.0f;
            GLfloat y1 = -1.0f * (((float)q.y1 + 16.0f) * 2.0f / (float)get_screen_height() - 1.0f);

            float c[4] = {
                (float)color.r / 255.0f,
                (float)color.g / 255.0f,
                (float)color.b / 255.0f,
                (float)color.a / 255.0f,
            };
            GLfloat char_vertices[48] = {
                x0, y0, q.s0, q.t0, c[0], c[1], c[2], c[3],
                x1, y0, q.s1, q.t0, c[0], c[1], c[2], c[3],
                x1, y1, q.s1, q.t1, c[0], c[1], c[2], c[3],
                x0, y0, q.s0, q.t0, c[0], c[1], c[2], c[3],
                x1, y1, q.s1, q.t1, c[0], c[1], c[2], c[3],
                x0, y1, q.s0, q.t1, c[0], c[1], c[2], c[3],
            };

            for (int j = 0; j < 48; j++) {
                vertices[i * 48 + j] = char_vertices[j];
            }
        }
    }

    GLuint vao = 0;
    GLuint vbo = 0;
    GLint uniform = glGetUniformLocation(program_text, "tex");
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 48 * sizeof(GLfloat) * count, vertices, GL_STATIC_DRAW);
    glBindVertexArray(vao);
    GLsizei stride = 8 * sizeof(GLfloat);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font->tex);
    glUniform1i(uniform, 0);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, NULL);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(4 * sizeof(GLfloat))); // TODO is this void pointer correct?

    glUseProgram(program_text);
    glDrawArrays(GL_TRIANGLES, 0, 48 * count);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

void set_clear_color(Color color) {
    glClearColor(
            (float)color.r / 255.0f,
            (float)color.g / 255.0f,
            (float)color.b / 255.0f,
            (float)color.a / 255.0f);
}

void clear_screen() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
