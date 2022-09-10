R""(

#version 330 core

layout (location = 0) in vec3 in_xyz;
layout (location = 1) in vec2 in_uv;

out vec2 uv;

void main(void) {
    gl_Position = vec4(in_xyz, 1.0);
    uv = in_uv;
}

)""

