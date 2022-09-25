R""(
#version 330 core
#extension GL_ARB_arrays_of_arrays: enable
#extension GL_ARB_shading_language_420pack: enable

// Some parts taken from here: https://www.shadertoy.com/view/XtlSD7
//                             https://www.shadertoy.com/view/Ms23DR
//                             https://www.shadertoy.com/view/ltB3zD

const float VIEWPORT_WIDTH = 1920.0f;
const float VIEWPORT_HEIGHT = 1080.0f;

//const vec4 BACKGROUND_COLOR = vec4(0.153f, 0.106f, 0.149f, 1.0f);
//const vec4 BACKGROUND_COLOR = vec4(0.153f, 0.706f, 0.949f, 1.0f);
const vec4 BACKGROUND_COLOR = vec4(0.0f, 0.0f, 0.0f, 1.0f);

const float PP_BRIGHTNESS = 1.5f;

const float PI = 3.1415926535f;
const float PHI = 1.61803398874989484820459;

out vec4 frag_color;

in vec2 uv;

uniform sampler2D frame_buffer_texture;

uniform float time;
uniform uint display_width;
uniform uint display_height;

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

vec2 curve(vec2 uv)
{
	uv = (uv - 0.5) * 2.0;
	uv *= 1.1;	
	uv.x *= 1.0 + pow((abs(uv.y) / 4.0), 3.0);
	uv.y *= 1.0 + pow((abs(uv.x) / 3.0), 3.0);
	uv  = (uv / 2.0) + 0.5;
	uv =  uv *0.92 + 0.04;
	return uv;
}

vec4 apply_grille_effect(vec4 in_color, vec2 uv)
{
    uv *= 0.5f;
    int pix = int(uv.x * VIEWPORT_WIDTH);
    float vert_stripes  = 0.05f * (sin( (float(VIEWPORT_WIDTH) *  (1.00f*PI)) * uv.x) + 19.0f); 
    float horiz_stripes = 0.1f  * (sin( (float(VIEWPORT_HEIGHT) * (1.27f*PI)) * uv.y) + 9.0f); 
    vec4 color = horiz_stripes * vert_stripes * in_color;
    color.a = 1.0f;
    return color;
}

vec4 apply_vignette(vec4 in_color, vec2 uv) 
{
    float vignette = uv.x * uv.y * ( 1.0 - uv.x ) * ( 1.0 - uv.y );
    vignette = clamp( pow( 16.0 * vignette, 0.3), 0.0, 1.0 );
    vec4 color = vignette * in_color;
    color.a = 1.0f;
    return color;
}

vec4 apply_rgb_grid_pattern(vec4 in_color, vec2 uv)
{
    ivec2 pixel = ivec2(VIEWPORT_WIDTH * uv.x, VIEWPORT_HEIGHT * uv.y);
    //int pi = int(gl_FragCoord.x + (int(gl_FragCoord.y / 3) % 2)) % 3;
    int pi = (pixel.x + ((pixel.y / 3) % 2)) % 3;
    int pix = pixel.x % 3 + 1;
    int piy = clamp(pixel.y % 4, 0, 1);
    vec4 color = in_color;
    switch (pix * piy) {
        case 0: color *= vec4(0.0f, 0.0f, 0.0f, 1.0f); break;
        case 1: color *= vec4(1.0f, 0.0f, 0.0f, 1.0f); break;
        case 2: color *= vec4(0.0f, 1.0f, 0.0f, 1.0f); break;
        case 3: color *= vec4(0.0f, 0.0f, 1.0f, 1.0f); break;
        default: color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    }
    return color;
}
void main()
{
    uint left_margin   = uint( (display_width - VIEWPORT_WIDTH) / 2 );
    uint bottom_margin = uint( (display_height - VIEWPORT_HEIGHT) / 2 );
    uint right_margin  = uint( VIEWPORT_WIDTH + left_margin );
    uint top_margin    = uint( VIEWPORT_HEIGHT + bottom_margin );

    // Place native fb in the middle of the display
    vec4 color;
    vec4 pnoise;
    if (    gl_FragCoord.x < left_margin   ||
            gl_FragCoord.x > right_margin  ||
            gl_FragCoord.y < bottom_margin ||
            gl_FragCoord.y > top_margin) {
        // outside margins
        color = BACKGROUND_COLOR;
    } else {
        // inside margins
        vec2 fb_uv = vec2(
            (gl_FragCoord.x - left_margin) / VIEWPORT_WIDTH,
            (gl_FragCoord.y - bottom_margin) / VIEWPORT_HEIGHT
        );
        fb_uv = curve(fb_uv);
        color = texture(frame_buffer_texture, fb_uv);
        if (    fb_uv.x > 1.0f || fb_uv.x < 0.0f ||
                fb_uv.y > 1.0f || fb_uv.y < 0.0f) {
            color = BACKGROUND_COLOR;
        }
        color *= PP_BRIGHTNESS;
        color *= 0.85f + 0.15f * rgb_noise(gl_FragCoord.xy, time);
        //color = apply_rgb_grid_pattern(color, fb_uv);
        color = apply_grille_effect(color, fb_uv);
        color = apply_vignette(color, fb_uv);
    }
    
    frag_color = color;

}

)""

