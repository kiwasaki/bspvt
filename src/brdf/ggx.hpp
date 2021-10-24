#ifndef _BRDF_GGX_HPP_
#define _BRDF_GGX_HPP_

#include<random>
#include"brdf.hpp"

// random number generator class
class rng {
public:
	//constructor
	rng() : m_mt(std::random_device {}()) {}

	//generate uniform random number [ 0, 1 ]
	inline const float rand() { return m_rng(m_mt); }

	//generate random number (size_t) between x0 and x1 - 1
	inline const size_t rand(const size_t x0, const size_t x1)
	{
		const float x = m_rng(m_mt);
		return std::min(std::max(size_t(x0 + x * (x1 - x0)), x0), x1 - 1);
	}

private:
	//std::random_device m_device;
	std::mt19937 m_mt;
	std::uniform_real_distribution< float > m_rng;
};

namespace vcl {

class ggx : public brdf {

public:
	//コンストラクタ
    ggx( const vec3& wo, const vec3& n, const float alpha = 0.1f, const float F0 = 0.f ) : brdf( wo, n ), m_alpha( alpha ), m_F0( F0 ) {}

	//set関数(texture light)
	inline void set_gv(const vcl::vec3 &gv) { m_gv = gv; }
	inline void set_lp0(const vcl::vec3 &lp0) { m_lp0 = lp0; }
	inline void set_u_coord(const vcl::vec3 &u_coord) { m_u_coord = u_coord; }
	inline void set_v_coord(const vcl::vec3 &v_coord) { m_v_coord = v_coord; }
	inline void set_ortho_lv(const vcl::vec3 &ortho_lv) { m_ortho_lv = ortho_lv; }
	inline void set_ortho_uv(const vcl::vec2 &ortho_uv) { m_ortho_uv = ortho_uv; }
	inline void set_LOD(const float LOD) { m_LOD = LOD; }
	inline void set_poly_w(const float poly_w) { m_poly_w = poly_w; }
	inline void set_poly_h(const float poly_h) { m_poly_h = poly_h; }

	//set関数(env map)
	inline void set_ortho_uv_for_env_map(const vcl::vec2 ortho_uv_for_env_map, const uint8_t face)
	{
		m_ortho_uv_for_env_map[face] = ortho_uv_for_env_map;
	}
	inline void set_LOD_for_env_map(const float LOD_for_env_map, const uint8_t face)
	{
		m_LOD_for_env_map[face] = LOD_for_env_map;
	}

	//アクセッサ
	inline const vcl::vec3 &gv() const { return m_gv; }
	inline const vcl::vec3 &lp0() const { return m_lp0; }
	inline const vcl::vec3 &u_coord() const { return m_u_coord; }
	inline const vcl::vec3 &v_coord() const { return m_v_coord; }
	inline const vcl::vec3 &ortho_lv() const { return m_ortho_lv; }
	inline const vcl::vec2 &ortho_uv() const { return m_ortho_uv; }
	inline const float &LOD() const { return m_LOD; }
	inline const float &poly_w() const { return m_poly_w; }
	inline const float &poly_h() const { return m_poly_h; }

	inline const vcl::vec2 &ortho_uv_for_env_map(const uint8_t face) const { return m_ortho_uv_for_env_map[face]; }
	inline const float &LOD_for_env_map(const uint8_t face) const { return m_LOD_for_env_map[face]; }

    const vcl::vec3 sample(rng& rng, float& pdf_w) const
	{
		const float xi1 = rng.rand();
		const float xi2 = rng.rand();

		const float theta = atan(m_alpha * sqrt(xi1 / (1.f - xi1)));
		const float phi = 2.f * M_PI * xi2;
		const float st = sin(theta);
		const float ct = cos(theta);
		const float sp = sin(phi);
		const float cp = cos(phi);
		const vcl::vec3 lwh = normalize(vcl::vec3(st * cp, st * sp, ct));
		const vcl::vec3 wi = to_world(normalize(2.f * dot(m_lwo, lwh) * lwh - m_lwo));
		pdf_w = pdf(wi);
		return wi;
	}

