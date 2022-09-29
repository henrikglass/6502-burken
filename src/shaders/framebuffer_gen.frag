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
const uint N_SPRITES       =    5u;

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
const uint VGA_CTRL_SPRITE1_ENABLE = 3u;
const uint VGA_CTRL_SPRITE2_ENABLE = 4u;
const uint VGA_CTRL_SPRITE3_ENABLE = 5u;
const uint VGA_CTRL_SPRITE4_ENABLE = 6u;
const uint VGA_CTRL_SPRITE5_ENABLE = 7u;

const uint VGA_SPRITE_COLOR_OFFSET = 0x20u;
const uint VGA_SPRITE_POS_X_OFFSET = 0x21u;
const uint VGA_SPRITE_POS_Y_OFFSET = 0x23u;

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
uniform usampler2D vga_sprite_buffer;

/*
 * Sample the VGA text buffer.
 *
 * @x           The x offset in number of tiles [0, 79].
 * @y           The y offset in number of tiles [0, 24].
 */
uvec4 sample_tile(uint x, uint y) 
{
    return texelFetch(vga_text_buffer, ivec2(x, y), 0);
}

/*
 * Sample the VGA character buffer.
 *
 * @char_code   The char code
 * @x           The x offset inside a tile in pixels [0, 7].
 * @y           The y offset inside a tile in pixels [0, 7].
 */
uint sample_char(uint char_code, uint x, uint y)
{
    x = (CHAR_SIZE - 1u) - x; // flip horizontally 
    uint row = texelFetch(vga_char_buffer, ivec2(y, char_code), 0).r;
    return (row >> x) & 1u;
}

/*
 * Sample the VGA color buffer.
 *
 * @c   The color index [0, 15]
 */
vec4 sample_color(uint c) 
{
    return texelFetch(vga_color_buffer, ivec2(c, 0), 0);
}

/*
 * Sample the VGA sprite buffer.
 *
 * @sprite_nr   The sprite number
 * @x           The x offset inside a sprite in pixels [0, 15].
 * @y           The y offset inside a sprite in pixels [0, 15].
 */
uint sample_sprite(uint sprite_nr, uint x, uint y) 
{
    uint tile_x = x / 8u;
    uint tile_y = y / 8u;
    uint pixel_x = (CHAR_SIZE - 1u) - (x % 8u); // flip horizontally 
    uint pixel_y = y % 8u;
    uint row_idx = 2u * 8u * tile_y + 8u * tile_x + pixel_y;
    uint row = texelFetch(vga_sprite_buffer, ivec2(row_idx, sprite_nr), 0).r;
    return (row >> pixel_x) & 1u;
}

/*
 * tile data consists of two bytes, one color/attribute byte and one
 * character byte:
 *
 *             high byte                       low byte
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

    // handle sprites
    int vga_ctrl = vga_ctrl_register;
    for (uint sprite_nr = 0u; sprite_nr < N_SPRITES; sprite_nr++) {
        
        // skip sprites that are not enabled through the vga ctrl register
        if (!bool((vga_ctrl >> (3u + sprite_nr)) & 1)) 
            continue;

        // read the sprite footer data
        uint color    = texelFetch(vga_sprite_buffer, ivec2(VGA_SPRITE_COLOR_OFFSET,      sprite_nr), 0).r;
        uint pos_x_ll = texelFetch(vga_sprite_buffer, ivec2(VGA_SPRITE_POS_X_OFFSET,      sprite_nr), 0).r;
        uint pos_x_hh = texelFetch(vga_sprite_buffer, ivec2(VGA_SPRITE_POS_X_OFFSET + 1u, sprite_nr), 0).r;
        uint pos_y_ll = texelFetch(vga_sprite_buffer, ivec2(VGA_SPRITE_POS_Y_OFFSET,      sprite_nr), 0).r;
        uint pos_y_hh = texelFetch(vga_sprite_buffer, ivec2(VGA_SPRITE_POS_Y_OFFSET + 1u, sprite_nr), 0).r;

        // figure out the sprite position
        int pos_x = int((pos_x_hh << 8) + pos_x_ll);
        int pos_y = int((pos_y_hh << 8) + pos_y_ll);

        // figure out this fragments position in 6502-space (620x200, border margins, etc.)
        int pixel_pos_x = int(pixel_x / PIXEL_RATIO_X);
        int pixel_pos_y = int(pixel_y / PIXEL_RATIO_Y);

        // transform current screen-relative fragment position into sprite-relative position
        int pos_in_sprite_x = pixel_pos_x - pos_x; 
        int pos_in_sprite_y = pixel_pos_y - pos_y; 
        
        // color pixel if inside sprite
        if (pos_in_sprite_x >= 0 && pos_in_sprite_y >= 0 && 
                pos_in_sprite_x < 16 && pos_in_sprite_y < 16) {
            pixel = sample_sprite(0u, uint(pos_in_sprite_x), uint(pos_in_sprite_y));
            if (pixel != 0u)
                frag_color = sample_color(color & 0x0Fu);
        }

    }
    //uint xx = (pixel_x / PIXEL_RATIO_X) - 320u;
    //uint yy = (pixel_y / PIXEL_RATIO_Y) - 100u;
    //if (xx >= 0u && yy >= 0u && xx < 16u && yy < 16u) {
    //    pixel = sample_sprite(0u, xx, yy);
    //    if (pixel != 0u)
    //        frag_color = vec4(0.8, 0.8, 0.8, 1.0);
    //}
    
}

)""

