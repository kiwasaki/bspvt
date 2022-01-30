#include<fstream>
#include<sstream>
//#include"glslprogram.hpp"

namespace vcl {

void glslprogram::use()
{
    if( m_program_handle == 0 ) return;
    glUseProgram( m_program_handle );
}

void glslprogram::print_glsl_version()
{
    const GLubyte *renderer     = glGetString( GL_RENDERER );
    const GLubyte *vendor       = glGetString( GL_VENDOR );
    const GLubyte *version      = glGetString( GL_VERSION );
    const GLubyte *glslversion  = glGetString( GL_SHADING_LANGUAGE_VERSION );
    GLint major, minor;

    glGetIntegerv( GL_MAJOR_VERSION, &major );
    glGetIntegerv( GL_MINOR_VERSION, &minor );
    std::cout << "gl vendor     : " << vendor       << "\n";
    std::cout << "gl renderer   : " << renderer     << "\n";
    std::cout << "gl version    : " << version      << "\n";
    std::cout << "glsl version  : " << glslversion  << "\n";
}

std::string glslprogram::load_file( const std::string& filename )
{
    std::ostringstream code;
    std::ifstream fin( filename, std::ios::in );
    if( fin.fail() ) {
        std::cerr << "Cannot open file : " << filename << "\n";
    }
    code << fin.rdbuf();
    fin.close();
    return code.str();
}

void glslprogram::create_shader_program()
{
    std::string code;
    m_program_handle = glCreateProgram();
    if( m_program_handle == 0 ) {
        std::cerr << "Cannot create program object.\n";
        exit( -1 );
    }

    m_vertex_shader_handle = glCreateShader( GL_VERTEX_SHADER );
    code = load_file( m_vsfilename );
    const char* vcode = code.c_str();

    glShaderSource( m_vertex_shader_handle, 1, &vcode, nullptr );
    glCompileShader( m_vertex_shader_handle );
    compile_status( m_vertex_shader_handle );

    m_fragment_shader_handle = glCreateShader( GL_FRAGMENT_SHADER );
    code = load_file( m_fsfilename );
    const char* fcode = code.c_str();
    glShaderSource( m_fragment_shader_handle, 1, &fcode, nullptr );
    glCompileShader( m_fragment_shader_handle );
    compile_status( m_fragment_shader_handle );

    glAttachShader( m_program_handle, m_vertex_shader_handle );
    glAttachShader( m_program_handle, m_fragment_shader_handle );

    glLinkProgram( m_program_handle );
    link_status( m_program_handle );
}

void glslprogram::compile_status( const GLuint _handle )
{
    GLint result;
    int length, written;
    char *clog;
    glGetShaderiv( _handle, GL_COMPILE_STATUS, &result );
    if( result == GL_FALSE ) {
        glGetShaderiv( _handle, GL_INFO_LOG_LENGTH, &length );
        if( length > 0 ) {
            clog = new char [ length ];
            written = 0;
            glGetShaderInfoLog( _handle, length, &written, clog );
            m_log = clog;
            delete [] clog;
        }
        std::cerr << "Cannot compile shader : " << _handle << "\n";
        std::cerr << m_log << std::endl;
        exit( -1 );
    }
}

void glslprogram::link_status( const GLuint _handle )
{
    GLint status;
    GLint loglen;
    char *clog;
    GLsizei written;
    glGetProgramiv( _handle, GL_LINK_STATUS, &status );
    if( status == GL_FALSE ) {
        glGetProgramiv( _handle, GL_INFO_LOG_LENGTH, &loglen );
        if( loglen > 0 ) {
            clog = new char [ loglen ];
            glGetProgramInfoLog( _handle, loglen, &written, clog );
            m_log = clog;
            delete [] clog;
        }
        std::cerr << "Cannot link shader program\n";
        std::cerr << m_log << std::endl;
        exit( -1 );
    }
}

void glslprogram::print_active_attribute() const
{
    GLint maxLength, nAttribs;
    GLchar *name;
    GLint written, size, location;
    GLenum type;
    glGetProgramiv( m_program_handle, GL_ACTIVE_ATTRIBUTES, &nAttribs );
    glGetProgramiv( m_program_handle, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLength );
    name = new GLchar [ maxLength ];

    std::cout << "Index | Name " << std::endl;
    for( GLuint i = 0; i < nAttribs; i++ ) {
        glGetActiveAttrib( m_program_handle, i, maxLength, &written, &size, &type, name );
        location = glGetAttribLocation( m_program_handle, name );
        std::cout << location << " : " << name << "\n";
    }

    delete [] name;
}

}