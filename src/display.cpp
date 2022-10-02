#include "display.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <vector>

GLenum glCheckError_(const char *file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__) 
#define diva_error_check() if(glCheckError_(__FILE__, __LINE__) != GL_NO_ERROR) exit(1)

// default screen size
unsigned int display_width  = 1920;
unsigned int display_height = 1080;

std::chrono::time_point<std::chrono::high_resolution_clock> timer_start;
float timer_current;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    display_width  = width;
    display_height = height;
}

unsigned int make_shader_program(const char *vert_shader_src, const char *frag_shader_src)
{
    unsigned int vert_shader = glCreateShader(GL_VERTEX_SHADER);
    unsigned int frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    unsigned int shader_program = glCreateProgram();
    glShaderSource(vert_shader, 1, &vert_shader_src, nullptr);
    glShaderSource(frag_shader, 1, &frag_shader_src, nullptr);
    glCompileShader(vert_shader);
    glCompileShader(frag_shader);
    glAttachShader(shader_program, vert_shader);
    glAttachShader(shader_program, frag_shader);
    glLinkProgram(shader_program);
    
    // check for compile errors
    char log[512];
    int vert_success, frag_success, link_success;
    glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &vert_success);
    glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &frag_success);
    glGetProgramiv(shader_program, GL_LINK_STATUS, &link_success);
    if (!(vert_success & frag_success & link_success))
    {
        glGetShaderInfoLog(vert_shader, 512, nullptr, log);
        std::cout << "Vertex shader error:\n" << log << std::endl;
        glGetShaderInfoLog(frag_shader, 512, nullptr, log);
        std::cout << "Fragment shader error:\n" << log << std::endl;
        glGetProgramInfoLog(shader_program, 512, nullptr, log);
        std::cout << "Linking error:\n" << log << std::endl;
        exit(1); // TODO handle these in a better way
    }

    // delete shaders
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);

    diva_error_check();

    return shader_program;
}

void Display::setup_frambuffer_gen_program()
{
    glUseProgram(this->framebuffer_gen_program);

    // setup textures to hold vga text buffer, char buffer, color palette,
    // and sprite buffer.
    glGenTextures(1, &this->vga_text_texture);
    glGenTextures(1, &this->vga_char_texture);
    glGenTextures(1, &this->vga_color_texture);
    glGenTextures(1, &this->vga_sprite_buffer_texture);

    // configure vga text buffer texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->vga_text_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    this->update_vga_text_buffer_texture();
   
    // configure vga char buffer texture
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, this->vga_char_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    this->update_vga_char_buffer_texture();
    
    // configure vga color buffer texture
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, this->vga_color_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    this->update_vga_color_buffer_texture();
    
    // configure vga sprite buffer texture
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, this->vga_sprite_buffer_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    this->update_vga_sprite_buffer_texture();
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Associate texture units with corresponding sampler
    glUniform1i(glGetUniformLocation(this->framebuffer_gen_program, "vga_text_buffer"), 0);
    glUniform1i(glGetUniformLocation(this->framebuffer_gen_program, "vga_char_buffer"), 1);
    
    // setup framebuffer for rendering to
    glGenFramebuffers(1, &this->offscreen_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, this->offscreen_fbo);

    glGenTextures(1, &this->framebuffer_tex_native);
    glGenTextures(1, &this->framebuffer_tex_ntsc_active);
    glGenTextures(1, &this->framebuffer_tex_ntsc_swap);

    // 640x200 is the native resolution (80*8, and 25*8)
    // 660x210 is with 10 pixel margins on sides and 5 on 
    // top and bottom.
    glBindTexture(GL_TEXTURE_2D, this->framebuffer_tex_native);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 660, 210, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    // --- DEBUG --- 
    // Load Test Card into native framebuffer texture
    //int width, height, nrChannels;
    //stbi_set_flip_vertically_on_load(true);
    //unsigned char *data = stbi_load("extra/images/testcard3.png", &width, &height, &nrChannels, 0);
    //switch (nrChannels) {
    //    case 3: 
    //        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    //        break;
    //    case 4: 
    //        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    //        break;
    //}

    // 1280x800 seems to look good.
    glBindTexture(GL_TEXTURE_2D, this->framebuffer_tex_ntsc_active);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1920, 1080, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glBindTexture(GL_TEXTURE_2D, this->framebuffer_tex_ntsc_swap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1920, 1080, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->framebuffer_tex_native, 0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
	    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
        exit(1);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    diva_error_check();
}

void Display::setup_ntsc_encode_program()
{
    glUseProgram(this->ntsc_encode_program);
    glUniform1i(glGetUniformLocation(this->ntsc_encode_program, "frame_buffer_texture"), 0);
    
    diva_error_check();
}

void Display::setup_ntsc_decode_pass_a_program()
{
    glUseProgram(this->ntsc_decode_pass_a_program);
    glUniform1i(glGetUniformLocation(this->ntsc_decode_pass_a_program, "frame_buffer_texture"), 0);
    
    diva_error_check();
}

void Display::setup_ntsc_decode_pass_b_program()
{
    glUseProgram(this->ntsc_decode_pass_b_program);
    glUniform1i(glGetUniformLocation(this->ntsc_decode_pass_b_program, "frame_buffer_texture"), 0);
    
    diva_error_check();
}

void Display::setup_post_process_program()
{
    glUseProgram(this->post_process_program);
    glUniform1i(glGetUniformLocation(this->post_process_program, "frame_buffer_texture"), 0);
    
    diva_error_check();
}

void Display::use_frambuffer_gen_program()
{
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->framebuffer_tex_native, 0);
    glUseProgram(this->framebuffer_gen_program);

    // update uniforms
    glUniform1i(glGetUniformLocation(this->framebuffer_gen_program, "vga_ctrl_register"), this->mem[Layout::VGA_CTRL]);
    glUniform1f(glGetUniformLocation(this->framebuffer_gen_program, "time"), glfwGetTime());
    glUniform1i(glGetUniformLocation(this->framebuffer_gen_program, "vga_text_buffer"), 0);
    glUniform1i(glGetUniformLocation(this->framebuffer_gen_program, "vga_char_buffer"), 1);
    glUniform1i(glGetUniformLocation(this->framebuffer_gen_program, "vga_color_buffer"), 2);
    glUniform1i(glGetUniformLocation(this->framebuffer_gen_program, "vga_sprite_buffer"), 3);
    
    // update textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->vga_text_texture);
    this->update_vga_text_buffer_texture();

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, this->vga_char_texture);
    this->update_vga_char_buffer_texture();
    
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, this->vga_color_texture);
    this->update_vga_color_buffer_texture();
    
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, this->vga_sprite_buffer_texture);
    this->update_vga_sprite_buffer_texture();
    
    glClearColor(0.7f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT); 
}

