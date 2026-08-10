#version 330 core

out vec2 frag_uv;
out vec4 frag_tint;

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_uv;
layout(location = 3) in vec4 v_tint;

uniform mat4 u_matrix;

void main(void) {
    gl_Position = u_matrix * vec4(v_position, 1.0);

    frag_uv = v_uv;
	frag_tint = v_tint;
}
