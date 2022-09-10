R""(
#version 330 core
#extension GL_ARB_arrays_of_arrays: enable
#extension GL_ARB_shading_language_420pack: enable

// Code mostly taken from https://www.shadertoy.com/view/llyGzR

const float VIEWPORT_WIDTH = 1320.0f;
const float VIEWPORT_HEIGHT = 820.0f;

const float PI = 3.1415926535f;
const float TAU = 2.0f * PI;

const float F_COL     = 1.0f /  4.0f;
const float F_LUMA_LP = 1.0f /  6.0f;
const float F_COL_BW  = 1.0f / 50.0f;
const int   FIR_SIZE  = 29;

out vec4 frag_color;

in vec2 uv;

uniform sampler2D frame_buffer_texture;

vec3 get_pixel_rgb()
{
    //float x = gl_FragCoord.x / (VIEWPORT_WIDTH);
    //float y = gl_FragCoord.y / (VIEWPORT_HEIGHT);
    //return texture(frame_buffer_texture, vec2(x, y)).xyz;
    return texture(frame_buffer_texture, uv).xyz;
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

//FIR bandpass filter
//Fa/Fb = Low/High cutoff freq., Fs = Sample freq., N = # of taps, i = Tap index
float bandpass(float Fa, float Fb, float Fs, int N, int i)
{
    float wa = (Fa/Fs);
    float wb = (Fb/Fs);

    float wnd = WindowBlackman(0.16, N, i);

    return 2.0*(wb-wa) * wnd * (sinc(2.0*wb * float(i - N/2)) - sinc(2.0*wa * float(i - N/2)));
}

vec2 oscillator(float oscillator_freq, float sample_freq, float sample_idx)
{
    float phase = (TAU * oscillator_freq * floor(sample_idx)) / sample_freq;
    return vec2(cos(phase), sin(phase));
}

void main()
{
    float Fs = VIEWPORT_WIDTH;
    float Fcol = Fs * F_COL;
    float Fcolbw = Fs * F_COL_BW;
    float Flumlp = Fs * F_LUMA_LP;
    float n = floor(gl_FragCoord.x);

    float y_sig = 0.0;
    float iq_sig = 0.0;

    vec2 cOsc = oscillator(Fcol, Fs, n);

    n += float(FIR_SIZE)/2.0;

    //Separate luma(Y) & chroma(IQ) signals
    for(int i = 0;i < FIR_SIZE;i++)
    {
        int tpidx = FIR_SIZE - i - 1;
        float lp = lowpass(Flumlp, Fs, FIR_SIZE, tpidx);
        float bp = bandpass(Fcol - Fcolbw, Fcol + Fcolbw, Fs, FIR_SIZE, tpidx);

        y_sig  += sample2D(frame_buffer_texture, vec2(VIEWPORT_WIDTH, VIEWPORT_HEIGHT), vec2(n - float(i), gl_FragCoord.y)).r * lp;
        iq_sig += sample2D(frame_buffer_texture, vec2(VIEWPORT_WIDTH, VIEWPORT_HEIGHT), vec2(n - float(i), gl_FragCoord.y)).r * bp;
    }

    //Shift IQ signal down from Fcol to DC
    vec2 iq_sig_mix = cmul(vec2(iq_sig, 0), cOsc);

    frag_color = vec4(y_sig, iq_sig_mix, 1.0f);
}

)""

