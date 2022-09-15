#ifndef IMGUI_LAYER_H
#define IMGUI_LAYER_H

#include "m6502.h"
#include "memory.h"

#include <GLFW/glfw3.h>

/*
 * We use this struct to pass information to and from the ImguiLayer.
 *
 * TODO rename this
 */
struct ImguiLayerInfo 
{
    bool changed                     = true;
    bool show_mem_edit               = true;
    bool execution_paused            = true; 
    bool step_execution              = false;
    bool follow_pc_on_step           = false;
    bool reset_cpu                   = false;
    bool turbo_mode                  = false;
    float requested_clock_speed      = 10000.0f; // Hz
    float measured_clock_speed       = 0.0f;
    float frame_time; // ms
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