void Display::use_ntsc_encode_program() 
{
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->framebuffer_tex_ntsc_active, 0);

    glUseProgram(this->ntsc_encode_program);
    glUniform1i(glGetUniformLocation(this->ntsc_encode_program, "frame_buffer_texture"), 0);
    glUniform1f(glGetUniformLocation(this->ntsc_encode_program, "time"), glfwGetTime());
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->framebuffer_tex_native);
    
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT); 
}

void Display::use_ntsc_decode_pass_a_program() 
{
    std::swap(this->framebuffer_tex_ntsc_active, this->framebuffer_tex_ntsc_swap);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->framebuffer_tex_ntsc_active, 0);

    glUseProgram(this->ntsc_decode_pass_a_program);
    glUniform1i(glGetUniformLocation(this->ntsc_decode_pass_a_program, "frame_buffer_texture"), 0);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->framebuffer_tex_ntsc_swap);
    
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT); 
}

void Display::use_ntsc_decode_pass_b_program() 
{
    std::swap(this->framebuffer_tex_ntsc_active, this->framebuffer_tex_ntsc_swap);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->framebuffer_tex_ntsc_active, 0);

    glUseProgram(this->ntsc_decode_pass_b_program);
    glUniform1i(glGetUniformLocation(this->ntsc_decode_pass_b_program, "frame_buffer_texture"), 0);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->framebuffer_tex_ntsc_swap);
    
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT); 
}

void Display::use_post_process_program() 
{
    std::swap(this->framebuffer_tex_ntsc_active, this->framebuffer_tex_ntsc_swap);

    glUseProgram(this->post_process_program);

    glUniform1i(glGetUniformLocation(this->post_process_program, "frame_buffer_texture"), 0);
    glUniform1ui(glGetUniformLocation(this->post_process_program, "display_width"),  display_width);
    glUniform1ui(glGetUniformLocation(this->post_process_program, "display_height"), display_height);
    glUniform1f(glGetUniformLocation(this->post_process_program, "time"), glfwGetTime());
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->framebuffer_tex_ntsc_swap);
    
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT); 
}

