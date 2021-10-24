#ifndef _GLSLMODEL_HPP_
#define _GLSLMODEL_HPP_

#include<vector>
#include<string>
#include<sstream>
#include<array>
#include<GL/glew.h>
#include"objloader.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace vcl {

#define BUFFER_OFFSET( bytes ) ( ( GLubyte * ) NULL + ( bytes ) )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class glslmodel {

public:

    glslmodel() = default;

    glslmodel( const std::string& _pathname, const std::string& _filename ) : m_pathname( _pathname ), m_filename( _filename ), m_vertex_size( 0 ), m_triangle_size( 0 )
    {
        init();
    }

    void init( const std::string& _pathname, const std::string& _filename )
    {
        m_pathname = _pathname;
        m_filename = _filename;
        init();
    }

    ~glslmodel() = default;

    void display() const;
    std::string to_string() const;

private:

    std::string m_pathname;
    std::string m_filename;
    GLuint m_vao_id;
    GLuint m_vbo_id[ 4 ];
    GLuint m_vbo_index_id;
    size_t m_vertex_size;
    size_t m_triangle_size;

    std::vector< obj::vec3f > m_vertex;
    std::vector< obj::col3f > m_color;
    std::vector< obj::vec3f > m_normal;
    std::vector< obj::vec2f > m_texcoord;
    std::vector< obj::vec3i > m_index;
    obj::vec3f m_min, m_max;

    void init();

};


void glslmodel::init()
{
    std::vector< obj::mesh > mesh = obj::loadobj( m_pathname, m_filename );

    const size_t geometry_size = mesh.size();
    std::vector< size_t > vertex_size( geometry_size, 0 );
    std::vector< size_t > offset( geometry_size + 1, 0 );
    std::vector< size_t > triangle_size( geometry_size, 0 );
    std::vector< size_t > triangle_offset( geometry_size + 1, 0 );

    for( size_t i = 0; i < geometry_size; ++i ) {
        vertex_size[ i ] = mesh[ i ].positions.size();
        triangle_size[ i ] = mesh[ i ].triangles.size();
        offset[ i + 1 ] = offset[ i ] + vertex_size[ i ];
        triangle_offset[ i + 1 ] = triangle_offset[ i ] + triangle_size[ i ];
        m_triangle_size += triangle_size[ i ];
    }

    m_vertex_size = offset[ geometry_size ];

    m_min = { std::numeric_limits< float >::max() };
    m_max = { std::numeric_limits< float >::lowest() };

    m_vertex.resize( m_vertex_size );
    m_normal.resize( m_vertex_size );
    m_texcoord.resize( m_vertex_size );
    m_color.resize( m_vertex_size );
    m_index.resize( m_triangle_size );

    for( size_t i = 0; i < geometry_size; ++i ) {
        for( int j = 0; j < vertex_size[ i ]; ++j ) {
            m_vertex[ j + offset[ i ] ] = mesh[ i ].positions[ j ];
            m_normal[ j + offset[ i ] ] = mesh[ i ].normals[ j ];
            m_color[  j + offset[ i ] ] = mesh[ i ].material.Kd;
            if( m_min.x > mesh[ i ].positions[ j ].x ) m_min.x = mesh[ i ].positions[ j ].x;
            if( m_min.y > mesh[ i ].positions[ j ].y ) m_min.y = mesh[ i ].positions[ j ].y;
            if( m_min.z > mesh[ i ].positions[ j ].z ) m_min.z = mesh[ i ].positions[ j ].z;
            if( m_max.x < mesh[ i ].positions[ j ].x ) m_max.x = mesh[ i ].positions[ j ].x;
            if( m_max.y < mesh[ i ].positions[ j ].y ) m_max.y = mesh[ i ].positions[ j ].y;
            if( m_max.z < mesh[ i ].positions[ j ].z ) m_max.z = mesh[ i ].positions[ j ].z;
        }
        for( int j = 0; j < triangle_size[ i ]; ++j ) {
            m_index[ j + triangle_offset[ i ] ].i = mesh[ i ].triangles[ j ].i + int( offset[ i ] );
            m_index[ j + triangle_offset[ i ] ].j = mesh[ i ].triangles[ j ].j + int( offset[ i ] );
            m_index[ j + triangle_offset[ i ] ].k = mesh[ i ].triangles[ j ].k + int( offset[ i ] );
        }
    }

    glGenBuffers( 4, m_vbo_id );
    glBindBuffer( GL_ARRAY_BUFFER, m_vbo_id[ 0 ] );
    glBufferData( GL_ARRAY_BUFFER, m_vertex_size * 3 * sizeof( float ), m_vertex.data(), GL_STATIC_DRAW );

    glBindBuffer( GL_ARRAY_BUFFER, m_vbo_id[ 1 ] );
    glBufferData( GL_ARRAY_BUFFER, m_vertex_size * 3 * sizeof( float ), m_normal.data(), GL_STATIC_DRAW );

    glBindBuffer( GL_ARRAY_BUFFER, m_vbo_id[ 2 ] );
    glBufferData( GL_ARRAY_BUFFER, m_vertex_size * 3 * sizeof( float ), m_color.data(), GL_STATIC_DRAW );

    glGenBuffers( 1, &m_vbo_index_id );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_vbo_index_id );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, m_triangle_size * 3 * sizeof( GLuint ), m_index.data(), GL_STATIC_DRAW );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

    //generate vertex array object
    glGenVertexArrays( 1, &m_vao_id );
    glBindVertexArray( m_vao_id );

    glEnableVertexAttribArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, m_vbo_id[ 0 ] );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( 0 ) );

    glEnableVertexAttribArray( 1 );
    glBindBuffer( GL_ARRAY_BUFFER, m_vbo_id[ 1 ] );
    glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( 0 ) );

    glEnableVertexAttribArray( 2 );
    glBindBuffer( GL_ARRAY_BUFFER, m_vbo_id[ 2 ] );
    glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( 0 ) );

    glBindVertexArray( 0 );

    //std::cout << to_string();
}


void glslmodel::display() const
{
    glBindVertexArray( m_vao_id );

    //vertex
    glEnableVertexAttribArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, m_vbo_id[ 0 ] );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( 0 ) );

    //normal
    glEnableVertexAttribArray( 1 );
    glBindBuffer( GL_ARRAY_BUFFER, m_vbo_id[ 1 ] );
    glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( 0 ) );

    //color
    glEnableVertexAttribArray( 2 );
    glBindBuffer( GL_ARRAY_BUFFER, m_vbo_id[ 2 ] );
    glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( 0 ) );

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_vbo_index_id );
    glDrawElements( GL_TRIANGLES, GLsizei( 3 * m_triangle_size ), GL_UNSIGNED_INT, BUFFER_OFFSET( 0 ) );

    glDisableVertexAttribArray( 2 );
    glDisableVertexAttribArray( 1 );
    glDisableVertexAttribArray( 0 );

    glBindVertexArray( 0 );
}


std::string glslmodel::to_string() const
{
    std::stringstream oss;
    oss << "glslmodel\n" << "vertex_size : " << m_vertex_size << "\ntriangle_size : " << m_triangle_size << "\n";
    oss << "filename : " << m_pathname + m_filename << "\n";
    oss << "bbox : ( " << m_min.x <<", " << m_min.y << ", " << m_min.z << " ) : ( " << m_max.x << ", " << m_max.y << ", " << m_max.z << " ) : ( "
        << m_max.x - m_min.x << ", " << m_max.y - m_min.y << ", " << m_max.z - m_min.z << " )\n";
    return oss.str();
}

};


#endif //_GLSLMODEL_HPP_
