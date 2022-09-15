#include "imgui_layer.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "imgui_memory_editor/imgui_memory_editor.h"

static MemoryEditor mem_edit;

static u16 last_frame_pc;

void show_freq(float freq)
{
    ImGui::SameLine();
    if (freq >= 100000000.0f) {
        ImGui::Text("%.3f GHz", freq / 1000000.0f);
    } else if (freq >= 100000.0f) {
        ImGui::Text("%.3f MHz", freq / 1000000.0f);
    } else if (freq >= 100.0f) {
        ImGui::Text("%.3f KHz", freq / 1000.0f);
    } else {
        ImGui::Text("%.3f Hz", freq);
    }
}

void show_cpu_stats(const Cpu *cpu)
{
    ImGui::Separator();
    ImGui::Text("CPU status:\n");
    ImGui::Text("ACC: 0x%02X\tX: 0x%02X\tY: 0x%02X", 
            cpu->ACC, 
            cpu->X, 
            cpu->Y
    );
    ImGui::Text("SP:  0x%02X", cpu->SP);
    ImGui::Text("PC:  0x%02X", cpu->PC);
    ImGui::Text("CARRY (C):    %d    ZERO (Z):    %d    IRQB (I):        %d    DECIMAL (D):    %d", 
            (bool)(cpu->SR & (1 << BIT_C)),
            (bool)(cpu->SR & (1 << BIT_Z)),
            (bool)(cpu->SR & (1 << BIT_I)),
            (bool)(cpu->SR & (1 << BIT_D))
    );
    ImGui::Text("BRK (B):      %d    UNUSED:      %d    OVERFLOW (V):    %d    NEGATIVE (N):   %d", 
            (bool)(cpu->SR & (1 << BIT_B)),
            (bool)(cpu->SR & (1 << BIT_UNUSED)),
            (bool)(cpu->SR & (1 << BIT_V)),
            (bool)(cpu->SR & (1 << BIT_N))
    );
}

void show_simulation_control(ImguiLayerInfo *info)
{
    ImGui::Separator();
    ImGui::Text("Simulation control:\n");
    if (ImGui::Button(info->execution_paused ? "Resume" : "Pause")) {
        info->execution_paused = !info->execution_paused;
        info->changed = true;
    }
    ImGui::SameLine();

    if (ImGui::Button("Step")) {
        info->step_execution = true;
        info->changed = true;
    }
    ImGui::SameLine();
    if (ImGui::Button((info->follow_pc_on_step) ? "Follow PC on step: ON" : "Follow PC on step: OFF")) {
        info->follow_pc_on_step = !info->follow_pc_on_step;
        info->changed = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset CPU")) {
        info->reset_cpu = true;
        info->changed = true;
    }
    ImGui::SameLine();
    if (ImGui::Button((info->turbo_mode) ? "Turbo mode: ON" : "Turbo mode: OFF")) {
        info->turbo_mode = !info->turbo_mode;
        info->changed = true;
    }
    float f32_min = 5.0f;
    float f32_max = M6502Constants::CLOCK_SPEED_MAX;
    info->changed |= ImGui::SliderScalar("CPU speed:", ImGuiDataType_Float, &info->requested_clock_speed, &f32_min, &f32_max,  "", ImGuiSliderFlags_Logarithmic);
    show_freq(info->requested_clock_speed);
    ImGui::Text("Measured CPU clock speed: ");
    if (!info->execution_paused &&
            info->requested_clock_speed > 1000.0f &&
            !info->turbo_mode) {
        show_freq(info->measured_clock_speed);
    } else {
        ImGui::SameLine();
        ImGui::Text("-");
    }
}

void show_mem_editor(const Memory *mem, const Cpu *cpu, ImguiLayerInfo *info)
{
    ImGui::Separator();
    if (ImGui::Button(info->show_mem_edit ? "Hide memory editor" : "Show memory editor")) {
        info->show_mem_edit = !info->show_mem_edit; 
        info->changed = true;
    }
    if (info->show_mem_edit) {
        ImGui::BeginChild("Memory editor");
        mem_edit.DrawContents(mem->data, Layout::MEM_SIZE, 0x0000);
        ImGui::EndChild();
    }
    if (info->follow_pc_on_step && 
            (info->execution_paused || info->requested_clock_speed < 1000.0f) &&
            cpu->PC != last_frame_pc) {
        mem_edit.GotoAddrAndHighlight(cpu->PC, cpu->PC);
    }
}

void show_disassmbler(const Memory *mem, const Cpu *cpu, ImguiLayerInfo *info)
{
    ImGui::Separator();
    ImGui::Text("Disassembler: TODO\n");
    ImGui::Text("Op at PC: %s\n", instruction_table[(*mem)[cpu->PC]].mnemonic.c_str());
}

/*
 * TODO make this entire function less garbage.
 */
void ImguiLayer::draw_main_window() const
{
    ImGui::Begin("6502-burken");

    ImGui::Text("Fps: %f", this->info->frames_per_second);

    show_cpu_stats(&this->cpu);

    // --- program visualization ---
    show_disassmbler(&this->mem, &this->cpu, this->info);

    // --- Execution speed control --- 
    show_simulation_control(this->info);
    
    // --- Memory inspector ---
    show_mem_editor(&this->mem, &this->cpu, this->info);

    ImGui::End();

    // some logic
    last_frame_pc = this->cpu.PC;
}

void ImguiLayer::setup(GLFWwindow *window, const char *glsl_version) const
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    //ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

void ImguiLayer::draw()
{
    // begin new frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // --- draw GUI! ---
    this->draw_main_window();

    // render
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImguiLayer::shutdown() const
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
