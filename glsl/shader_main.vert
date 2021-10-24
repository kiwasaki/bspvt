// vertex shader for linearly transformed cosines [Heitz et al. 2016]

#version 410

layout ( location = 0 ) in vec3 vertex_position;
layout ( location = 1 ) in vec3 vertex_normal;
layout ( location = 2 ) in vec3 vertex_color;
layout ( location = 3 ) in vec2 vertex_texcoord;

uniform mat4x4 mvp;
out vec3 position;
out vec3 color;
out vec2 texcoord;

void main() {
    gl_Position = mvp * vec4( vertex_position, 1.f );
    position    = vertex_position;
    color       = vertex_color;
    texcoord    = vertex_texcoord;
}
