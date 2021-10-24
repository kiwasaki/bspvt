#ifndef _POLYGON3D_HPP_
#define _POLYGON3D_HPP_

#include<array>
#include"../math/vec3.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace vcl {

//
class polygon3d {

public:

    polygon3d() : m_n( 0 ), m_vertex() {}

    polygon3d( const polygon3d& p ) : m_n( p.m_n ), m_vertex() { for( size_t i = 0; i < m_n; ++i ) m_vertex[ i ] = p[ i ]; }

    polygon3d& operator=( const polygon3d& p ) {
        m_n = p.m_n;
        for( size_t i = 0; i < m_n; ++i ) {
            m_vertex[ i ] = p.m_vertex[ i ];
        }
        return *this;
    }

    ~polygon3d() = default;

    inline void emplace_back( const vec3& v ) {
        if( m_n > N ) {
            std::cerr << "error : num of vertices exceeds 16 \n";
            return;
        }
        m_vertex[ m_n++ ] = v;
    }

    inline size_t size() const { return m_n; }

    inline vec3 &operator[]( const size_t i ) { return assert( i < m_n ), m_vertex[ i ]; }

    inline const vec3 &operator[]( const size_t i ) const { return assert( i < m_n ), m_vertex[ i ]; }

    inline void clear() { m_n = 0; }

private:
    static constexpr size_t N = 16;
    size_t m_n;
    std::array< vec3, N > m_vertex;

};

//
static polygon3d frustum_clipping( const polygon3d& p, const std::array< vec3, 4 > normal )
{
    constexpr size_t N = 16;
    std::array< float, N > d = {};
    polygon3d output[ 2 ];
    constexpr float eps = 1e-10f;
    size_t id = 1;
    output[ 1 - id ] = p;
    for( size_t k = 0; k < 4; ++k ) {
        for( size_t i = 0, n = output[ 1 - id ].size(); i < n; ++i ) {
            d[ i ] = dot( output[ 1 - id ][ i ], normal[ k ] );
        }
        for( size_t i = 0, n = output[ 1 - id ].size(); i < n; ++i ) {
            const vec3 p0 = output[ 1 - id ][ i ];
            const vec3 p1 = ( i == n - 1 )? output[ 1 - id ][ 0 ] : output[ 1 - id ][ i + 1 ];
            const float d0 = d[ i ];
            const float d1 = ( i == n - 1 )? d[ 0 ] : d[ i + 1 ];
            const bool b0 = ( d0 >= 0.f );
            const bool b1 = ( d1 >= 0.f );
            if( !b0 && !b1 ) continue;

            if( b0 ) {
                if( b1 ) {
                    //both vertices are inside for positive
                    output[ id ].emplace_back( p1 );
                } else {
                    // edge exits for positive and edge enters for negative
                    const float t = - d0 / ( d1 - d0 );
                    const vec3 p2 = p0 + t * ( p1 - p0 );
                    if( t > eps ) output[ id ].emplace_back( p2 );
                }
            } else {
                //edge enters for positive and edge exits for negative
                const float t = - d0 / ( d1 - d0 );
                const vec3 p2 = p0 + t * ( p1 - p0 );
                if ( eps < t && t < 1.f - eps ) {
                    output[ id ].emplace_back( p2 );
                }
                output[ id ].emplace_back( p1 );
            }
        }
        id = 1 - id;
        output[ id ].clear();
        if( output[ 1 - id ].size() <= 2 ) {
            output[ 1 - id ].clear();
            return output[ 1 - id ];
        }
    }
    return output[ 1 - id ];
}

//
static std::array< polygon3d, 6 > frustum_clipping( const polygon3d& p )
{
    return {
        frustum_clipping( p, { vec3( -1.f, -1.f, 0.f ), vec3( -1.f, 1.f, 0.f ), vec3( -1.f, 0.f, -1.f ), vec3( -1.f, 0.f,  1.f ) } ),
        frustum_clipping( p, { vec3(  1.f,  1.f, 0.f ), vec3( 1.f, -1.f, 0.f ), vec3( 1.f,  0.f,  1.f ), vec3( 1.f,  0.f, -1.f ) } ),
        frustum_clipping( p, { vec3( -1.f, -1.f, 0.f ), vec3( 1.f, -1.f, 0.f ), vec3( 0.f, -1.f, -1.f ), vec3( 0.f, -1.f,  1.f ) } ),
        frustum_clipping( p, { vec3(  1.f,  1.f, 0.f ), vec3( -1.f, 1.f, 0.f ), vec3( 0.f,  1.f,  1.f ), vec3(  0.f, 1.f, -1.f ) } ),
        frustum_clipping( p, { vec3( -1.f, 0.f, -1.f ), vec3( 1.f, 0.f, -1.f ), vec3( 0.f, -1.f, -1.f ), vec3( 0.f,  1.f, -1.f ) } ),
        frustum_clipping( p, { vec3(  1.f,  0.f, 1.f ), vec3( -1.f, 0.f, 1.f ), vec3(  0.f,  1.f, 1.f ), vec3(  0.f, -1.f, 1.f ) } )
    };
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}


#endif //_POLYGON3D_HPP_
