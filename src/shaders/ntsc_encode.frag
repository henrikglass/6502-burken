R""(
#version 330 core
#extension GL_ARB_arrays_of_arrays: enable
#extension GL_ARB_shading_language_420pack: enable

// Code mostly taken from https://www.shadertoy.com/view/llyGzR

const float VIEWPORT_WIDTH = 1920.0f;
const float VIEWPORT_HEIGHT = 1080.0f;

const float PI = 3.1415926535f;
const float TAU = 2.0f * PI;
const float PHI = 1.61803398874989484820459;

const float F_COL = 1.0f / 4.0f;

in vec2 uv;

out vec4 frag_color;

uniform float time;
uniform sampler2D frame_buffer_texture;

float noise(vec2 uv, float seed)
{
    return fract(tan(distance(uv * PHI, uv) * seed) * uv.x);
}

vec4 rgb_noise(vec2 uv, float seed)
{
    return vec4(noise(uv, seed),
                noise(uv, seed + 0.1f),
                noise(uv, seed + 0.2f),
                1.0f);
}

vec3 get_pixel_rgb()
{
    float x = gl_FragCoord.x / VIEWPORT_WIDTH;
    float y = gl_FragCoord.y / VIEWPORT_HEIGHT;
    return texture(frame_buffer_texture, vec2(x, y)).xyz;
    //return texture(frame_buffer_texture, uv).xyz;
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
    float fs    = VIEWPORT_WIDTH;
    float fcol  = fs * F_COL; 
    float n     = floor(gl_FragCoord.x);

    vec3 pixel_rgb = get_pixel_rgb();
    vec3 pixel_yiq = rgb_to_yiq(pixel_rgb);

    vec2 pixel_osc = oscillator(fcol, fs, n);
    float sig      = pixel_yiq.x + dot(pixel_osc, pixel_yiq.yz);

    frag_color = vec4(sig, 0.0f, 0.0f, 1.0f);

    // add noise
    frag_color.x *= 0.77f + 0.23f * rgb_noise(gl_FragCoord.xy, time).x;
}

)""

