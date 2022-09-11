#ifndef IMGUI_LAYER_H
#define IMGUI_LAYER_H

#include "m6502.h"
#include "memory.h"

#include <GLFW/glfw3.h>

/*
 * We use this struct to pass information to and from the ImguiLayer.
 */
struct ImguiLayerInfo 
{
    bool show_mem_edit    = true;
    bool execution_paused = false; 
    bool step_execution   = false;
    bool reset_cpu        = false;
    float execution_speed = 10000.0f; // Hz
    //float execution_speed = 1000000.0f; // Hz
};

class ImguiLayer
{
public:
    ImguiLayer(const Cpu &cpu, const Memory &mem, ImguiLayerInfo *info) : cpu(cpu), mem(mem), info(info) {};
    void setup(GLFWwindow *window, const char *glsl_version) const;
    void draw();
    void shutdown() const;
private:
    const Cpu &cpu;
    const Memory &mem;
    ImguiLayerInfo *info;

    void draw_main_window() const;
};

#endif