void Display::update_vga_text_buffer_texture()
{
    // maybe only do this when this memory is changed except for every frame, 
    // or figure out another better way.
    glActiveTexture(GL_TEXTURE0);
    glTexImage2D(
            GL_TEXTURE_2D, 
            0, 
            GL_RG8UI, // GL_RG8. R --> char byte. G --> color byte.
            VGA_TEXT_COLUMNS,
            VGA_TEXT_ROWS, 
            0, 
            GL_RG_INTEGER, 
            GL_UNSIGNED_BYTE, 
            this->mem.data + Layout::VGA_TEXT_BUF_LOW
    ); 
}

void Display::update_vga_char_buffer_texture()
{
    // maybe only do this when this memory is changed except for every frame, 
    // or figure out another better way.
    // configure vga char buffer texture
    glActiveTexture(GL_TEXTURE1);
    glTexImage2D(
            GL_TEXTURE_2D, 
            0, 
            GL_R8UI, // GL_RG8. R --> a row in a character.
            VGA_CHAR_SIZE,
            VGA_N_CHARS, 
            0, 
            GL_RED_INTEGER, 
            GL_UNSIGNED_BYTE, 
            this->mem.data + Layout::VGA_CHAR_BUF_LOW
    ); 
}

void Display::update_vga_color_buffer_texture()
{
    glActiveTexture(GL_TEXTURE2);
    glTexImage2D(
            GL_TEXTURE_2D, 
            0, 
            GL_RGB, // GL_RGB8. R, G, B --> actually colors this time.
            VGA_N_COLORS,  // 16
            1,
            0, 
            GL_RGB, 
            GL_UNSIGNED_BYTE, 
            this->mem.data + Layout::VGA_COLOR_BUF_LOW
    ); 
}

void Display::update_vga_sprite_buffer_texture()
{
    glActiveTexture(GL_TEXTURE3);
    glTexImage2D(
            GL_TEXTURE_2D, 
            0, 
            GL_R8UI,
            VGA_SPRITE_SIZE,
            VGA_N_SPRITES,
            0, 
            GL_RED_INTEGER, 
            GL_UNSIGNED_BYTE, 
            this->mem.data + Layout::VGA_SPRITE_BUF_LOW
    ); 
}

Display::~Display() {
    if (this->imgui_layer != nullptr)
        imgui_layer->shutdown();
    glfwTerminate();
} 

