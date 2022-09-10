#include "display.h"
#include "util.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>

// default screen size
unsigned int display_width  = 1320;
unsigned int display_height =  820;

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

#include <vector>

void Display::setup_frambuffer_gen_program()
{
    glUseProgram(this->framebuffer_gen_program);

    // setup textures to hold vga text buffer and char buffer.
    glGenTextures(1, &this->vga_text_texture);
    glGenTextures(1, &this->vga_char_texture);

    // configure vga text buffer texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->vga_text_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
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
   
    // configure vga char buffer texture
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, this->vga_char_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
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
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Associate texture units with corresponding sampler
    glUniform1i(glGetUniformLocation(this->framebuffer_gen_program, "vga_text_buffer"), 0);
    glUniform1i(glGetUniformLocation(this->framebuffer_gen_program, "vga_char_buffer"), 1);
    
    // setup framebuffer for rendering to
    glGenFramebuffers(1, &this->framebuffer_gen_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer_gen_fbo);

    //std::vector<unsigned char> dadd;
    //for (int i = 0; i < 640*200; i++) {
    //    if (i % 2 == 0) {
    //        dadd.push_back(255);
    //        dadd.push_back(255);
    //        dadd.push_back(255);
    //        dadd.push_back(255);
    //    } else {
    //        dadd.push_back(255);
    //        dadd.push_back(0);
    //        dadd.push_back(0);
    //        dadd.push_back(255);
    //    }
    //}


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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1320, 820, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glBindTexture(GL_TEXTURE_2D, this->framebuffer_tex_ntsc_swap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1320, 820, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
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
    glUniform1i(glGetUniformLocation(this->framebuffer_gen_program, "vga_text_buffer"), 0);
    glUniform1i(glGetUniformLocation(this->framebuffer_gen_program, "vga_char_buffer"), 1);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->vga_text_texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, this->vga_char_texture);
    
    glClearColor(0.7f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT); 
}

void Display::use_ntsc_encode_program() 
{
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->framebuffer_tex_ntsc_active, 0);

    glUseProgram(this->ntsc_encode_program);
    glUniform1i(glGetUniformLocation(this->ntsc_encode_program, "frame_buffer_texture"), 0);
    
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
    glUniform1f(glGetUniformLocation(this->post_process_program, "time"), timer_current);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->framebuffer_tex_ntsc_swap);
    
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT); 
}

Display::~Display() {
    glfwTerminate();
} 

int Display::setup() 
{
    // glfw setup. Apparently vmware is only good enough for version 3.3
    glfwInit();
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
        return 1;
    }
    
    glfwSwapBuffers(this->window);
    glfwPollEvents();

    glBindVertexArray(this->VAO);
    
    // ---------------- redraw ----------------

    // --- BIND "WORKING" FRAMEBUFFER ---
    glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer_gen_fbo);

    // first pass: generate the "native framebuffer" image
    this->use_frambuffer_gen_program();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    
    // second pass: encode ntsc color
    this->use_ntsc_encode_program();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    
    // third pass:  decode ntsc color (pass a)
    this->use_ntsc_decode_pass_a_program();
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // fourth pass: decode ntsc color (pass b)
    this->use_ntsc_decode_pass_b_program();
    glDrawArrays(GL_TRIANGLES, 0, 3);
   
    // --- BIND DEFAULT FRAMEBUFFER ---
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // fitfh pass: post process. Display image.
    this->use_post_process_program();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    
    // ------------ measure time --------------
    auto time_elapsed = std::chrono::high_resolution_clock::now() - timer_start;
    long long us = std::chrono::duration_cast<std::chrono::microseconds>(time_elapsed).count();
    timer_current = float(double(us) / 1000000.0);

    return 0;
}
    

std::thread Display::start() {
    return std::thread([this](){
        // setup rendering context. Must be same thread.
        if (this->setup() != 0)
            exit(1);

        // set timer reference
        timer_start = std::chrono::high_resolution_clock::now();

        // enter render loop 
        bool should_exit = false;
        while(!should_exit) {
            std::this_thread::sleep_for(std::chrono::milliseconds(16)); // approx 60 fps
            should_exit = this->loop();
        }

        // terminate entire program on window close.
        // Not the most pretty solution but idc.
        exit(0);
    });
}
