#ifndef DISPLAY_H
#define DISPLAY_H

#include "layout.h"
#include "m6502.h"

// TODO maybe some of these aren't needed
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

struct Display 
{
    /*
     * We include this for monitoring with ImGui
     * const because no cheating allowed
     */
    const Cpu &cpu;

    /*
     * We read the DISPLAY_* pages from memory to
     * construct a framebuffer
     */
    const Memory &mem;

    Display(const Cpu &cpu, const Memory &mem) : cpu(cpu), mem(mem) 
    {
        int err = this->setup();
        if (err != 0)
            exit(1);
    };
    ~Display();

    int update();

private:
    GLFWwindow *window;
    int setup();
    const char *vert_shader_source =
        #include "shaders/vert_shader.glsl"
    ;
    const char *frag_shader_source =
        #include "shaders/frag_shader.glsl"
    ;
};

#endif
