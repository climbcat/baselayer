#ifndef __SHADERS_H__
#define __SHADERS_H__


#include <GL/glew.h>
#include "../baselayer.h"


void CheckShaderCompilationErrors(GLuint shader, const char *header_info) {
    int success;
    char info_log[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        printf("%s:%s\n", header_info, info_log);
    }
}
void CheckShaderLinkErrors(GLuint program, const char *header_info) {
    int success;
    char info_log[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, info_log);
        printf("%s:%s\n", header_info, info_log);
    }
}
void ShaderProgramLink(GLuint *program, const GLchar *vsh_src, const GLchar *fsh_src, const GLchar *frag_data_loc = "o_color") {
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vsh_src, NULL);
    glCompileShader(vs);
    CheckShaderCompilationErrors(vs, "vertex shader compilation error");

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fsh_src, NULL);
    glCompileShader(fs);
    CheckShaderCompilationErrors(fs, "fragment shader compilation error");

    *program = glCreateProgram();
    glAttachShader(*program, vs);
    glAttachShader(*program, fs);
    glBindFragDataLocation(*program, 0, frag_data_loc);
    glLinkProgram(*program);
    CheckShaderLinkErrors(*program, "shader program link error");

    glDeleteShader(vs);
    glDeleteShader(fs);
    glUseProgram(*program);
}

struct ScreenQuadTextureProgram {
    // draws a frame buffer / texture to the screen
    GLuint program = 0;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint tex = 0;
    u32 nverts = 0;

    const GLchar* vert_src = R"glsl(
        #version 330 core

        in vec2 position;
        in vec2 tex_coord;
        out vec2 coord;

        void main()
        {
            gl_Position = vec4(position, 0.0, 1.0);
            coord = tex_coord;
        }
    )glsl";
    const GLchar* frag_src = R"glsl(
        #version 330 core

        in vec2 coord;
        out vec4 o_color;
        uniform sampler2D sampler;

        void main()
        {
            o_color = texture(sampler, coord);
        }
    )glsl";

    void Init(u8* imgbuffer, u32 width, u32 height) {
        float sqreen_quad_verts[] = {
            1.0f,  1.0f, 1.0f, 1.0f,
            -1.0f,  1.0f, 0.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f, 0.0f
        };

        ShaderProgramLink(&program, vert_src, frag_src);
        u32 stride = 4;
        nverts = 4;

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        // texture
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // quad
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * stride * nverts, &sqreen_quad_verts, GL_STATIC_DRAW);

        GLint pos_attr = glGetAttribLocation(program, "position");
        glVertexAttribPointer(pos_attr, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), 0);
        glEnableVertexAttribArray(pos_attr);
        GLint tex_attr = glGetAttribLocation(program, "tex_coord");
        glVertexAttribPointer(tex_attr, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*) (2 * sizeof(float)));
        glEnableVertexAttribArray(tex_attr);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void Draw(u8* imgbuffer, u32 width, u32 height) {
        // clear to black, textures may use opaque pixels
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        // TODO: what is the better way to update texture data than to (probably) allocate every frame?
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imgbuffer);
        glGenerateMipmap(GL_TEXTURE_2D);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, nverts);
        glBindVertexArray(0);
    }
};


#endif
