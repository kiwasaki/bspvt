#ifndef GLMTEST_GLFWGLSL_HPP
#define GLMTEST_GLFWGLSL_HPP

#include<glm/gtc/matrix_transform.hpp>
#include"glslprogram.hpp"
#include"glfw.hpp"
#include"../config.hpp"
#include"glmcamera.hpp"
#include"../cubemap/gl_cubemap.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace vcl {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class glslmodel;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum class camera_mode {
    walk,
    pan,
    rotate,
    turn,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class glfwglsl : public glfw {

public:

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    explicit glfwglsl( const std::string& _title = std::string( "opengl render" ), const int _width = 512, const int _height = 512 ) : glfw( _title, _width, _height ), m_model_size( 0 )
    {
        glewExperimental = GL_TRUE; //required to use vertex array object
        GLenum error = glewInit();
        if( error != GLEW_OK ) {
            std::cerr << "glew initialization failed.\n";
            exit( -1 );
        }

        m_glsl = std::make_unique< glslprogram >( "../glsl/simple_shader.vert", "../glsl/simple_shader.frag" );
        m_mvp_id = glGetUniformLocation( m_glsl->program_handle(), "mvp" ); if( m_mvp_id < 0 ) std::cerr << "uniform mat4 mvp is not found in shader.\n";
        m_mode = camera_mode::turn;//camera_mode::rotate; //camera_mode::walk;
        glfwSetCursorPosCallback( m_window, motion_function );
        glfwSetKeyCallback( m_window, keyboard_function );

        glEnable( GL_DEPTH_TEST );
        glDepthFunc( GL_LEQUAL );
    }

    ~glfwglsl() override = default;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void loop() override
    {
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        m_glsl->use();
        m_glsl->bind();

        glUniformMatrix4fv( m_mvp_id, 1, GL_FALSE, &m_mvp[ 0 ][ 0 ] );
        for( int i = 0; i < m_model_size; ++i ) {
            m_model[ i ].display();
        }
        m_glsl->unbind();
    }

    static void mouse_function( GLFWwindow *window, int button, int action, int mods );

    static void motion_function( GLFWwindow *window, double xpos, double ypos );

    static void keyboard_function( GLFWwindow *window, int key, int scancode, int action, int mods );


    void init_model( const size_t model_size, const std::string& pathname, const std::vector< std::string >& filename );

    void init_camera();

    void update_camera( float dx, float dy );

    const std::unique_ptr< glmcamera >& camera() const { return m_camera; }

private:

    size_t m_model_size;
    std::unique_ptr< glslprogram > m_glsl;
    std::unique_ptr< glslmodel [] > m_model;
    glm::mat4 m_modelview_matrix;
    glm::mat4 m_projection_matrix;
    glm::mat4 m_mvp; //projection_matrix * modelview_matrix;
    GLint m_mvp_id;
    std::unique_ptr< glmcamera > m_camera;
    camera_mode m_mode;
    bool m_camera_update = false;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void glfwglsl::init_model( const size_t model_size, const std::string& pathname, const std::vector< std::string >& filename )
{
    m_model_size = model_size;
    m_model = std::make_unique< glslmodel [] >( model_size );
    for( int i = 0; i < model_size; ++i ) {
        m_model[ i ].init( pathname, filename[ i ] );
        std::cout << m_model[ i ].to_string();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void glfwglsl::init_camera()
{
    const glm::vec3 eye = { config::eye_x, config::eye_y, config::eye_z };
    const glm::vec3 ref = { config::ref_x, config::ref_y, config::ref_z };
    m_camera = std::make_unique< glmcamera >( eye, ref, config::window_size_x, config::window_size_y, config::cam_fovy );
    m_modelview_matrix = m_camera->modelview_matrix();
    m_projection_matrix = m_camera->projection_matrix();
    m_mvp = m_camera->mvp();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void glfwglsl::mouse_function( GLFWwindow *window, int button, int action, int mods )
{
    double px, py;
    auto *const instance = static_cast< glfwglsl * >( glfwGetWindowUserPointer( window ) );
    if( button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS ) {
        glfwGetCursorPos( window, &px, &py );
        instance->m_px = px;
        instance->m_py = py;
        //std::cout << px << " " << py << "\n";
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void glfwglsl::motion_function( GLFWwindow *window, double xpos, double ypos )
{
    auto *const instance = static_cast< glfwglsl * >( glfwGetWindowUserPointer( window ) );
    const auto dx = static_cast< float >( xpos - instance->m_px );
    const auto dy = static_cast< float >( ypos - instance->m_py );
    instance->update_camera( dx, dy );
    instance->m_px = xpos;
    instance->m_py = ypos;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void glfwglsl::update_camera( const float dx, const float dy )
{
    const float scale = 0.01f;
    const float angle_scale = 0.005f;
    if( !m_camera_update ) return;
    switch( m_mode ) {
        case camera_mode::walk:
            m_camera->walk( scale * dx );
            break;

        case camera_mode::pan:
            m_camera->pan( - scale * dx, scale * dy );
            break;

        case camera_mode::rotate:
            m_camera->rotate( angle_scale * dy, angle_scale * dx );
            break;

        case camera_mode::turn:
            m_camera->turn( angle_scale * dy, angle_scale * dx );
            break;
    }
    m_mvp = m_camera->mvp();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void glfwglsl::keyboard_function( GLFWwindow *window, int key, int scancode, int action, int mods )
{
    auto *const instance = static_cast< glfwglsl * >( glfwGetWindowUserPointer( window ) );
    if( key == GLFW_KEY_Q && action == GLFW_PRESS ) {
        glfwSetWindowShouldClose( window, GL_TRUE );
    } else if( key == GLFW_KEY_W && action == GLFW_PRESS ) {
        instance->m_mode = camera_mode::walk;
    } else if( key == GLFW_KEY_T && action == GLFW_PRESS ) {
        instance->m_mode = camera_mode::turn;
    } else if( key == GLFW_KEY_R && action == GLFW_PRESS ) {
        instance->m_mode = camera_mode::rotate;
    } else if( key == GLFW_KEY_P && action == GLFW_PRESS ) {
        instance->m_mode = camera_mode::pan;
    } else if( key == GLFW_KEY_C && action == GLFW_PRESS ) {
        instance->m_camera_update = !instance->m_camera_update;
    } else if( key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS ) {
        //print camera eye and ref
        std::cout << "camera eye " << instance->m_camera->eye().x << " " << instance->m_camera->eye().y << " " << instance->m_camera->eye().z << "\n";
        std::cout << "camera ref " << instance->m_camera->ref().x << " " << instance->m_camera->ref().y << " " << instance->m_camera->ref().z << "\n";
        std::cout << "camera par " << instance->m_camera->vr()    << " " << instance->m_camera->theta() << " " << instance->m_camera->phi()   << "\n";
    }
}

}

#endif
