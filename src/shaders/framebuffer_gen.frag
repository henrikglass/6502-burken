R""(
#version 330 core
#extension GL_ARB_arrays_of_arrays: enable
#extension GL_ARB_shading_language_420pack: enable


/*
 * Various constants (TODO uniforms?) describing the size and layout of 
 * things. To make the math work out, it's important that these values 
 * are reasonably chosen. Although, 640x200 should be enough.
 * 
 * E.g. VIEWPORT_WIDTH should be a multiple of CHAR_SIZE * TEXT_COLS 
 * (640, 1280, 2560, etc.)
 */
const uint CHAR_SIZE       =    8u;
const uint CHARSET_SIZE    =  128u;
const uint VIEWPORT_WIDTH  =  640u;
const uint VIEWPORT_HEIGHT =  200u;
const uint LEFT_MARGIN     =   10u;
const uint BOTTOM_MARGIN   =    5u;
const uint TEXT_COLS       =   80u;
const uint TEXT_ROWS       =   25u;

/*
 * The size of a tile in viewport pixels
 */
const uint TILE_WIDTH_IN_PIXELS  = VIEWPORT_WIDTH / TEXT_COLS;
const uint TILE_HEIGHT_IN_PIXELS = VIEWPORT_HEIGHT / TEXT_ROWS;

/*
 * The ratio of viewport pixels to "char pixels".
 */
const uint PIXEL_RATIO_X = VIEWPORT_WIDTH / (CHAR_SIZE*TEXT_COLS);
const uint PIXEL_RATIO_Y = VIEWPORT_HEIGHT / (CHAR_SIZE*TEXT_ROWS);

const uint VGA_CTRL_BLINK_BIT      = 0u;
const uint VGA_CTRL_INVERSE_BIT    = 1u;
const uint VGA_CTRL_MONOCHROME_BIT = 2u;

/*
 * Color palette is based on "NA16" with some changes.
 *
 * from: https://lospec.com/palette-list/na16
 */
const vec4 palette[16] = {
    vec4(0.118f, 0.118f, 0.118f, 1.0f),  // 0b0000 // #1e1e1e
    vec4(0.090f, 0.263f, 0.294f, 1.0f),  // 0b0001
    vec4(0.204f, 0.522f, 0.616f, 1.0f),  // 0b0010
    vec4(0.494f, 0.769f, 0.757f, 1.0f),  // 0b0011
    vec4(0.439f, 0.216f, 0.498f, 1.0f),  // 0b0100
    vec4(0.824f, 0.392f, 0.443f, 1.0f),  // 0b0101
    vec4(0.616f, 0.118f, 0.231f, 1.0f),  // 0b0110
    vec4(0.894f, 0.580f, 0.227f, 1.0f),  // 0b0111
    vec4(0.392f, 0.490f, 0.204f, 1.0f),  // 0b1000
    vec4(0.753f, 0.780f, 0.255f, 1.0f),  // 0b1001
    vec4(0.961f, 0.929f, 0.729f, 1.0f),  // 0b1010
    vec4(0.843f, 0.608f, 0.490f, 1.0f),  // 0b1011
    vec4(0.604f, 0.388f, 0.282f, 1.0f),  // 0b1100
    vec4(0.243f, 0.129f, 0.216f, 1.0f),  // 0b1101
    vec4(0.345f, 0.271f, 0.388f, 1.0f),  // 0b1110
    vec4(0.549f, 0.561f, 0.682f, 1.0f),  // 0b1111
};

in vec2 uv;
out vec4 frag_color;

uniform int vga_ctrl_register;
uniform float time;

uniform usampler2D vga_text_buffer;
uniform usampler2D vga_char_buffer;
uniform sampler2D vga_color_buffer;

/*
 * Sample the VGA text buffer.
 */
uvec4 sample_tile(uint x, uint y) 
{
    return texelFetch(vga_text_buffer, ivec2(x, y), 0);
}

/*
 * Sample the VGA character buffer.
 *
 * @char_code   The char code
 * @x           The x offset inside a tile in pixels.
 * @y           The y offset inside a tile in pixels.
 */
uint sample_char(uint char_code, uint x, uint y)
{
    x = (CHAR_SIZE - 1u) - x; // flip horizontally 
    uint row = texelFetch(vga_char_buffer, ivec2(y, char_code), 0).r;
    return (row >> x) & 1u;
}

/*
 * Sample the VGA color buffer.
 */
vec4 sample_color(uint c) 
{
    return texelFetch(vga_color_buffer, ivec2(c, 0), 0);
}

/*
 * tile data consists of two bytes, one color/attribute byte and one
 * character byte:
 *
 * | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 * |   |           |               |                               |
 *   b       bg            fg                    char
 *
 *   b     =  blink character
 *   bg    =  background color
 *   fg    =  foreground color
 *   char  =  character code
 */
void main() 
{
    // figure out where we are
    uint pixel_x = uint(gl_FragCoord.x) - LEFT_MARGIN;
    uint pixel_y = VIEWPORT_HEIGHT - (uint(gl_FragCoord.y) - BOTTOM_MARGIN) - 1u; // swap y axis
    uint tile_x  = pixel_x / TILE_WIDTH_IN_PIXELS;
    uint tile_y  = pixel_y / TILE_HEIGHT_IN_PIXELS;
    uint char_x  = (pixel_x % TILE_WIDTH_IN_PIXELS) / PIXEL_RATIO_X;
    uint char_y  = (pixel_y % TILE_HEIGHT_IN_PIXELS) / PIXEL_RATIO_Y;

    // read the VGA CTRL register
    bool ctrl_blink_enable  = bool((vga_ctrl_register >> VGA_CTRL_BLINK_BIT) & 1);
    bool ctrl_color_inverse = bool((vga_ctrl_register >> VGA_CTRL_INVERSE_BIT) & 1);
    bool ctrl_monochrome    = bool((vga_ctrl_register >> VGA_CTRL_MONOCHROME_BIT) & 1);

    // color the pixel
    uvec4 tile      = sample_tile(tile_x, tile_y);
    uint char_code  = tile.r;
    vec4 fg_color   = sample_color((tile.g >> 0u) & 0x0Fu);
    vec4 bg_color   = sample_color((tile.g >> 4u) & 0x07u);
    uint blink      = tile.g >> 7u;

    // handle blink
    if (ctrl_blink_enable) {
        // handle blink bit set
        char_code = ((blink == 1u) && ((int(time*2) % 2)) == 1) ? 0x00u : char_code;
    } else {
        // with blink disabled, the blink bit acts as an extra color index bit
        bg_color = sample_color((tile.g >> 4u) & 0x0Fu);
    }

    // handle monochrome
    if (ctrl_monochrome) {
        // these colors look fine
        fg_color = vec4(0.820f, 0.820f, 0.631f, 1.0f);
        bg_color = vec4(0.020f, 0.059f, 0.133f, 1.0f);
    } 

    // handle inverse 
    if (ctrl_color_inverse) {
        vec4 tmp = fg_color;
        fg_color = bg_color;
        bg_color = tmp;
    }

    // color the pixel
    uint pixel = sample_char(char_code, char_x, char_y);
    frag_color = bg_color;
    if (pixel != 0u)
        frag_color = fg_color;
}

)""

