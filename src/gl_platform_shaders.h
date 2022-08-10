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
/* "    fragment_position = vec3(world * vec4(position, 1.0));\n" */
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
