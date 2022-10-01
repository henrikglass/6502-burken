#include "imgui_layer.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "imgui_memory_editor/imgui_memory_editor.h"
#include "util.h"

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

void show_simulation_control(UiInfo *info)
{
    ImGui::Separator();
    ImGui::Text("Simulation control:\n");
    if (ImGui::Button(info->execution_paused ? "Resume" : "Pause")) {
        info->execution_paused = !info->execution_paused;
        info->step_execution = true;
        info->changed = true;
    }
    ImGui::SameLine();

    if (ImGui::Button("Step")) {
        info->step_execution = true;
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

void show_mem_editor(const Memory *mem, const Cpu *cpu, UiInfo *info)
{
    ImGui::Separator();
    ImGui::Text("Memory editor:\n");
    if (ImGui::Button(info->show_mem_edit ? "Hide memory editor" : "Show memory editor")) {
        info->show_mem_edit = !info->show_mem_edit; 
        info->changed = true;
    }
    ImGui::SameLine();
    if (ImGui::Button((info->memedit_follow_pc) ? "Follow PC on step: ON" : "Follow PC on step: OFF")) {
        info->memedit_follow_pc = !info->memedit_follow_pc;
        info->changed = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Jump to PC")) {
        mem_edit.GotoAddrAndHighlight(cpu->PC, cpu->PC);
    }
    ImGui::SameLine();
    if (ImGui::Button("Jump to SP")) {
        mem_edit.GotoAddrAndHighlight(cpu->SP, cpu->SP);
    }
    if (info->show_mem_edit) {
        ImGui::BeginChild("Memory editor");
        mem_edit.DrawContents(mem->data, Layout::MEM_SIZE, 0x0000);
        ImGui::EndChild();
    }
    if (info->memedit_follow_pc && 
            (info->execution_paused || info->requested_clock_speed < 100.0f /*hz*/) &&
            cpu->PC != last_frame_pc) {
        mem_edit.GotoAddrAndHighlight(cpu->PC, cpu->PC);
    }
}

#include <iostream>

void show_disassmbler(const Cpu *cpu, Disassembler *disasm, UiInfo *info)
{
    const int DISASSEMBLER_CONTENT_HEIGHT = 275;
    const int DISASSEMBLER_SCROLL_MARGIN = 25; 
    
    static u16 disassembly_start = 0x8000;
    static u16 disassembly_end   = 0xFFFF;
    static char textinput_start[5] = ""; 
    static char textinput_end[5] = ""; 
    
    auto padding = [](int n){
        return std::string(3 * n, ' ').c_str();
    };

    ImGui::Separator();
    ImGui::Text("Live disassembler:\n");
    if (ImGui::Button(info->show_disasm ? "Hide live disassembler" : "Show live disassembler")) {
        info->show_disasm = !info->show_disasm; 
        info->changed = true;
    }
    
    ImGui::SameLine();
    if (ImGui::Button(info->disasm_follow_pc ? "Follow PC: ON" : "Follow PC: OFF")) {
        info->disasm_follow_pc = !info->disasm_follow_pc; 
        info->changed = true;
    }
    

    if (!info->show_disasm) {
        return;
    }

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
    ImGui::BeginChild("Live disassembler", {0, DISASSEMBLER_CONTENT_HEIGHT}, false, window_flags);
       
    auto instrs = disasm->get_disassembly(disassembly_start, disassembly_end);
    int row = 0;
    int tgt = 0;
    for (auto *instr : instrs) {
        for (u16 bp : info->breakpoints) {
            if (bp == instr->addr) {
                if (info->breakpoints_enabled)
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), ">>>");
                else
                    ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.3f, 1.0f), ">>>");
                ImGui::SameLine();
            }
        }
        if (instr->addr == cpu->PC) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "> 0x%04X:   %s%s  %s <", 
                    instr->addr, 
                    instr->bytes_str.c_str(), 
                    padding(3 - instr->len),
                    instr->assembly_str.c_str());
            tgt = row;
        } else {
            ImGui::Text("  0x%04X:   %s%s  %s", 
                    instr->addr, 
                    instr->bytes_str.c_str(), 
                    padding(3 - instr->len),
                    instr->assembly_str.c_str());
        }
        row++;
    }

    if (info->disasm_follow_pc) {
        float scroll_max = ImGui::GetScrollMaxY();
        float own_scroll_pos = ImGui::GetScrollY();
        float tgt_scroll_pos = float(tgt)/float(row) * (scroll_max + DISASSEMBLER_CONTENT_HEIGHT);
        
        if (own_scroll_pos + DISASSEMBLER_CONTENT_HEIGHT < tgt_scroll_pos + DISASSEMBLER_SCROLL_MARGIN/2 ||
                own_scroll_pos > tgt_scroll_pos) {
            ImGui::SetScrollY(tgt_scroll_pos - DISASSEMBLER_SCROLL_MARGIN);
        }
    }

    ImGui::EndChild();

    // filtered text input
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_CharsHexadecimal   | 
                                ImGuiInputTextFlags_EnterReturnsTrue   | 
                                ImGuiInputTextFlags_AutoSelectAll      | 
                                ImGuiInputTextFlags_NoHorizontalScroll;

    ImGui::Separator();
    if (ImGui::Button("Set range")) {
        std::cout << "a" << std::endl;
        u16 start = Util::hex_to_u16(textinput_start);
        u16 end   = Util::hex_to_u16(textinput_end);
        if (start < end) {
            std::cout << "b" << std::endl;
            disassembly_start = start;
            disassembly_end   = end;
        }
    }
    ImGui::SameLine();
    ImGui::PushItemWidth(50);
    ImGui::InputText("start", textinput_start, 5, flags);
    ImGui::SameLine();
    ImGui::InputText("end",   textinput_end,   5, flags);
    ImGui::PopItemWidth();
}

