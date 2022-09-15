#include "imgui_layer.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "imgui_memory_editor/imgui_memory_editor.h"

static MemoryEditor mem_edit;

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

/*
 * TODO make this entire function less garbage.
 */
void ImguiLayer::draw_main_window() const
{
    static u16 last_frame_pc;

    bool &changed = this->info->changed;
    ImGui::Begin("6502-burken");

    // --- CPU status ---
    ImGui::Separator();
    ImGui::Text("CPU status:\n");
    ImGui::Text("ACC: 0x%02X\tX: 0x%02X\tY: 0x%02X", 
            this->cpu.ACC, 
            this->cpu.X, 
            this->cpu.Y
    );
    ImGui::Text("SP:  0x%02X", this->cpu.SP);
    ImGui::Text("PC:  0x%02X", this->cpu.PC);
    ImGui::Text("CARRY (C):    %d    ZERO (Z):    %d    IRQB (I):        %d    DECIMAL (D):    %d", 
            (bool)(this->cpu.SR & (1 << BIT_C)),
            (bool)(this->cpu.SR & (1 << BIT_Z)),
            (bool)(this->cpu.SR & (1 << BIT_I)),
            (bool)(this->cpu.SR & (1 << BIT_D))
    );
    ImGui::Text("BRK (B):      %d    UNUSED:      %d    OVERFLOW (V):    %d    NEGATIVE (N):   %d", 
            (bool)(this->cpu.SR & (1 << BIT_B)),
            (bool)(this->cpu.SR & (1 << BIT_UNUSED)),
            (bool)(this->cpu.SR & (1 << BIT_V)),
            (bool)(this->cpu.SR & (1 << BIT_N))
    );
   
    // --- program visualization ---
    ImGui::Separator();
    ImGui::Text("Program: TODO\n");
    ImGui::Text("Op at PC: %s\n", instruction_table[this->mem[this->cpu.PC]].mnemonic.c_str());

    // --- Execution speed control --- 
    ImGui::Separator();
    ImGui::Text("Simulation control:\n");
    if (ImGui::Button((this->info->execution_paused) ? "Resume" : "Pause")) {
        this->info->execution_paused = !this->info->execution_paused;
        changed = true;
    }
    ImGui::SameLine();

    if (ImGui::Button("Step")) {
        this->info->step_execution = true;
        changed = true;
    }
    ImGui::SameLine();
    if (ImGui::Button((this->info->follow_pc_on_step) ? "Follow PC on step: ON" : "Follow PC on step: OFF")) {
        this->info->follow_pc_on_step = !this->info->follow_pc_on_step;
        changed = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset CPU")) {
        this->info->reset_cpu = true;
        changed = true;
    }
    ImGui::SameLine();
    if (ImGui::Button((this->info->turbo_mode) ? "Turbo mode: ON" : "Turbo mode: OFF")) {
        this->info->turbo_mode = !this->info->turbo_mode;
        changed = true;
    }
    float f32_min = 5.0f;
    float f32_max = M6502Constants::CLOCK_SPEED_MAX;
    changed |= ImGui::SliderScalar("CPU speed:", ImGuiDataType_Float, &this->info->requested_clock_speed, &f32_min, &f32_max,  "", ImGuiSliderFlags_Logarithmic);
    show_freq(this->info->requested_clock_speed);
    ImGui::Text("Measured CPU clock speed: ");
    if (!this->info->execution_paused &&
            this->info->requested_clock_speed > 1000.0f &&
            !this->info->turbo_mode) {
        show_freq(this->info->measured_clock_speed);
    } else {
        ImGui::SameLine();
        ImGui::Text("-");
    }

    
    // --- Memory inspector ---
    ImGui::Separator();
    if (ImGui::Button(this->info->show_mem_edit ? "Hide memory editor" : "Show memory editor")) {
        this->info->show_mem_edit = !this->info->show_mem_edit; 
    }
    if (this->info->show_mem_edit) {
        ImGui::BeginChild("Memory editor");
        mem_edit.DrawContents(this->mem.data, Layout::MEM_SIZE, 0x0000);
        ImGui::EndChild();
    }
    if (this->info->follow_pc_on_step && 
            (this->info->execution_paused || this->info->requested_clock_speed < 1000.0f) &&
            this->cpu.PC != last_frame_pc) {
        mem_edit.GotoAddrAndHighlight(this->cpu.PC, this->cpu.PC);
    }

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
