
#ifndef _SRC_PRT_EDGETREE_RENDERER_HPP_
#define _SRC_PRT_EDGETREE_RENDERER_HPP_

#include<iostream>
#include<string>
#include<algorithm>
#include<numeric>

#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/matrix.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtc/matrix_transform.hpp>

#include"../math/vec3.hpp"
#include"../gl/objloader.hpp"
#include"../gl/glslprogram.hpp"
#include"../ltc/ltc.hpp"
#include"../polygon/polygon.hpp"
#include"../polygon/polygon3d.hpp"
#include"../tgaloader/tgaloader.hpp"
#include"../bspvt/bspvt.hpp"
#include"../brdf/ggx.hpp"

namespace vcl {

namespace rndr {

namespace experimental {

using etree = vcl::bspvt::tree< float >;
using enode = vcl::bspvt::node< float >;
using polygon = vcl::experimental::polygon;

class renderer
{

public:

	//コンストラクタ
	renderer( const std::string& path, const std::string& model, const int framebuffer_size_x, const int framebuffer_size_y, const std::string& light_tex_filename = "", const std::string& envmap_filename ="", const size_t nt = std::thread::hardware_concurrency() );

	//描画
	void display( const glm::mat4x4& mvp ) const;

	void calc_radiance( const vec3& eye, const std::vector< vec3 >& polygon, const col3& le, ltc::LTC< float >& ltc );

	//視点から可視な頂点の計算
	void calc_visible_vertices( const glm::mat4x4& mvp );

	template< class T >
	col3 calc_radiance( const std::unique_ptr< enode >& node, const polygon& p, const T& brdf, const std::tuple< glm::mat3, float >& ltc, const uint8_t& face ) const;

	void load_edge_tree( const std::string& filename );

	//放射照度を計算
	float calc_irradiance( const std::unique_ptr< enode >& node, const polygon& p, const glm::mat3& mat, const uint8_t& face, uint16_t& ni, uint16_t& nl ) const;

	//環境照明用の可視領域の計算
	void calc_visible_domain_for_envmap();

	//環境照明用の輝度計算
	col3 calc_radiance_envmap( const std::vector< polygon >& visp, const std::vector< float >& data, const glm::mat3& mat, const float norm, const std::array< float, 3 >& texcoord, const uint8_t& face );

	bool use_envmap() const { return m_envmap_flag; }

	const std::unique_ptr< uint16_t [] >& inner_node_traversal() const { return m_ni; }
	const std::unique_ptr< uint16_t [] >& leaf_node_traversal()  const { return m_nl; }
	const std::vector< int >& visible_vertex_list() const { return m_visible_vertex_id; }
	size_t visible_vertex_size() const { return m_visible_vertex_num; }

	//
	const size_t material_size() const { return m_nm; }
	const std::unique_ptr< std::string [] >& material_name() const { return m_mat_name; }
	void set_kd( const size_t id, const col3& col ) { assert( id < m_nm ); m_kd[ id ] = col; }
	void set_ka( const size_t id, const float alpha ) { assert( id < m_nm && 1e-2f < alpha && alpha <= 1.f ); m_ka[ id ].x = alpha; }
	col3 kd( const size_t i ) const { return m_kd[ i ]; }
	float alpha( const size_t i ) const { return m_ka[ i ].x; }
	void set_tex_light_flag( const bool flag ) { m_tex_light_flag = flag; }
	void set_env_light_flag( const bool flag ) { m_envmap_flag = flag; }
	void set_envmap_scale( const float scale ) { m_envmap_scale = scale; }

private:

	size_t                                  m_nt; //number of threads
	std::unique_ptr< vec3 [] >              m_vertex;
	std::unique_ptr< vec3 [] >              m_normal;
	std::unique_ptr< vec2 [] >              m_texcoord;
	std::unique_ptr< col3 [] >              m_radiance;
	std::unique_ptr< unsigned int [] >      m_index;
	std::unique_ptr< etree [] >             m_tree;
	std::unique_ptr< uint16_t [] >          m_ni;
	std::unique_ptr< uint16_t [] >          m_nl;


	size_t                                  m_nv; //number of total vertices
	size_t                                  m_nf; //number of total triangles
	std::vector< size_t >                   m_voffset;
	std::vector< size_t >                   m_toffset;

	//material
	size_t                                  m_nm; //number of mesh groups
	std::unique_ptr< uint8_t [] >           m_mat;
	std::unique_ptr< col3 [] >              m_kd;
	std::unique_ptr< col3 [] >              m_ks;
	std::unique_ptr< col3 [] >              m_ka; //used for alpha parameter for GGX
	std::unique_ptr< GLuint [] >            m_tex_id;
	std::unique_ptr< std::string [] >       m_mat_name;

	//
	GLuint                                  m_vao_id;
	GLuint                                  m_vbo_id[ 4 ];
	std::unique_ptr< GLuint [] >            m_vbo_index;
	std::unique_ptr< vcl::glslprogram >     m_shader;
	GLint                                   m_shader_mvp_id;
	GLint                                   m_shader_tex_id;

	//variables for texture light
	bool                                    m_tex_light_flag;   //テクスチャ光源を使用するか否か
	std::unique_ptr< TGAImage >             m_tex_light;        //テクスチャ光源用の画像
//	std::array< float, 4 >                  m_poly_coef;        //(textured) polygonal lightの平面の式の係数