void show_breakpoints(Disassembler *disasm, UiInfo *info)
{
    static char textinput[5] = ""; 
    
    ImGui::Separator();
    ImGui::Text("Breakpoints");
    if (!ImGui::BeginListBox(" BP", ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing())))
        return; 
   
    static size_t selected_item_idx = 0;

    for (size_t i = 0; i < info->breakpoints.size(); i++) {
        const bool is_selected = (selected_item_idx == i);
        u16 bp_addr = info->breakpoints[i];
        std::string instr = Util::int_to_hex(bp_addr) + ":  " + disasm->disassemble_instruction(bp_addr).assembly_str;
        if (ImGui::Selectable(instr.c_str(), is_selected))
            selected_item_idx = i;

        if (is_selected)
            ImGui::SetItemDefaultFocus();
    }

    ImGui::EndListBox();

    // filtered text input
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_CharsHexadecimal   | 
                                ImGuiInputTextFlags_EnterReturnsTrue   | 
                                ImGuiInputTextFlags_AutoSelectAll      | 
                                ImGuiInputTextFlags_NoHorizontalScroll;
    bool add_breakpoint = ImGui::InputText("New breakpoint address", textinput, 5, flags);
    
    add_breakpoint = add_breakpoint | ImGui::Button("Add Breakpoint");
    
    // add breakpoint button
    if (add_breakpoint) {
        u16 new_bp = Util::hex_to_u16(textinput);
        auto it = std::upper_bound(info->breakpoints.cbegin(), info->breakpoints.cend(), new_bp);
        info->breakpoints.insert(it, new_bp);
    }

    
    // delete breakpoint button
    ImGui::SameLine();
    if (ImGui::Button("Delete Breakpoint")) {
        if (selected_item_idx >= 0 && selected_item_idx < info->breakpoints.size()) {
            info->breakpoints.erase(info->breakpoints.begin() + selected_item_idx);
        }
    }

    ImGui::SameLine();
    if (ImGui::Button((info->breakpoints_enabled) ? "Breakpoints: Enabled" : "Breakpoints: Disabled")) {
        info->breakpoints_enabled = !info->breakpoints_enabled;
    }

}

/*
 * TODO make this entire function less garbage.
 */
void ImguiLayer::draw_main_window() const
{
    ImGui::Begin("6502-burken");

    ImGui::Text("Fps: %3.1f", this->info->frames_per_second);

    // --- CPU registers & state ---
    show_cpu_stats(&this->cpu);

    // --- Breakpoints ---
    show_breakpoints(this->disasm, this->info);

    // --- program visualization ---
    show_disassmbler(&this->cpu, this->disasm, this->info);

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
    //ImGui::ShowDemoWindow();
    this->draw_main_window();

    // render
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool ImguiLayer::want_capture_io() const
{
    ImGuiIO& io = ImGui::GetIO();
    return io.WantCaptureKeyboard || io.WantCaptureMouse;
}

void ImguiLayer::shutdown() const
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
