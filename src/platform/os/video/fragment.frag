#version 330 core

in vec2 frag_uv;

out vec4 color;

uniform sampler2D u_texture;
uniform vec4 u_color;

void main(void) {
    color = u_color * texture(u_texture, frag_uv);
}