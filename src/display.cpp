#include "display.h"

#include <iostream>
#include <thread>
#include <chrono>

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

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
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

    //GLFWmonitor *monitor = glfwGetPrimaryMonitor(); 
    //const GLFWvidmode *mode = glfwGetVideoMode(monitor);
 
    //glfwWindowHint(GLFW_RED_BITS,     mode->redBits);
    //glfwWindowHint(GLFW_GREEN_BITS,   mode->greenBits);
    //glfwWindowHint(GLFW_BLUE_BITS,    mode->blueBits);
    //glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    
    this->window = glfwCreateWindow(1280, 800, "6502-burken", nullptr, nullptr);
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

    glViewport(0,0,1280,800);
    
    // compile shaders and link
    unsigned int vert_shader = glCreateShader(GL_VERTEX_SHADER);
    unsigned int frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    unsigned int shader_program = glCreateProgram();
    glShaderSource(vert_shader, 1, &(this->vert_shader_source), nullptr);
    glShaderSource(frag_shader, 1, &(this->frag_shader_source), nullptr);
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
        return -1;
    }

    // delete shaders
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);
    
    glUseProgram(shader_program);
    
    // misc. enables, disables, & other settings
    glClearColor(0.4f, 0.2f, 0.2f, 1.0f);

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

    // setup uniform buffer objects to hold vga text buffer and char buffer.
    //unsigned int text_buffer_idx = glGetUniformBlockIndex(shader_program, "VgaTextBuffer");
    //unsigned int char_buffer_idx = glGetUniformBlockIndex(shader_program, "VgaCharBuffer");
    //glUniformBlockBinding(shader_program, text_buffer_idx, 0);
    //glUniformBlockBinding(shader_program, char_buffer_idx, 1);

    //glGenBuffers(1, &this->vga_text_ubo);
    //glGenBuffers(1, &this->vga_char_ubo);

    //glBindBuffer(GL_UNIFORM_BUFFER, this->vga_text_ubo);
    //glBufferData(GL_UNIFORM_BUFFER, Layout::VGA_TEXT_BUF_SIZE, this->mem.data + Layout::VGA_TEXT_BUF_LOW, GL_STATIC_DRAW); // TODO consider GL_DYNAMIC_DRAW
    //glBindBuffer(GL_UNIFORM_BUFFER, this->vga_char_ubo);
    //glBufferData(GL_UNIFORM_BUFFER, Layout::VGA_CHAR_BUF_SIZE, this->mem.data + Layout::VGA_CHAR_BUF_LOW, GL_STATIC_DRAW); // TODO consider GL_DYNAMIC_DRAW
    //glBindBuffer(GL_UNIFORM_BUFFER, 0);

    //glBindBufferBase(GL_UNIFORM_BUFFER, 0, this->vga_text_ubo);
    //glBindBufferBase(GL_UNIFORM_BUFFER, 1, this->vga_char_ubo);
    
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
   
    glCheckError();

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
    
    glCheckError();


    // Associate texture units with corresponding sampler
    glUniform1i(glGetUniformLocation(shader_program, "vga_text_buffer"), 0);
    glUniform1i(glGetUniformLocation(shader_program, "vga_char_buffer"), 1);

    //size_t size = Layout::VGA_CHAR_BUF_SIZE + Layout::VGA_TEXT_BUF_SIZE;
    //glGenBuffers(1, &this->vga_buffers_ubo);
    //glBindBuffer(GL_UNIFORM_BUFFER, this->vga_buffers_ubo);
    //glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STATIC_DRAW); // TODO consider GL_DYNAMIC_DRAW
    //glBindBuffer(GL_UNIFORM_BUFFER, 0);
    //glBindBufferRange(GL_UNIFORM_BUFFER, 0, this->vga_buffers_ubo, 0, size);

    //// fill text buffer
    //glBindBuffer(GL_UNIFORM_BUFFER, vga_buffers_ubo);
    //glBufferSubData(GL_UNIFORM_BUFFER, 0, Layout::VGA_TEXT_BUF_SIZE, this->mem.data + 
    //        Layout::VGA_TEXT_BUF_LOW);
    //glBindBuffer(GL_UNIFORM_BUFFER, 0);
    //
    //// fill char buffer
    //glBindBuffer(GL_UNIFORM_BUFFER, vga_buffers_ubo);
    //glBufferSubData(GL_UNIFORM_BUFFER, Layout::VGA_TEXT_BUF_SIZE, Layout::VGA_CHAR_BUF_SIZE, 
    //        this->mem.data + Layout::VGA_CHAR_BUF_LOW);
    //glBindBuffer(GL_UNIFORM_BUFFER, 0);

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

    // ---------------- redraw ----------------

    glClear(GL_COLOR_BUFFER_BIT); 

    // Not really necessary, but good practice to rebind stuff.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->vga_text_texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, this->vga_char_texture);
    glBindVertexArray(this->VAO);
    

    glDrawArrays(GL_TRIANGLES, 0, 3);
    
    std::cout << "Hello" << frame++ << std::endl;
    return 0;
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
