R""(
#version 330 core
#extension GL_ARB_arrays_of_arrays: enable
#extension GL_ARB_shading_language_420pack: enable

const uint CHAR_SIZE       =    8u;
const uint CHARSET_SIZE    =  128u;
const uint VIEWPORT_WIDTH  = 1280u;
const uint VIEWPORT_HEIGHT =  800u;
const uint TEXT_COLS       =   80u;
const uint TEXT_ROWS       =   25u;

const uint TILE_WIDTH_IN_PIXELS  = VIEWPORT_WIDTH / TEXT_COLS;
const uint TILE_HEIGHT_IN_PIXELS = VIEWPORT_HEIGHT / TEXT_ROWS;

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

out vec4 frag_color;

uniform usampler2D vga_text_buffer;
uniform usampler2D vga_char_buffer;

/*
 * Sample the VGA text buffer.
 *
 * @char_code   The char code
 * @x           The x offset inside a tile. Valid range: [0,7]
 * @y           The y offset inside a tile. Valid range: [0,7]
 */
uvec4 sample_tile(uint x, uint y) 
{
    return texture(
            vga_text_buffer,
            vec2(float(x) / float(TEXT_COLS), float(25u - y) / float(TEXT_ROWS)) +
            vec2(0.5f / float(TEXT_COLS), 0.5f / float(TEXT_ROWS))
    );
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
    x = TILE_WIDTH_IN_PIXELS - x; // flip horizontally 
    uint row = texture(
            vga_char_buffer,
            vec2(float(y) / float(TILE_HEIGHT_IN_PIXELS), float(char_code) / float(CHARSET_SIZE))// +
            //vec2(0.5f / float(CHAR_SIZE), 0.5f / float(CHARSET_SIZE))
    ).r;
    return (row >> ((x * 8u) / TILE_WIDTH_IN_PIXELS)) & 1u;
}

void main() 
{
    //uint xi = uint(VIEWPORT_WIDTH) - uint(gl_FragCoord.x);
    //uint yi = uint(VIEWPORT_HEIGHT) - uint(gl_FragCoord.y);
    //uint tile_width  = uint(VIEWPORT_WIDTH  / float(TEXT_COLS));
    //uint tile_height = uint(VIEWPORT_HEIGHT / float(TEXT_ROWS));
    //float x = gl_FragCoord.x / VIEWPORT_WIDTH;
    //float y = gl_FragCoord.y / VIEWPORT_HEIGHT;
    //uint tile_idx_x = uint(x * TEXT_COLS);
    //uint tile_idx_y = uint(y * TEXT_ROWS);
    //uint char_idx_x = uint(((xi % tile_width) / float(tile_width)) * 8.0f);
    //uint char_idx_y = uint(((yi % tile_height) / float(tile_height)) * 8.0f);


    uint pixel_x = uint(gl_FragCoord.x);
    uint pixel_y = VIEWPORT_HEIGHT - uint(gl_FragCoord.y); // swap y axis
    uint tile_x  = pixel_x / TILE_WIDTH_IN_PIXELS;
    uint tile_y  = pixel_y / TILE_HEIGHT_IN_PIXELS;
    uint char_x  = pixel_x % TILE_WIDTH_IN_PIXELS;
    uint char_y  = pixel_y % TILE_HEIGHT_IN_PIXELS;

    uvec4 tile      = sample_tile(tile_x, tile_y);
    uint char_code  = tile.r;
    vec4 fg_color   = palette[(tile.g >> 0u) & 0x0Fu];
    vec4 bg_color   = palette[(tile.g >> 4u) & 0x0Fu];
    uint pixel      = sample_char(char_code, char_x, char_y);
    
    //frag_color = /*bg_color +*/ vec4(float(char_y) / 8.0f, 0.0f, float(char_x) / 8.0f, 1.0f);
    frag_color = palette[0];//vec4(0.0f,0.0f,0.0f,1.0f);
    
    if (pixel != 0u)
        frag_color = bg_color;

//    if (char_x == 0u || char_y == 0u)
//        frag_color = vec4(1.0f,1.0f,1.0f,1.0f);
    
}

)""

