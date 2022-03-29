R""(
#version 330 core
#extension GL_ARB_arrays_of_arrays: enable
#extension GL_ARB_shading_language_420pack: enable

// Code mostly taken from https://www.shadertoy.com/view/llyGzR

const float VIEWPORT_WIDTH = 1280.0f;
const float VIEWPORT_HEIGHT = 600.0f;

const float PI = 3.1415926535f;
const float TAU = 2.0f * PI;

const float F_COL = 1.0f / 4.0f;

out vec4 frag_color;

uniform sampler2D frame_buffer_texture;

vec3 get_pixel_rgb()
{
    float x = gl_FragCoord.x / VIEWPORT_WIDTH;
    float y = gl_FragCoord.y / VIEWPORT_HEIGHT;
    return texture(frame_buffer_texture, vec2(x, y)).xyz;
}

vec3 rgb_to_yiq(vec3 rgb)
{
    mat3 rgb2yiq = mat3(0.299, 0.596, 0.211,
                        0.587,-0.274,-0.523,
                        0.114,-0.322, 0.312);
    return rgb2yiq * rgb;
}

vec2 oscillator(float oscillator_freq, float sample_freq, float sample_idx)
{
    float phase = (TAU * oscillator_freq * floor(sample_idx)) / sample_freq;
    return vec2(cos(phase), sin(phase));
}

void main() 
{
    frag_color = vec4(get_pixel_rgb(), 1.0f);
}

)""

