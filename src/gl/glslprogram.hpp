#ifndef _GLSLPROGRAM_HPP
#define _GLSLPROGRAM_HPP

#include<iostream>
#include<GL/glew.h>

namespace vcl {
/**
 * @class glslprogram
 * @brief class to handle glsl
 */
class glslprogram {

public:

    glslprogram() : m_program_handle( 0 ), m_vertex_shader_handle( 0 ), m_fragment_shader_handle( 0 ), m_geometry_shader_handle( 0 ) {}

    glslprogram( const std::string& _vsfilename, const std::string& _fsfilename ) : m_program_handle( 0 ), m_vertex_shader_handle( 0 ), m_fragment_shader_handle( 0 ), m_geometry_shader_handle( 0 ), m_vsfilename( _vsfilename ), m_fsfilename( _fsfilename )
    {
        create_shader_program();
    }

    ~glslprogram()
    {
        glDeleteProgram( m_program_handle );
        if( m_vertex_shader_handle > 0 ) glDeleteShader( m_vertex_shader_handle );
        if( m_fragment_shader_handle > 0 ) glDeleteShader( m_fragment_shader_handle );
        if( m_geometry_shader_handle > 0 ) glDeleteShader( m_geometry_shader_handle );
    }

    void use();

    std::string log() const { return m_log; }

    void bind_attrib_location( const GLuint location, const std::string& name ) { glBindAttribLocation( m_program_handle, 0, name.c_str() ); }
    void bind_frag_data_location( const GLuint location, const std::string& name ) { glBindFragDataLocation( m_program_handle, 0, name.c_str() ); }

    static void print_glsl_version();

    GLuint vertex_shader_handle() const { return m_vertex_shader_handle; }
    GLuint geometry_shader_handle() const { return m_geometry_shader_handle; }
    GLuint fragment_shader_handle() const { return m_fragment_shader_handle; }
    GLuint program_handle() const { return m_program_handle; }

    std::string load_file( const std::string& filename );
    void compile_status( const GLuint handle );
    void link_status( const GLuint handle );

    void bind() const { glUseProgram( m_program_handle ); }
    void unbind() const { glUseProgram( 0 ); }
    void print_active_attribute() const;

private:
    GLuint m_program_handle;
    GLuint m_vertex_shader_handle;
    GLuint m_fragment_shader_handle;
    GLuint m_geometry_shader_handle;

    std::string m_log;
    std::string m_vsfilename;
    std::string m_fsfilename;
    std::string m_gsfilename;

    void create_shader_program();
};

}

#include"glslprogram-impl.hpp"

#endif
