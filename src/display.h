#ifndef DISPLAY_H
#define DISPLAY_H

#include "m6502.h"

// TODO maybe some of these aren't needed
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <thread>

struct Vertex
{
    glm::vec3 pos;
    glm::vec2 uv;
};

struct Display 
{

    Display(const Cpu &cpu, const Memory &mem) : cpu(cpu), mem(mem) 
    {
        //int err = this->setup();
        //if (err != 0)
        //    exit(1);
    };
    ~Display();

    
    /*
     * Spawn a new thread and start rendering.
     */
    std::thread start();

private:

    /*
     * Sets up the opengl rendering context, compiles shaders, etc. Is called
     * by the constructor.
     */
    int setup();

    void setup_ntsc_encode_program();
    void setup_ntsc_decode_pass_a_program();
    void setup_ntsc_decode_pass_b_program();
    void setup_frambuffer_gen_program();
    void setup_post_process_program();
    
    /*
     * Returns 1 if program should terminate (i.e. user closes the window).
     */
    int loop();
    
    void use_ntsc_encode_program();
    void use_ntsc_decode_pass_a_program();
    void use_ntsc_decode_pass_b_program();
    void use_frambuffer_gen_program();
    void use_post_process_program();
    
    /*
     * We include this for monitoring with ImGui. Const because no cheating 
     * allowed.
     */
    const Cpu &cpu;

    /*
     * We read the VGA_* pages from memory to construct the image.
     */
    const Memory &mem;

    /*
     * OpenGL and GLFW related stuff
     */
    const char *pass_through_vert_shader_source =
        #include "shaders/pass_through.vert"
    ;
    const char *framebuffer_gen_frag_shader_source =
        #include "shaders/framebuffer_gen.frag"
    ;
    const char *ntsc_encode_frag_shader_source =
        #include "shaders/ntsc_encode.frag"
    ;
    const char *ntsc_decode_pass_a_frag_shader_source =
        #include "shaders/ntsc_decode_pass_a.frag"
    ;
    const char *ntsc_decode_pass_b_frag_shader_source =
        #include "shaders/ntsc_decode_pass_b.frag"
    ;
    const char *post_process_frag_shader_source =
        #include "shaders/post_process.frag"
    ;

    GLFWwindow *window;
    
    // Constants
    const unsigned int VGA_TEXT_COLUMNS = 80;
    const unsigned int VGA_TEXT_ROWS    = 25;
    const unsigned int VGA_CHAR_SIZE    =  8;
    const unsigned int VGA_N_CHARS      = Layout::VGA_CHAR_BUF_SIZE / VGA_CHAR_SIZE;
    const unsigned int RESOLUTION_X     = VGA_TEXT_COLUMNS * VGA_CHAR_SIZE;
    const unsigned int RESOLUTION_Y     = VGA_TEXT_ROWS * VGA_CHAR_SIZE;

    // programs
    unsigned int framebuffer_gen_program;
    unsigned int ntsc_encode_program;
    unsigned int ntsc_decode_pass_a_program;
    unsigned int ntsc_decode_pass_b_program;
    unsigned int post_process_program;

    // fbos
    unsigned int framebuffer_gen_fbo;
    
    // vbos
    unsigned int full_screen_tri_vbo; 

    //textures
    unsigned int vga_text_texture;
    unsigned int vga_char_texture;
    unsigned int framebuffer_tex_native;
    unsigned int framebuffer_tex_ntsc_active;
    unsigned int framebuffer_tex_ntsc_swap;

    // other uniforms
    //unsigned int display_width  = 1280;
    //unsigned int display_height =  800;

    // vaos
    unsigned int VAO;

    
};

#endif