	//variables for environment lighting
	bool                                                            m_envmap_flag;
	std::unique_ptr< std::array< std::vector< polygon >, 6 > [] >   m_visible_domain;
	std::unique_ptr< std::array< std::vector< float >,   6 > [] >   m_visible_domain_data;
	std::unique_ptr< TGAImage [] >                                  m_envmap;
	float                                                           m_envmap_scale;
	const std::array< std::array< vcl::vec3, 4 >, 6 > m_env_poly =
	{
		std::array{vcl::vec3( -1.f, +1.f, +1.f ), vcl::vec3( -1.f, -1.f, +1.f ), vcl::vec3( -1.f, -1.f, -1.f ), vcl::vec3( -1.f, +1.f, -1.f ) },//NX
		std::array{vcl::vec3( +1.f, +1.f, -1.f ), vcl::vec3( +1.f, -1.f, -1.f ), vcl::vec3( +1.f, -1.f, +1.f ), vcl::vec3( +1.f, +1.f, +1.f ) },//PX
		std::array{vcl::vec3( -1.f, -1.f, -1.f ), vcl::vec3( -1.f, -1.f, +1.f ), vcl::vec3( +1.f, -1.f, +1.f ), vcl::vec3( +1.f, -1.f, -1.f ) },//NY
		std::array{vcl::vec3( -1.f, +1.f, +1.f ), vcl::vec3( -1.f, +1.f, -1.f ), vcl::vec3( +1.f, +1.f, -1.f ), vcl::vec3( +1.f, +1.f, +1.f ) },//PY
		std::array{vcl::vec3( -1.f, +1.f, -1.f ), vcl::vec3( -1.f, -1.f, -1.f ), vcl::vec3( +1.f, -1.f, -1.f ), vcl::vec3( +1.f, +1.f, -1.f ) },//NZ
		std::array{vcl::vec3( +1.f, +1.f, +1.f ), vcl::vec3( +1.f, -1.f, +1.f ), vcl::vec3( -1.f, -1.f, +1.f ), vcl::vec3( -1.f, +1.f, +1.f ) },//PZ
	};

	//variables for off-screen rendering using framebuffer object
	GLuint                                  m_framebuffer_id;
	int                                     m_framebuffer_size_x;
	int                                     m_framebuffer_size_y;
	GLuint                                  m_color_buffer_tex_id;
	GLuint                                  m_depth_buffer_tex_id;
	std::vector< GLint >                    m_triangle_buffer;
	std::vector< bool  >                    m_visible_vertex_flag;
	std::vector< int   >                    m_visible_vertex_id;
	size_t                                  m_visible_vertex_num;
	GLuint                                  m_fb_vao_id;
	GLuint                                  m_fb_vbo_id;
	GLuint                                  m_fb_vbo_index_id;
	GLint                                   m_fb_mvp;
	std::unique_ptr< vcl::glslprogram >     m_fb_shader;


	// calculate incident direction for (x,y) on the cubemap face x,y ¥in [-1,1]
	inline vec3 omega_i( const float x, const float y, const uint8_t face ) const
	{
		switch( face ) {
		case 0: //NX
			return normalize( vec3( - 1.f, - y,   x ) );
		case 1: //PX
			return normalize( vec3(   1.f, - y, - x ) );
		case 2: //NY
			return normalize( vec3( x, - 1.f, - y ) );
		case 3: //PY
			return normalize( vec3( x,   1.f,   y ) );
		case 4: //NZ
			return normalize( vec3( - x, - y, - 1.f ) );
		case 5: //PZ
			return normalize( vec3(   x, - y,   1.f ) );
		default:
			return vec3();
		}
	}

	inline glm::vec3 omega( const float& x, const float& y, const uint8_t& face ) const
	{
		switch( face ) {
		case 0: //NX
			return glm::vec3( - 1.f, - y,   x );
		case 1: //PX
			return glm::vec3(   1.f, - y, - x );
		case 2: //NY
			return glm::vec3( x, - 1.f, - y );
		case 3: //PY
			return glm::vec3( x,   1.f,   y );
		case 4: //NZ
			return glm::vec3( - x, - y, - 1.f );
		case 5: //PZ
			return glm::vec3(   x, - y,   1.f );
		default:
			return glm::vec3();
		}
	}

	//inline std::array< polygon, 6 > projection( const std::array< std::vector< vec3 >, 6 >& input ) const;
	inline std::array< polygon, 6 > projection( const std::array< polygon3d, 6 >& input ) const;

	inline void init_framebuffer();

	void calc_visible_domain( const std::unique_ptr< enode >& node, const polygon& p, std::vector< polygon > &visible_domain, std::vector< float > &visible_domain_data );

	//テクスチャ光源のテクスチャ座標を計算
	std::array< float, 3 >  calc_light_texcoord( const glm::mat3& mat, const glm::mat3& mat_inv, const std::vector< vcl::vec3 >& p ) const;

	//環境マップのテクスチャ座標を計算
	template< class T > std::array< float, 3 >  calc_envmap_texcoord( const glm::mat3& mat, const glm::mat3& mat_inv, const T& brdf, const uint8_t face ) const;


};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} //edgetree

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} //rndr

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} //vcl

#include"renderer-impl.hpp"

#endif //_SRC_PRT_EDGETREE_RENDERER_HPP_
