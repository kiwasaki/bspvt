
#ifndef _EXP_POLYGON_HPP_
#define _EXP_POLYGON_HPP_

#include<array>
#include"../math/vec2.hpp"
//#include<glm/gtx/string_cast.hpp>

namespace vcl {

namespace experimental {

// 頂点列{p_0,...,p_{n-1}で多角形を表現
// p_0p_1, ..., p_{n-1}p_0が辺
// no bounding box
class polygon {
public:
	static constexpr int max_size = 20;

	//コンストラクタ
	polygon() : m_size( 0 ) {}

	//
	void emplace_back( const vec2& p ) {
		assert( m_size < max_size );
		m_p[ m_size++ ] = p;
	}

	vec2 &operator[]( const size_t i ) { return assert( i < m_size ), m_p[ i ]; }
	const vec2 &operator[]( const size_t i ) const { return assert( i < m_size ), m_p[ i ]; }

	vec2 min() const { return vec2(); }
	vec2 max() const { return vec2(); }

	int size() const { return m_size; }
	void clear() { m_size = 0; }
private:
	int m_size;
	vec2 m_p[ max_size ];
	//vec2 m_min;
	//vec2 m_max;
};

//
std::array< polygon, 2 > split( const polygon &p, const float a, const float b, const float c ) {
	constexpr float eps = 1e-6f;
	constexpr float sd_eps = 1e-5f; //符号付き距離の許容範囲
	constexpr size_t max_size = 16;
	std::array< float, max_size > sd = {};
	std::array< polygon, 2 > pp;

	const auto pmin = p.min();
	const auto pmax = p.max();
	const int ne = p.size(); //多角形pの辺(頂点)の数

	//trivial reject
	bool trivial_reject;
	{
		const float dx = ( a >= 0.f ) ? pmin.x : pmax.x;
		const float dy = ( b >= 0.f ) ? pmin.y : pmax.y;
		trivial_reject = ( a * dx + b * dy + c >= 0.f );
	}
	//trivial_rejectの符号付き距離が0以上なら多角形pはsplit lineの右側に存在 分割はなし
	if ( trivial_reject )
		return pp[ 1 ] = p, pp;

	//trivial accept
	bool trivial_accept;
	{
		const float dx = ( a >= 0.f ) ? pmax.x : pmin.x;
		const float dy = ( b >= 0.f ) ? pmax.y : pmin.y;
		trivial_accept = ( a * dx + b * dy + c <= 0.f );
	};
	//trivial_acceptの符号付き距離が0未満なら多角形pはsplit lineの左側に存在 分割はなし
	if ( trivial_accept )
		return pp[ 0 ] = p, pp;

	//多角形の各頂点での符号付き距離を計算
	//int id[ 2 ] = { ne, ne };
	int id[ 4 ] = { ne, ne, ne, ne };
	size_t count = 0;
	bool all_positive = true;
	bool all_negative = true;
	for ( size_t i = 0; i < ne; ++i ) {
		sd[ i ] = a * p[ i ].x + b * p[ i ].y + c;
		if ( sd[ i ] >   sd_eps )
			all_negative = false;
		if ( sd[ i ] < - sd_eps )
			all_positive = false;
	}
	//全頂点の符号付き距離が0以上 -> 多角形は分割線の右側
	if ( all_positive )
		return pp[ 1 ] = p, pp;

	//全頂点の符号付き距離が0以下 -> 多角形は分割線の左側
	if ( all_negative )
		return pp[ 0 ] = p, pp;

	//split lineで多角形pは分割される
	for ( size_t i = 0; i < ne; ++i ) {
		size_t j = ( i == ne - 1 ) ? 0 : i + 1;
		if ( ( sd[ i ] >= 0.f && sd[ j ] < 0.f ) || ( sd[ i ] <= 0.f && sd[ j ] > 0.f ) ) {
			id[ count++ ] = i;
		}
		if ( count > 2 ) {
			printf( " %zu : split function error. Currently convex polygon is only supported.\n", count );
			std::cout << a << ", " << b << ", " << c << std::endl;
			std::cout << "signed distance" << std::endl;
			for( size_t k = 0; k < ne; ++k ) {
				std::cout << sd[ k ] << std::endl;
			}
			std::cout << "p" << std::endl;
			for( size_t k = 0; k < ne; ++k ) {
//				std::cout << glm::to_string( p[ k ] ) << std::endl;
				std::cout << p[ k ] << std::endl;
			}
			return pp;
		}
	}
	//assert( id[ 0 ] != ne && id[ 1 ] != ne );

	//多角形の辺とsplit lineとの交点計算
	vec2 p0, p1;
	const vec2 pi0 = p[ id[ 0 ] ];
	const vec2 pi1 = ( id[ 0 ] >= ne - 1 ) ? p[ 0 ] : p[ id[ 0 ] + 1 ];
	const vec2 pj0 = p[ id[ 1 ] ];
	const vec2 pj1 = ( id[ 1 ] >= ne - 1 ) ? p[ 0 ] : p[ id[ 1 ] + 1 ];
	const float d0 = a * ( pi1.x - pi0.x ) + b * ( pi1.y - pi0.y );
	const float d1 = a * ( pj1.x - pj0.x ) + b * ( pj1.y - pj0.y );
	const float e0 = -a * pi0.x - b * pi0.y - c;
	const float e1 = -a * pj0.x - b * pj0.y - c;
	const bool valid0 = ( std::abs( d0 ) > eps );
	const bool valid1 = ( std::abs( d1 ) > eps );
	if ( valid0 && valid1 ) {
		const float t0 = e0 / d0;
		const float t1 = e1 / d1;
		//assert( 0.f <= t0 && t0 < 1.f && 0.f <= t1 && t1 < 1.f );
		p0 = ( 1.f - t0 ) * pi0 + t0 * pi1;
		p1 = ( 1.f - t1 ) * pj0 + t1 * pj1;

		{
			const int beg = ( id[ 0 ] + 1 >= ne ) ? 0 : id[ 0 ] + 1;
			int oid = ( sd[ beg ] >= 0.f ) ? 1 : 0; //beg番目の始点の符号付き距離が正なら右側(1), そうでなければ左側(0)
			pp[ oid ].emplace_back( p0 );
			for ( int i = beg; i <= id[ 1 ]; ++i ) {
				pp[ oid ].emplace_back( p[ i ] );
			}
			if ( std::abs( t1 ) > eps )
				pp[ oid ].emplace_back( p1 );
		}

		{
			const int beg = ( id[ 1 ] + 1 >= ne ) ? 0 : id[ 1 ] + 1;
			int oid = ( sd[ beg ] >  0.f ) ? 1 : 0; //beg番目の始点の符号付き距離が正なら右側(1), そうでなければ左側(0)
			pp[ oid ].emplace_back( p1 );
			for ( int i = beg;; ++i ) {
				i = ( i > ne - 1 ) ? 0 : i;
				pp[ oid ].emplace_back( p[ i ] );
				if ( i == id[ 0 ] )
					break;
			}
			if ( std::abs( t0 ) > eps )
				pp[ oid ].emplace_back( p0 );
		}
		return pp;
	} else {
		/*
		//printf( "split function error\n" );
		std::cout << "split function error " << d0 << ", " << d1 << ", " << e0 << ", " << e1 << std::endl;
		std::cout << pi0 << ", " << pi1 << ", " << pj0 << ", " << pj1 << std::endl;
//		std::cout << glm::to_string( pi0 ) << ", " << glm::to_string( pi1 ) << ", " << glm::to_string( pj0 ) << ", " << glm::to_string( pj1 ) << std::endl;
		std::cout << e1 / d1 << std::endl;
		*/
		return pp;
	}
}


//
namespace anon {
// pp[ 0 ] : split lineの左側にある(分割された)多角形, pp[ 1 ] : split lineの右側にある(分割された)多角形
std::array< polygon, 2 > split( const polygon &p, const float a, const float b, const float c ) {
	constexpr float eps = 1e-6f;
	constexpr float sd_eps = 1e-5f; //符号付き距離の許容範囲
	float sd0, sd1;
	std::array< polygon, 2 > pp;
	const int ne = p.size(); //多角形pの辺(頂点)の数
#if 0
	const auto pmin = p.min();
	const auto pmax = p.max();
	//trivial reject
	bool trivial_reject;
	{
		const float dx = ( a >= 0.f ) ? pmin.x : pmax.x;
		const float dy = ( b >= 0.f ) ? pmin.y : pmax.y;
		trivial_reject = ( a * dx + b * dy + c >= 0.f );
	}
	//trivial_rejectの符号付き距離が0以上なら多角形pはsplit lineの右側に存在 分割はなし
	if ( trivial_reject ) return pp[ 1 ] = p, pp;

	//trivial accept
	bool trivial_accept;
	{
		const float dx = ( a >= 0.f ) ? pmax.x : pmin.x;
		const float dy = ( b >= 0.f ) ? pmax.y : pmin.y;
		trivial_accept = ( a * dx + b * dy + c <= 0.f );
	};
	//trivial_acceptの符号付き距離が0未満なら多角形pはsplit lineの左側に存在 分割はなし
	if ( trivial_accept ) return pp[ 0 ] = p, pp;
#endif
	//多角形の各頂点での符号付き距離を計算
	vec2 p0, p1;
	sd0 = a * p[ ne - 1 ].x + b * p[ ne - 1 ].y + c;
	for ( int i = 0; i < ne; ++i ) {
		int j = ( i == 0 ) ? ne - 1 : i - 1;
		sd1 = a * p[ i ].x + b * p[ i ].y + c;
#if 0
		bool enter = false;
		bool exit  = false;
		if( sd0 < 0.f && sd1 >= 0.f ) {
			const float t = ( - sd0 ) / ( sd1 - sd0 ); assert( 0.f <= t && t < 1.f );
			p0 = p[ j ] + t * ( p[ i ] - p[ j ] );
			enter = true;
		}
		if( sd0 >= 0.f && sd1 < 0.f ) {
			const float t =     sd0  / ( sd0 - sd1 ); assert( 0.f <= t && t < 1.f );
			p1 = p[ j ] + t * ( p[ i ] - p[ j ] );
			exit = true;
		}
		if( sd1 >= 0.f ) {
			if( enter ) { pp[ 0 ].emplace_back( p0 ); pp[ 1 ].emplace_back( p0 ); }
			pp[ 1 ].emplace_back( p[ i ] );
		} else {
			if( exit  ) { pp[ 0 ].emplace_back( p1 ); pp[ 1 ].emplace_back( p1 ); }
			pp[ 0 ].emplace_back( p[ i ] );
		}
#else
		if( sd1 >= 0.f ) {
			if( sd0 < 0.f ) {
				const float t = ( - sd0 ) / ( sd1 - sd0 ); assert( 0.f <= t && t < 1.f );
				p0 = p[ j ] + t * ( p[ i ] - p[ j ] );
				pp[ 0 ].emplace_back( p0 ); pp[ 1 ].emplace_back( p0 );
			}
			pp[ 1 ].emplace_back( p[ i ] );
		} else {
			if( sd0 >= 0.f ) {
				const float t = sd0 / ( sd0 - sd1 ); assert( 0.f <= t && t < 1.f );
				p1 = p[ j ] + t * ( p[ i ] - p[ j ] );
				pp[ 0 ].emplace_back( p1 ); pp[ 1 ].emplace_back( p1 );
			}
			pp[ 0 ].emplace_back( p[ i ] );
		}
#endif
		sd0 = sd1;
	}
	return pp;
}

}

}

}

#endif //_EXP_POLYGON_HPP_
