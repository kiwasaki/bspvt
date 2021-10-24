#ifndef _BRDF_HPP_
#define _BRDF_HPP_

#include<iostream>
#include"../math/vec3.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace vcl {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// brdf base class
class brdf {

public:
    //constructor
    brdf() = default;

    //brdf( const vec3& wo, const vec3& n ) : m_wo( wo ), m_n( n ), m_t( normalize( cross( n, ( std::abs( n.x ) < std::abs( n.y ) ) ? vec3( 0.0, n.z, - n.y ) : vec3( - n.z, 0.0, n.x ) ) ) ), m_b( cross( m_t, m_n ) ), m_lwo( to_local( m_wo ) ){}
	brdf( const vec3& wo, const vec3& n ) : m_wo( wo ), m_n( n ), m_t( normalize( wo - n * dot(wo, n) ) ), m_b( normalize(cross( m_n, m_t )) ), m_lwo( to_local( m_wo ) ){}

    //sample incident direction (wi) and its pdf
    //virtual vec3 sample( random_number_generator& rng, double& pdf_w ) = 0;

    //calculate pdf for direction wi
    virtual float pdf( const vec3& wi ) const = 0;
    //evaluate brdf for incident direction wi
    virtual col3 f( const vec3& wi ) const = 0;
    //return outgoing (reflected) direction wo
    inline vec3 wo() const { return m_wo; }
    //return outgoing (reflected) local direction lwo
    inline vec3 lwo() const { return m_lwo; }
    //return normal direction t
    inline vec3 t() const { return m_t; }
    //return normal direction b
    inline vec3 b() const { return m_b; }
    //return normal direction n
    inline vec3 n() const { return m_n; }
    //convert direction w in world coordinate to direction lw in local coordinate
    inline vec3 to_local( const vec3& w ) const { return vec3( dot( m_t, w ), dot( m_b, w ), dot( m_n, w ) ); }
    //convert direction lw in local coordinate to direction w in world coordinate
    inline vec3 to_world( const vec3& w ) const { return w.x * m_t + w.y * m_b + w.z * m_n; }

protected:
    vec3 m_wo; //outgoing direction in world coordinate
    vec3 m_n; //normal
    vec3 m_t; //tangent
    vec3 m_b; //binormal
    vec3 m_lwo; //outgoing direction in local coordinate system
};



}




#endif //_BRDF_HPP_
