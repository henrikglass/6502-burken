R""(
#version 330 core
#extension GL_ARB_arrays_of_arrays: enable
#extension GL_ARB_shading_language_420pack: enable

// Code mostly taken from https://www.shadertoy.com/view/llyGzR

const float VIEWPORT_WIDTH = 1320.0f;
const float VIEWPORT_HEIGHT = 820.0f;

const float PI = 3.1415926535f;
const float TAU = 2.0f * PI;

const float HUE        = 0.03f;
const float SATURATION = 43.0f;
const float BRIGHTNESS =  1.45f;

const float F_COL     = 1.0f /  4.0f;
const float F_LUMA_LP = 1.0f /  6.0f;
const float F_COL_BW  = 1.0f / 50.0f;
const int   FIR_SIZE  = 29;

out vec4 frag_color;

in vec2 uv;

uniform sampler2D frame_buffer_texture;

vec3 get_pixel_rgb()
{
    float x = gl_FragCoord.x / (VIEWPORT_WIDTH);
    float y = gl_FragCoord.y / (VIEWPORT_HEIGHT);
    return texture(frame_buffer_texture, vec2(x, y)).xyz;
    //return texture(frame_buffer_texture, uv).xyz;
}

vec3 yiq_to_rgb(vec3 yiq)
{
    mat3 yiq2rgb = mat3(1.000, 1.000, 1.000,
                        0.956,-0.272,-1.106,
                        0.621,-0.647, 1.703);
    return yiq2rgb * yiq;
}

//Non-normalized texture sampling.
vec4 sample2D(sampler2D sampler,vec2 resolution, vec2 uv)
{
    return texture(sampler, uv / resolution);
}

//Complex multiply
vec2 cmul(vec2 a, vec2 b)
{
   return vec2((a.x * b.x) - (a.y * b.y), (a.x * b.y) + (a.y * b.x));
}

float sinc(float x)
{
	return (x == 0.0) ? 1.0 : sin(x * PI) / (x * PI);
}

//https://en.wikipedia.org/wiki/Window_function
float WindowBlackman(float a, int N, int i)
{
    float a0 = (1.0 - a) / 2.0;
    float a1 = 0.5;
    float a2 = a / 2.0;

    float wnd = a0;
    wnd -= a1 * cos(2.0 * PI * (float(i) / float(N - 1)));
    wnd += a2 * cos(4.0 * PI * (float(i) / float(N - 1)));

    return wnd;
}

//FIR lowpass filter
//Fc = Cutoff freq., Fs = Sample freq., N = # of taps, i = Tap index
float lowpass(float Fc, float Fs, int N, int i)
{
    float wc = (Fc/Fs);

    float wnd = WindowBlackman(0.16, N, i);

    return 2.0*wc * wnd * sinc(2.0*wc * float(i - N/2));
}

//Angle -> 2D rotation matrix 
mat2 rotate(float a)
{
    return mat2( cos(a), sin(a),
                -sin(a), cos(a));
}

void main()
{
    float Fs = VIEWPORT_WIDTH;
    float Fcol = Fs * F_COL;
    float Fcolbw = Fs * F_COL_BW;
    float Flumlp = Fs * F_LUMA_LP;
    float n = floor(gl_FragCoord.x);

    vec2 screen_uv = gl_FragCoord.xy;

    float luma = sample2D(frame_buffer_texture, vec2(VIEWPORT_WIDTH, VIEWPORT_HEIGHT), screen_uv).r;
    vec2 chroma = vec2(0);

    //Filtering out unwanted high freqency content from the chroma(IQ) signal.
    for(int i = 0; i < FIR_SIZE; i++) {
        int tpidx = FIR_SIZE - i - 1;
        float lp = lowpass(Flumlp, Fs, FIR_SIZE, tpidx);
        chroma += sample2D(frame_buffer_texture, vec2(VIEWPORT_WIDTH, VIEWPORT_HEIGHT), screen_uv - vec2(i - FIR_SIZE / 2, 0)).yz * lp;
    }

    chroma *= rotate(TAU * HUE);

    vec3 color = yiq_to_rgb(vec3(luma * BRIGHTNESS, chroma * SATURATION));

    frag_color = vec4(color, 1.0f);
   
    // Color adjusts
    frag_color.r *= 0.75f;
    frag_color.b *= 0.95f;
}

)""

