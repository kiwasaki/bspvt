#ifndef _VEC3_HPP_
#define _VEC3_HPP_

#include<iostream>
#include<cmath>
#include<string>
#include<cassert>
#include<sstream>

namespace vcl {


template< typename T >
class tvec3 {

public:
    //constructor, copy constructor, destructor
    tvec3() : x( T( 0 ) ), y( T( 0 ) ), z( T( 0 ) ) {}
    tvec3( const T _x, const T _y, const T _z ) : x( _x ), y( _y ), z( _z ) {}
    tvec3( const tvec3 &v ) = default;
    ~tvec3() = default;

    //arithmetic operator
    tvec3 operator+( const tvec3 &rhs ) const { return { x + rhs.x, y + rhs.y, z + rhs.z }; }
    tvec3 operator-( const tvec3 &rhs ) const { return { x - rhs.x, y - rhs.y, z - rhs.z }; }
    tvec3 operator*( const tvec3 &rhs ) const { return { x * rhs.x, y * rhs.y, z * rhs.z }; }
    tvec3 operator/( const tvec3 &rhs ) const { return { x / rhs.x, y / rhs.y, z / rhs.z }; }
    tvec3 operator*( const T scale ) const { return { scale * x, scale * y, scale * z }; }
    tvec3 operator/( const T scale ) const { return assert( fabs( scale ) > 1e-10f ), tvec3( x / scale, y / scale, z / scale ); }
    friend tvec3 operator*( const T scale, const tvec3 &v ) { return { scale * v.x, scale * v.y, scale * v.z }; }


    tvec3 &operator+=( const tvec3 &rhs ) { return x += rhs.x, y += rhs.y, z += rhs.z, *this; }
    tvec3 &operator-=( const tvec3 &rhs ) { return x -= rhs.x, y -= rhs.y, z -= rhs.z, *this; }
    tvec3 &operator*=( const T scale ) { return x *= scale, y *= scale, z *= scale, *this; }
    tvec3 &operator/=( const T scale ) { return x /= scale, y /= scale, z /= scale, *this; }
    tvec3 &operator*=( const tvec3 &rhs ) { return x *= rhs.x, y *= rhs.y, z *= rhs.z, *this; }
    tvec3 &operator/=( const tvec3 &rhs ) { return x /= rhs.x, y /= rhs.y, z /= rhs.z, *this; }

    T& operator[]( const int i ) { return assert( 0 <= i && i < 3 ), reinterpret_cast< T * >( this )[ i ]; }
    const T& operator[]( const int i ) const { return assert( 0 <= i && i < 3 ), reinterpret_cast< const T * >( this )[ i ]; }

    //unary operator
    tvec3 operator-() const { return tvec3( -x, -y, -z ); }

    std::string to_string() const
    {
        std::stringstream oss;
        oss << "vec3 [ " << x << ", " << y << ", " << z << " ]";
        return oss.str();
    }

    friend T dot( const tvec3 &v0, const tvec3 &v1 ) { return ( v0.x * v1.x + v0.y * v1.y + v0.z * v1.z ); }
    friend tvec3 cross( const tvec3 &v0, const tvec3 &v1 ) { return tvec3( v0.y * v1.z - v0.z * v1.y, v0.z * v1.x - v0.x * v1.z, v0.x * v1.y - v0.y * v1.x ); }
    friend T norm( const tvec3 &v ) { return T( sqrt( v.x * v.x + v.y * v.y + v.z * v.z ) ); }

    friend tvec3 normalize( const tvec3 &v )
    {
        const T l = norm( v );
        return tvec3( v.x / l, v.y / l, v.z / l );
    }

    friend std::ostream &operator<<( std::ostream &os, const tvec3 &v ) { return ( os << "[ " << v.x << ", " << v.y << ", " << v.z << " ]" ); }

    friend tvec3 min( const tvec3 &v0, const tvec3 &v1 ) { return tvec3( std::min( v0.x, v1.x ), std::min( v0.y, v1.y ), std::min( v0.z, v1.z ) ); }
    friend tvec3 max( const tvec3 &v0, const tvec3 &v1 ) { return tvec3( std::max( v0.x, v1.x ), std::max( v0.y, v1.y ), std::max( v0.z, v1.z ) ); }

    T x, y, z;
};

//using vec3 = tvec3< double >;
using vec3 = tvec3< float >;
using col3 = tvec3< float >;

};

#endif
