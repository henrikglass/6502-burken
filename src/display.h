#ifndef DISPLAY_H
#define DISPLAY_H

#include "m6502.h"

// TODO maybe some of these aren't needed
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <thread>

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
    
    /*
     * Returns 1 if program should terminate (i.e. user closes the window).
     */
    int loop();

    /*
     * We include this for monitoring with ImGui. Const because no cheating 
     * allowed.
     */
    const Cpu &cpu;

    /*
     * We read the VGA_* pages from memory to construct the image.
     */
    const Memory &mem;


    const unsigned int VGA_TEXT_COLUMNS = 80;
    const unsigned int VGA_TEXT_ROWS    = 25;
    const unsigned int VGA_CHAR_SIZE    =  8;
    const unsigned int VGA_N_CHARS      = Layout::VGA_CHAR_BUF_SIZE / VGA_CHAR_SIZE;

    /*
     * OpenGL and GLFW related stuff
     */
    const char *vert_shader_source =
        #include "shaders/vert_shader.glsl"
    ;
    const char *frag_shader_source =
        #include "shaders/frag_shader.glsl"
    ;

    GLFWwindow *window;

    unsigned int full_screen_tri_vbo; 
    unsigned int vga_text_ubo;
    unsigned int vga_char_ubo;
    unsigned int vga_text_texture;
    unsigned int vga_char_texture;
    unsigned int VAO;

    
};

#endif
