#version 330 core

in vec2 frag_uv;
in vec4 frag_tint;

out vec4 color;

uniform sampler2D u_texture;

void main(void) {
    color = frag_tint * texture(u_texture, frag_uv);
}
