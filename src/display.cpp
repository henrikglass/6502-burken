#include "display.h"
#include "util.h"

#include <iostream>
#include <thread>
#include <chrono>

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
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


    glGenTextures(1, &this->framebuffer_gen_fbo_color);
    glBindTexture(GL_TEXTURE_2D, this->framebuffer_gen_fbo_color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, RESOLUTION_X, RESOLUTION_Y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->framebuffer_gen_fbo_color, 0);

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


void Display::setup_ntsc_decode_program()
{
    glUseProgram(this->ntsc_decode_program);
    glUniform1i(glGetUniformLocation(this->ntsc_decode_program, "frame_buffer_texture"), 0);
    
    diva_error_check();
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

    this->window = glfwCreateWindow(1280, 600, "6502-burken", nullptr, nullptr);
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

    glViewport(0, 0, 1280, 600);
   
    this->framebuffer_gen_program = make_shader_program(
        this->pass_through_vert_shader_source,
        this->framebuffer_gen_frag_shader_source
    );
    
    this->ntsc_encode_program = make_shader_program(
        this->pass_through_vert_shader_source,
        this->ntsc_encode_frag_shader_source
    );
    
    this->ntsc_decode_program = make_shader_program(
        this->pass_through_vert_shader_source,
        this->ntsc_decode_frag_shader_source
    );

    // misc. enables, disables, & other settings
    glClearColor(0.7f, 0.0f, 0.0f, 1.0f);
    glDisable(GL_DEPTH_TEST);

    // setup VAO
    glGenVertexArrays(1, &this->VAO);
    glBindVertexArray(this->VAO);

    // setup full screen static quad 
    glGenBuffers(1, &(this->full_screen_tri_vbo));
    glBindBuffer(GL_ARRAY_BUFFER, this->full_screen_tri_vbo);
    glm::vec3 vertices[] = {
        glm::vec3(-1.0f, -1.0f, 0.0f),
        glm::vec3( 3.0f, -1.0f, 0.0f),
        glm::vec3(-1.0f,  3.0f, 0.0f)
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(glm::vec3), nullptr);
    glEnableVertexAttribArray(0);

    this->setup_frambuffer_gen_program();
    this->setup_ntsc_encode_program();
    this->setup_ntsc_decode_program();

    diva_error_check();
    
    return 0;
}

int frame = 0;
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

    // first pass: generate framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer_gen_fbo);
    this->use_frambuffer_gen_program();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    
    // second pass: encode ntsc color
    this->use_ntsc_encode_program();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    
    // second pass: decode ntsc color
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    this->use_ntsc_decode_program();
    glDrawArrays(GL_TRIANGLES, 0, 3);

    std::cout << "Hello" << frame++ << std::endl;
    return 0;
}
    
void Display::use_frambuffer_gen_program()
{
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
    glUseProgram(this->ntsc_encode_program);
    glUniform1i(glGetUniformLocation(this->ntsc_encode_program, "frame_buffer_texture"), 0);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->framebuffer_gen_fbo_color);
    
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT); 
}

void Display::use_ntsc_decode_program() 
{
    glUseProgram(this->ntsc_decode_program);
    glUniform1i(glGetUniformLocation(this->ntsc_decode_program, "frame_buffer_texture"), 0);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->framebuffer_gen_fbo_color);
    
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT); 
}

std::thread Display::start() {
    return std::thread([this](){
        // setup rendering context. Must be same thread.
        if (this->setup() != 0)
            exit(1);

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
