#ifndef IMGUI_LAYER_H
#define IMGUI_LAYER_H

#include "m6502.h"
#include "memory.h"

#include <GLFW/glfw3.h>

class ImguiLayer
{
public:
    ImguiLayer(const Cpu &cpu, const Memory &mem) : cpu(cpu), mem(mem) {};
    void setup(GLFWwindow *window, const char *glsl_version) const;
    void draw() const;
    void shutdown() const;
private:
    const Cpu &cpu;
    const Memory &mem;
};

#endif
