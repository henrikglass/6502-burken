R""(

#version 330 core

layout (location = 0) in vec3 in_vec;

void main(void) {
    gl_Position = vec4(in_vec, 1.0);
}

)""

