#ifndef _GLFW_HPP_
#define _GLFW_HPP_

//#include<OpenGL/glu.h>
#include<GLFW/glfw3.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace vcl {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class glfw {

public:

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    explicit glfw( const std::string &title = std::string( "OpenGL Render" ), const int width = 512, const int height = 512 ) : m_width( width ), m_height( height ), m_px( 0.0 ), m_py( 0.0 ), m_window( nullptr )
    {
        if( !glfwInit() ) {
            std::cerr << "Cannot initialize GLFW\n";
            exit( -1 );
        }

        glfwWindowHint ( GLFW_CONTEXT_VERSION_MAJOR, 3 );
        glfwWindowHint ( GLFW_CONTEXT_VERSION_MINOR, 2 );
        glfwWindowHint ( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint ( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

        m_window = glfwCreateWindow( m_width, m_height, title.c_str(), nullptr, nullptr );

        if( !m_window ) {
            std::cerr << "Cannot create glfw window\n";
            exit( -1 );
        }

        glfwMakeContextCurrent( m_window );
        glfwSetKeyCallback( m_window, keyboard_function );
        glfwSetMouseButtonCallback( m_window, mouse_function );
        //glfwSetCursorPosCallback( m_window, motion_function );
        glfwSetWindowUserPointer( m_window, this );
        glfwSwapInterval( 1 );
    }

    virtual ~glfw()
    {
        glfwDestroyWindow( m_window );
        glfwTerminate();
    }

    void run()
    {
        while( !glfwWindowShouldClose( m_window ) )
        {
            this->loop();
            glfwSwapBuffers( m_window );
            glfwPollEvents();
        }
    }

    static void keyboard_function( GLFWwindow *window, int key, int scancode, int action, int mods )
    {
        if( key == GLFW_KEY_Q && action == GLFW_PRESS ) {
            //std::cout << key << "\n";
            glfwSetWindowShouldClose( window, GL_TRUE );
        }
    }

    static void mouse_function( GLFWwindow *window, int button, int action, int mods )
    {
        double px, py;
        if( button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS ) {
            glfwGetCursorPos( window, &px, &py );
            //std::cout << px << " " << py << "\n";
        }
    }

    /*
    static void motion_function( GLFWwindow *window, double xpos, double ypos )
    {
        //std::cout << xpos << " : " << ypos << "\n";
        glfw_test *const instance = static_cast< glfw_test * >( glfwGetWindowUserPointer( window ) );
        instance->m_px = xpos;
        instance->m_py = ypos;
    }
    */

    virtual void loop() = 0;


protected:
    int m_width;
    int m_height;
    GLFWwindow *m_window;
    double m_px;
    double m_py;

};

}

#endif
