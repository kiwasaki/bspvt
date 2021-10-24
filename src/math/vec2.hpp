#ifndef _VEC2_HPP_
#define _VEC2_HPP_

#define _USE_MATH_DEFINES
#include<iostream>
#include<sstream>
#include<cmath>
#include<cassert>

namespace vcl {

template< typename T >
class tvec2 {

public:
    //constructor, copy constructor, destructor
    tvec2() : x( T( 0 ) ), y( T( 0 ) ) {}
    tvec2( const T _x, const T _y ) : x( _x ), y( _y ) {}
    tvec2( const tvec2 &v ) = default;
    ~tvec2() = default;

    tvec2 operator+( const tvec2 &rhs ) const { return tvec2( x + rhs.x, y + rhs.y ); }
    tvec2 operator-( const tvec2 &rhs ) const { return tvec2( x - rhs.x, y - rhs.y ); }
    tvec2 operator*( const tvec2 &rhs ) const { return tvec2( x * rhs.x, y * rhs.y ); }
    tvec2 operator/( const tvec2 &rhs ) const { return tvec2( x / rhs.x, y / rhs.y ); }
    tvec2 operator*( const T scale ) const { return tvec2( scale * x, scale * y ); }
    tvec2 operator/( const T scale ) const { return tvec2( x / scale, y / scale ); }

    friend tvec2 operator*( const T scale, const tvec2 &v ) { return tvec2( scale * v.x, scale * v.y ); }

    tvec2& operator+=( const tvec2 &rhs ) { return x += rhs.x, y += rhs.y, *this; }
    tvec2& operator-=( const tvec2 &rhs ) { return x -= rhs.x, y -= rhs.y, *this; }
    tvec2& operator*=( const T scale ) { return x *= scale, y *= scale, *this; }
    tvec2 &operator/=( const T scale ) { return x /= scale, y /= scale, *this; }

    T &operator[]( const int i ) { return assert( 0 <= i && i < 2 ), reinterpret_cast< T * >( this )[ i ]; }
    const T &operator[]( const int i ) const { return assert( 0 <= i && i < 2 ), reinterpret_cast< const T * >( this )[ i ]; }

    //unary operator
    tvec2 operator-() const { return tvec2( -x, -y ); }

    std::string to_string() const
    {
        std::stringstream oss;
        oss << "vec2 [ " << x << ", " << y << " ]";
        return oss.str();
    }

    friend T dot( const tvec2& v0, const tvec2& v1 ) { return ( v0.x * v1.x + v0.y * v1.y ); }
    friend T cross( const tvec2& v0, const tvec2& v1 ) { return ( v0.x * v1.y - v0.y * v1.x ); }
    friend T norm( const tvec2& v ) { return T( sqrt( v.x * v.x + v.y * v.y ) ); }
    friend float norm2( const tvec2& v ) { return ( v.x * v.x + v.y * v.y ); }

    friend tvec2 normalize( const tvec2& v ) { const T l = norm( v ); return tvec2( v.x / l, v.y / l ); }

    friend std::ostream &operator<<( std::ostream& os, const tvec2& v ) { return ( os << "[ " << v.x << ", " << v.y << " ]" ); }

    static tvec2 min( const tvec2& v0, const tvec2& v1 ) { return { std::min( v0.x, v1.x ), std::min( v0.y, v1.y ) }; }
    static tvec2 max( const tvec2& v0, const tvec2& v1 ) { return { std::max( v0.x, v1.x ), std::max( v0.y, v1.y ) }; }

    T x, y;
};

//using vec2 = tvec2< double >;
using vec2 = tvec2< float >;

};

#endif //_VEC2_HPP_
