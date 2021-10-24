#ifndef _GLMCAMERA_HPP_
#define _GLMCAMERA_HPP_

#include<iostream>
#include<sstream>
#include<glm/vec3.hpp>
#include<glm/matrix.hpp>
#include<glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include<glm/gtx/string_cast.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace vcl {


class glmcamera {

public:

    glmcamera( const glm::vec3& eye, const glm::vec3& ref, const int width = 512, const int height = 512, const float fovy = 45.f, const float near = 1e-3f, const float far = 1e2f )
            : m_eye( eye ), m_ref( ref ), m_width( width ), m_height( height ), m_aspect( ( float ) m_width / ( float ) m_height ), m_fovy( fovy ), m_tfov( tanf( glm::radians( fovy ) ) ), m_near( near ), m_far( far ) {
        const glm::vec3 v = m_eye - m_ref;
        const glm::vec3 d = normalize( m_eye - m_ref ); //( costh * sinph, sinth, costh * cosph )
        const float cth = sqrtf( d.x * d.x + d.z * d.z );
        //const float sth = lydir;
        const float cph = d.z / cth;
        const float sph = d.x / cth;
        //the = acosf( std::max( std::min( cth, 1.f ), -1.f ) );
        m_vr = length( v );
        m_th = acosf( cth );
        m_ph = atan2f( sph, cph );
        make_matrix();
    }

    ~glmcamera() = default;

    glm::mat4 modelview_matrix() const;
    glm::mat4 projection_matrix() const;
    glm::mat4 mvp() const;

    glm::vec3 eye() const { return m_eye; }
    glm::vec3 ref() const { return m_ref; }
    glm::vec3 u() const { return m_u; }
    glm::vec3 v() const { return m_v; }
    glm::vec3 w() const { return m_w; }
    float near() const { return m_near; }
    float far() const { return m_far; }
    float fovy() const { return m_fovy; }
    int width() const { return m_width; }
    int height() const { return m_height; }
    std::string to_string() const;

    void walk( const float delta )
    {
        m_eye -= delta * m_w;
        m_ref -= delta * m_w;
    }

    void turn( const float dth, const float dph )
    {
        m_th += dth;
        m_ph += dph;
        const float cth = cosf( m_th );
        const float sth = sinf( m_th );
        const float cph = cosf( m_ph );
        const float sph = sinf( m_ph );
        m_ref.x = -m_vr * cth * sph + m_eye.x;
        m_ref.y = -m_vr * sth + m_eye.y;
        m_ref.z = -m_vr * cth * cph + m_eye.z;
        m_w = { cth * sph, sth, cth * cph };
        m_u = { cph, 0.f, -sph };
        m_v = { -sth * sph, cth, -sth * cph };
    }

    void pan( const float dx, const float dy )
    {
        if ( fabs( dx ) > fabs( dy )) {
            m_eye += dx * m_u;
            m_ref += dx * m_u;
        } else {
            m_eye += dy * m_v;
            m_ref += dy * m_v;
        }
    }

    void rotate( const float dth, const float dph )
    {
        m_th += dth;
        m_ph += dph;
        const float cth = cosf( m_th );
        const float sth = sinf( m_th );
        const float cph = cosf( m_ph );
        const float sph = sinf( m_ph );
        m_eye.x = m_vr * cth * sph + m_ref.x;
        m_eye.y = m_vr * sth + m_ref.y;
        m_eye.z = m_vr * cth * cph + m_ref.z;
        m_w = { cth * sph, sth, cth * cph };
        m_u = { cph, 0.f, -sph };
        m_v = { -sth * sph, cth, -sth * cph };
    }

    const float vr() const { return m_vr; }
    const float theta() const { return m_th; }
    const float phi() const { return m_ph; }


private:

    glm::vec3 m_eye, m_ref;
    float m_vr, m_th, m_ph;
    float m_fovy, m_tfov;
    glm::vec3 m_u, m_v, m_w;
    float m_near, m_far;
    int m_width, m_height;
    float m_aspect;

    void make_matrix()
    {
        const glm::vec3 view = { ( m_eye.x - m_ref.x ) / m_vr, ( m_eye.y - m_ref.y ) / m_vr, ( m_eye.z - m_ref.z ) / m_vr };
        const float cth = sqrtf( view.x * view.x + view.z * view.z );
        const float sth = view.y;
        const float cph = view.z / cth;
        const float sph = view.x / cth;
        if ( fabs( cth ) < 1e-10f ) {
            m_u = glm::vec3( 1.f, 0.f, 0.f );
            m_v = glm::vec3( 0.f, 1.f, 0.f );
            m_w = glm::vec3( 0.f, 0.f, 1.f );
        } else {
            m_u = glm::vec3( cph, 0.f, -sph );
            m_v = glm::vec3( -sth * sph, cth, -sth * cph );
            m_w = view;
        }
    }
};

/**
 * @fn mat4 glmcamera::modelview_matrix() const
 * @brief calculate modelview matrix
 * @return mat4 modelview_matrix
 */
glm::mat4 glmcamera::modelview_matrix() const
{
    return glm::lookAt( m_eye, m_ref, m_v );
}

/**
 * @brief mat4 glmcamera::projection_matrix() const
 * @return mat4 projection_matrix
 */
glm::mat4 glmcamera::projection_matrix() const
{
    return glm::perspective( glm::radians( m_fovy ), m_aspect, m_near, m_far );
}

glm::mat4 glmcamera::mvp() const
{
    return glm::perspective( glm::radians( m_fovy ), m_aspect, m_near, m_far ) * glm::lookAt( m_eye, m_ref, m_v );
}

std::string glmcamera::to_string() const
{
    std::stringstream oss;
    oss << "glmcamera\n" << "eye : " << glm::to_string( m_eye ) << "\n" << "ref : " << glm::to_string( m_ref );
    oss << "\nu : " << glm::to_string( m_u ) << "\nv : " << glm::to_string( m_v ) << "\nw : " << glm::to_string( m_w );
    return oss.str();
}

}

#endif //GLSLMODEL_GLMCAMERA_HPP