	float pdf(const vcl::vec3& wi) const override
	{ 
		const float alpha2 = m_alpha * m_alpha;
		const vcl::vec3 wh = normalize((wi + m_wo));
		const float ct = std::clamp(dot(wh, m_n), -1.f, 1.f);
		const float theta = acos(ct);
		const float tt = tan(theta);
		const float J = 1.f / abs(4.f * dot(m_wo, wh));
		return alpha2 / (M_PI * pow(ct, 3) * pow(alpha2 + pow(tt, 2), 2)) * J;
	}
	
	inline const float calc_G_SMS( const vec3& lw ) const
	{
		return 2.f / ( 1.f + sqrt( 1.f + pow( m_alpha, 2 ) * ( ( 1.f / pow( lw.z, 2 ) ) - 1.f ) ) );
	}

    col3 f( const vec3& wi ) const override
    {	
        const vec3 h = to_local( normalize( wi + m_wo ) );
		const vec3 lwi = to_local( normalize( wi ) );
			
		//GGX分布
		const float Dggx = pow( m_alpha, 2 ) / ( float( M_PI ) * pow( ( 1.f + ( pow( m_alpha, 2 ) - 1.f ) * pow( h.z, 2 ) ), 2 ) );
		
		//フレネル項
		//const float F = m_F0 + ( 1.f - m_F0 ) * pow( ( 1.f - dot( m_lwo, h ) ), 5);
		const float F = 1.f;//今回のフィッティングではフレネル項は1

		//幾何項
		const float G_l = calc_G_SMS(lwi);
		const float G_v = calc_G_SMS(m_lwo);
		const float G = G_l * G_v;

		const float dot_ln = lwi.z;
		const float dot_vn = m_lwo.z;
		
        return ( dot_ln > 0.f & dot_vn > 0.f ) ? Dggx * G * F / ( 4.f * dot_ln * dot_vn ) * col3( 1.f, 1.f, 1.f ) : col3();
    }

	col3 f_with_cos( const vec3& wi ) const
    {	
        const vec3 h = to_local( normalize( wi + m_wo ) );
		const vec3 lwi = to_local( normalize( wi ) );
		
		//GGX分布
		const float Dggx = pow( m_alpha, 2 ) / ( float( M_PI ) * pow( ( 1.f + ( pow( m_alpha, 2 ) - 1.f ) * pow( h.z, 2 ) ), 2 ) );
		
		//フレネル項
		//const float F = m_F0 + ( 1.f - m_F0 ) * pow( ( 1.f - dot( m_lwo, h ) ), 5);
		const float F = 1.f;//今回のフィッティングではフレネル項は1

		//幾何項
		const float G_l = calc_G_SMS(lwi);
		const float G_v = calc_G_SMS(m_lwo);
		const float G = G_l * G_v;

		const float dot_ln = lwi.z;
		const float dot_vn = m_lwo.z;
		
		return ( dot_ln > 0.f ) ? Dggx * G * F / ( 4.f * dot_vn ) * col3( 1.f, 1.f, 1.f ) : col3();//余弦の重み付きver
    }

private:
    const float m_alpha;
    const float m_F0;

	//texture light用
	vcl::vec3 m_gv;//globalなシェーディング点の座標
	vcl::vec3 m_lp0;//localなpolygon[0]の座標
	vcl::vec3 m_u_coord;//local座標におけるU軸
	vcl::vec3 m_v_coord;//local座標におけるV軸
	vcl::vec3 m_ortho_lv;//local座標における直交方向ベクトルの交点座標
	vcl::vec2 m_ortho_uv;//UV座標空間における直交方向ベクトルの交点のUV座標
	float m_LOD;//LOD
	float m_poly_w;//local座標におけるpolyのwidth
	float m_poly_h;//local座標におけるpolyのheight

	//env map用
	std::array<vcl::vec2, 6> m_ortho_uv_for_env_map;
	std::array<float, 6> m_LOD_for_env_map;
};

}

#endif //_BRDF_GGX_HPP_
