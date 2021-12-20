#include "display.h"

#include <iostream>

Display::~Display() {
    glfwTerminate();
} 

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int Display::setup() 
{
    // glfw setup
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
    
    this->window = glfwCreateWindow(512, 512, "6502-burken", nullptr, nullptr);
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
        
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);

    return 0;
}

int Display::update()
{
    if(glfwWindowShouldClose(this->window)) {
        glfwTerminate();
        return 1;
    }

    glfwPollEvents();
    return 0;
}