int Display::setup() 
{
    // glfw setup. Apparently vmware is only good enough for version 3.3
    glfwInit();
    //const char *glsl_version = "#version 330 core";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    this->window = glfwCreateWindow(display_width, display_height, "6502-burken", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSwapInterval(1); // 0 = vsync off, 1 = vsync on

    // glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, display_width, display_height);
   
    this->framebuffer_gen_program = make_shader_program(
        this->pass_through_vert_shader_source,
        this->framebuffer_gen_frag_shader_source
    );
    
    this->ntsc_encode_program = make_shader_program(
        this->pass_through_vert_shader_source,
        this->ntsc_encode_frag_shader_source
    );
    
    this->ntsc_decode_pass_a_program = make_shader_program(
        this->pass_through_vert_shader_source,
        this->ntsc_decode_pass_a_frag_shader_source
    );
    
    this->ntsc_decode_pass_b_program = make_shader_program(
        this->pass_through_vert_shader_source,
        this->ntsc_decode_pass_b_frag_shader_source
    );
    
    this->post_process_program = make_shader_program(
        this->pass_through_vert_shader_source,
        this->post_process_frag_shader_source
    );

    // misc. enables, disables, & other settings
    glClearColor(0.7f, 0.0f, 0.0f, 1.0f);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // setup VAO
    glGenVertexArrays(1, &this->VAO);
    glBindVertexArray(this->VAO);

    // setup full screen static quad 
    glGenBuffers(1, &(this->full_screen_tri_vbo));
    glBindBuffer(GL_ARRAY_BUFFER, this->full_screen_tri_vbo);
    Vertex vertices[] = {
        {glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
        {glm::vec3( 3.0f, -1.0f, 0.0f), glm::vec2(2.0f, 0.0f)},
        {glm::vec3(-1.0f,  3.0f, 0.0f), glm::vec2(0.0f, 2.0f)}
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), (void *)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(Vertex), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    this->setup_frambuffer_gen_program();
    this->setup_ntsc_encode_program();
    this->setup_ntsc_decode_pass_a_program();
    this->setup_ntsc_decode_pass_b_program();
    this->setup_post_process_program();

    diva_error_check();
    
    return 0;
}

int Display::loop()
{
    // ----------------- misc. ----------------

    if (glfwWindowShouldClose(this->window)) {
        glfwTerminate();
        return 0;
    }
    
    glfwSwapBuffers(this->window);
    glfwPollEvents();

    glBindVertexArray(this->VAO);
    
    // ---------------- redraw ----------------

    // --- BIND "WORKING" FRAMEBUFFER ---
    glBindFramebuffer(GL_FRAMEBUFFER, this->offscreen_fbo);

    // FIRST PASS: generate the "native framebuffer" image
    this->use_frambuffer_gen_program();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    
    // SECOND PASS: encode ntsc color
    this->use_ntsc_encode_program();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    
    // THIRD PASS:  decode ntsc color (part 1)
    this->use_ntsc_decode_pass_a_program();
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // FOURTH PASS: decode ntsc color (part 2)
    this->use_ntsc_decode_pass_b_program();
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // FITFH PASS: post process. Display image.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    this->use_post_process_program();
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Draw imgui layer if we have one
    if (this->imgui_layer != nullptr)
        this->imgui_layer->draw();
    
    return 0;
}

void glfw_key_callback(GLFWwindow *window, int key_code, int scan_code, int action, int mods);

std::thread Display::start() 
{
    return std::thread([this](){

        using std::chrono::high_resolution_clock;
        using std::chrono::duration_cast;
        using std::chrono::duration;
        using std::chrono::milliseconds;

        // setup rendering context. Must be same thread.
        if (this->setup() != 0)
            exit(1);

        // set timer reference
        timer_start = std::chrono::high_resolution_clock::now();

        // register mouse/keyboard callback functions and do general setup of inputs
        glfwSetWindowUserPointer(this->window, this);
        if (this->keyboard != nullptr) {
            auto key_callback = [](GLFWwindow *window, int key_code, int scan_code, int action, int mods){
                Display *this_display = static_cast<Display*>(glfwGetWindowUserPointer(window));
                
                // ctrl-f (control focus) toggles this->focused
                if (mods == 2 && key_code == 70 && action == GLFW_PRESS) {
                    this_display->focused = !this_display->focused;
                    if (this_display->focused) {
                        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                        this_display->imgui_layer->disable_interaction();
                    } else {
                        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                        this_display->imgui_layer->enable_interaction();
                    }
                    return;
                }

                if (this_display->imgui_layer->want_capture_io())
                    return;
                if (action == GLFW_PRESS /* || action == GLFW_REPEAT */)
                    this_display->keyboard->press(key_code, mods);
            };
            glfwSetKeyCallback(this->window, key_callback);
        }
        if (this->mouse != nullptr) {
            if (glfwRawMouseMotionSupported()) {
                glfwSetInputMode(this->window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
            }

            auto cursor_move_callback = [](GLFWwindow *window, double xpos, double ypos) {
                Display *this_display = static_cast<Display*>(glfwGetWindowUserPointer(window));
                if (this_display->focused) {
                    this_display->mouse->poll(xpos, ypos);
                } else {
                    if (this_display->imgui_layer->want_capture_io())
                        return;
                }
            };
            glfwSetCursorPosCallback(this->window, cursor_move_callback);
        }

        // setup AFTER registering GLFW input callback. Otherwise backspace, enter key, etc.
        // doesn't work inside dear imgui.
        if (this->imgui_layer != nullptr) {
            const char *glsl_version = "#version 330 core";
            this->imgui_layer->setup(this->window, glsl_version);
        }

        // enter render loop 
        bool should_exit = false;
        while(!should_exit) {
            //std::this_thread::sleep_for(std::chrono::milliseconds(16)); // approx 60 fps
            double t_start = glfwGetTime();
            should_exit = this->loop();
            double t_end = glfwGetTime();

            //average this frame time with previous frames.
            this->imgui_layer->info->frames_per_second *= 0.9f;
            this->imgui_layer->info->frames_per_second += 0.1f / (t_end - t_start);
        }

        // terminate entire program on window close.
        // Not the most pretty solution but idc.
        exit(0);
    });
}

void Display::attach_imgui_layer(ImguiLayer *imgui_layer)
{
    this->imgui_layer = imgui_layer;
}
    
void Display::attach_keyboard(Keyboard *keyboard)
{
    this->keyboard = keyboard;
}
    
void Display::attach_mouse(Mouse *mouse)
{
    this->mouse = mouse;
}



