//#include"renderer.hpp"

namespace vcl {

namespace rndr {

namespace experimental {

#define BUFFER_OFFSET( bytes ) ( ( GLubyte * ) NULL + ( bytes ) )

#if defined( _WIN32 )
#undef LoadImage
#endif

//コンストラクタ
inline renderer::renderer( const std::string& path, const std::string& filename, const int framebuffer_size_x, const int framebuffer_size_y, const std::string& light_tex_filename, const std::string& envmap_dir, const size_t nt ) : m_nt( nt ), m_framebuffer_size_x( framebuffer_size_x ), m_framebuffer_size_y( framebuffer_size_y ), m_tex_light( nullptr )
{
	std::vector< obj::mesh > mesh = obj::loadobj( path, filename );
	m_nv = std::accumulate( mesh.begin(), mesh.end(), 0u, []( size_t init, const obj::mesh& m ){ return init + m.positions.size(); } );
	m_nf = std::accumulate( mesh.begin(), mesh.end(), 0u, []( size_t init, const obj::mesh& m ){ return init + m.triangles.size(); } );
	m_nm = mesh.size();
	print( m_nf );
	m_radiance      = std::make_unique< col3 [] >( m_nv );
	m_vertex        = std::make_unique< vec3 [] >( m_nv );
	m_normal        = std::make_unique< vec3 [] >( m_nv );
	m_texcoord      = std::make_unique< vec2 [] >( m_nv );
	m_index         = std::make_unique< unsigned int [] >( 3 * m_nf );
	m_tree          = std::make_unique< etree [] >( 6 * m_nv );
	m_ni            = std::make_unique< uint16_t [] >( m_nv );
	m_nl            = std::make_unique< uint16_t [] >( m_nv );
	m_voffset.resize( m_nm );
	m_toffset.resize( m_nm );
	m_triangle_buffer.resize( m_framebuffer_size_x * m_framebuffer_size_y );
	m_visible_vertex_flag.resize( m_nv );
	m_visible_vertex_id.resize( m_nv );

	//material
	m_vbo_index = std::make_unique< GLuint [] >( m_nm );
	m_mat       = std::make_unique< uint8_t [] >( m_nv );
	m_kd        = std::make_unique< col3 [] >( m_nm );
	m_ks        = std::make_unique< col3 [] >( m_nm );
	m_ka        = std::make_unique< col3 [] >( m_nm );
	m_tex_id    = std::make_unique< GLuint [] >( m_nm );
	m_mat_name  = std::make_unique< std::string [] >( m_nm );

	size_t vid = 0, tid = 0, mid = 0;
	for( const auto& m : mesh ) {
		//print( m.material.name );
		for( size_t i = 0, n = m.positions.size(); i < n; ++i ) {
			m_vertex[ vid ]     = { m.positions[ i ].x, m.positions[ i ].y, m.positions[ i ].z };
			m_normal[ vid ]     = { m.normals[ i ].x  , m.normals[ i ].y  , m.normals[ i ].z   };
//			m_texcoord[ vid ]   = ( m.texcoords.size() > 0 )? vec2( m.positions[ i ].x, m.positions[ i ].z ) : vec2();
			m_texcoord[ vid ]   = ( m.texcoords.size() > 0 )? vec2( m.texcoords[ i ].x, m.texcoords[ i ].y ) : vec2();
			m_mat[ vid ]        = mid;
			vid++;
		}
		for( size_t i = 0, n = m.triangles.size(); i < n; ++i ) {
			m_index[ 3 * tid + 0 ] = static_cast< unsigned int >( m.triangles[ i ].i );
			m_index[ 3 * tid + 1 ] = static_cast< unsigned int >( m.triangles[ i ].j );
			m_index[ 3 * tid + 2 ] = static_cast< unsigned int >( m.triangles[ i ].k );
			tid++;
		}

		//材質をmtlファイルから読み込む
		m_kd[ mid ] = col3( m.material.Kd.r, m.material.Kd.g, m.material.Kd.b );
		m_ks[ mid ] = col3( m.material.Ks.r, m.material.Ks.g, m.material.Ks.b );
		//m_ka[ mid ] = col3( m.material.Ka.r, m.material.Ka.g, m.material.Ka.b );
		m_ka[ mid ] = col3( 1.f, 1.f, 1.f );
		m_mat_name[ mid ] = m.material.name;
		m_voffset[ mid ] = m.positions.size();
		m_toffset[ mid ] = m.triangles.size();

		m_tex_id[ mid ] = GLuint( - 1 );
		if( m.material.map_Kd.size() > 0 ) {
			TGAImage map_Kd;
			glGenTextures( 1, &m_tex_id[ mid ] );
			map_Kd.LoadImage( ( path + m.material.map_Kd ).c_str() );
			glBindTexture( GL_TEXTURE_2D, m_tex_id[ mid ] );
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, map_Kd.w(), map_Kd.h(), 0, GL_RGB, GL_UNSIGNED_BYTE, map_Kd.data().get() );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
		} else {
			//dummy texture
			GLubyte dummy[ 3 ] = { 255, 255, 255 };
			glGenTextures( 1, &m_tex_id[ mid ] );
			glBindTexture( GL_TEXTURE_2D, m_tex_id[ mid ] );
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, dummy );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
		}

		mid++;
	}

	//VBOの設定
	glGenBuffers( 4, m_vbo_id );
	glBindBuffer( GL_ARRAY_BUFFER, m_vbo_id[ 0 ] );
	glBufferData( GL_ARRAY_BUFFER, m_nv * sizeof( vec3 ), m_vertex.get(), GL_STATIC_DRAW );

	glBindBuffer( GL_ARRAY_BUFFER, m_vbo_id[ 1 ] );
	glBufferData( GL_ARRAY_BUFFER, m_nv * sizeof( vec3 ), m_normal.get(), GL_STATIC_DRAW );

	glBindBuffer( GL_ARRAY_BUFFER, m_vbo_id[ 2 ] );
	glBufferData( GL_ARRAY_BUFFER, m_nv * sizeof( col3 ), m_radiance.get(), GL_STATIC_DRAW );

	glBindBuffer( GL_ARRAY_BUFFER, m_vbo_id[ 3 ] );
	glBufferData( GL_ARRAY_BUFFER, m_nv * sizeof( vec2 ), m_texcoord.get(), GL_STATIC_DRAW );

	glGenBuffers( m_nm, m_vbo_index.get() );
	size_t j = 0;
	for( size_t i = 0; i < m_nm; ++i ) {
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_vbo_index[ i ] );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, m_toffset[ i ] * 3 * sizeof( GLuint ), &m_index[ j ], GL_STATIC_DRAW );
		j += ( 3 * m_toffset[ i ] );
	}
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

	glEnableVertexAttribArray( 3 );
	glBindBuffer( GL_ARRAY_BUFFER, m_vbo_id[ 3 ] );
	glVertexAttribPointer( 3, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( 0 ) );

	glBindVertexArray( 0 );

	m_shader        = std::make_unique< vcl::glslprogram >( "../glsl/shader_main.vert", "../glsl/shader_main.frag" );
	m_shader_mvp_id = glGetUniformLocation( m_shader->program_handle(), "mvp"  ); if( m_shader_mvp_id < 0 ) std::cerr << "cannot get uniform location for mvp in m_shader"  << std::endl;
	m_shader_tex_id = glGetUniformLocation( m_shader->program_handle(), "tex0" ); if( m_shader_tex_id < 0 ) std::cerr << "cannot get uniform location for tex0 in m_shader" << std::endl;

	init_framebuffer();

	//テクスチャ光源の読み込み・設定
	if( light_tex_filename.size() > 0 ) {
		m_tex_light_flag    = true;
		m_tex_light         = std::make_unique< TGAImage >();
		m_tex_light->LoadImage( ( light_tex_filename + ".tga" ).c_str() );
		m_tex_light->LoadTex_with_margin( ( light_tex_filename + ".tex" ).c_str() );
	} else {
		m_tex_light_flag = false;
	}

	//環境照明用のデータ
	m_visible_domain        = std::make_unique< std::array< std::vector< polygon >, 6 > [] >( m_nv );
	m_visible_domain_data   = std::make_unique< std::array< std::vector< float   >, 6 > [] >( m_nv );
	if( envmap_dir.size() > 0 ) {
		m_envmap_flag   = true;
		m_envmap_scale  = 1.f;
		m_envmap        = std::make_unique< TGAImage [] >( 6 );
		m_envmap[ 0 ].LoadImage( ( envmap_dir + "negx.tga" ).c_str() );
		m_envmap[ 1 ].LoadImage( ( envmap_dir + "posx.tga" ).c_str() );
		m_envmap[ 2 ].LoadImage( ( envmap_dir + "negy.tga" ).c_str() );
		m_envmap[ 3 ].LoadImage( ( envmap_dir + "posy.tga" ).c_str() );
		m_envmap[ 4 ].LoadImage( ( envmap_dir + "negz.tga" ).c_str() );
		m_envmap[ 5 ].LoadImage( ( envmap_dir + "posz.tga" ).c_str() );
		m_envmap[ 0 ].LoadTex_with_margin( ( envmap_dir + "negx.tex" ).c_str() );
		m_envmap[ 1 ].LoadTex_with_margin( ( envmap_dir + "posx.tex" ).c_str() );
		m_envmap[ 2 ].LoadTex_with_margin( ( envmap_dir + "negy.tex" ).c_str() );
		m_envmap[ 3 ].LoadTex_with_margin( ( envmap_dir + "posy.tex" ).c_str() );
		m_envmap[ 4 ].LoadTex_with_margin( ( envmap_dir + "negz.tex" ).c_str() );
		m_envmap[ 5 ].LoadTex_with_margin( ( envmap_dir + "posz.tex" ).c_str() );
	} else {
		m_envmap_flag   = false;
		m_envmap        = nullptr;
		m_envmap_scale  = 1.f;
	}
}



//
inline void renderer::display( const glm::mat4x4& mvp ) const
{
	m_shader->bind();
	m_shader->use();

	glUniformMatrix4fv( m_shader_mvp_id, 1, GL_FALSE, glm::value_ptr( mvp ) );

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

	//texcoord
	glEnableVertexAttribArray( 3 );
	glBindBuffer( GL_ARRAY_BUFFER, m_vbo_id[ 3 ] );
	glVertexAttribPointer( 3, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( 0 ) );

	glEnable( GL_TEXTURE_2D );
	for( size_t i = 0; i < m_nm; ++i ) {
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, m_tex_id[ i ] );
		glUniform1i( m_shader_tex_id, 0 );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_vbo_index[ i ] );
		glDrawElements( GL_TRIANGLES, GLsizei( 3 * m_toffset[ i ] ), GL_UNSIGNED_INT, BUFFER_OFFSET( 0 ) );
	}
	glDisable( GL_TEXTURE_2D );

	glDisableVertexAttribArray( 3 );
	glDisableVertexAttribArray( 2 );
	glDisableVertexAttribArray( 1 );
	glDisableVertexAttribArray( 0 );

	glBindVertexArray( 0 );
}

//
inline void renderer::load_edge_tree( const std::string& filename )
{
	std::ifstream fin( filename, std::ios::binary );
	if( fin.fail() ) {
		std::cerr << "cannot open file " << filename << "\n";
		std::exit( - 1 );
	}
	size_t level = 0;
	size_t num_vertices;
	fin.read( ( char* ) &level, sizeof( unsigned char ) );
	fin.read( ( char* ) &num_vertices, sizeof( size_t ) );

	if( num_vertices != m_nv ) {
		std::cerr << "num of vertices : " << num_vertices << " is not equal to that in the wavefront file.\n";
		std::exit( -1 );
	}
	std::cout << "loading bspvt....\n";
	size_t v0_num = 0;
	size_t v1_num = 0;
	size_t vx_num = 0;
	size_t sum_depth = 0;
	size_t max_leaf_num = 0;
	size_t max_depth = 0;
	for( size_t i = 0, n = 6 * m_nv; i < n; ++i ) {
		m_tree[ i ].load( fin );
		max_leaf_num = std::max( max_leaf_num, m_tree[ i ].v0() + m_tree[ i ].v1() + m_tree[ i ].vx() );
		v0_num += m_tree[ i ].v0();
		v1_num += m_tree[ i ].v1();
		vx_num += m_tree[ i ].vx();
		sum_depth += m_tree[ i ].depth();
		max_depth = std::max( max_depth, static_cast< size_t >( m_tree[ i ].depth() ) );
	}
	fin.close();

	std::cout << "# of vertices : " << m_nv                                             << std::endl;
	std::cout << "total_leaf_num: " << v0_num + v1_num + vx_num                         << std::endl;
	std::cout << "#leaf of V = 0: " << v0_num                                           << std::endl;
	std::cout << "#leaf of V = 1: " << v1_num                                           << std::endl;
	std::cout << "#leaf in (0,1): " << vx_num                                           << std::endl;
	std::cout << " max leaf  num: " << max_leaf_num                                     << std::endl;
	std::cout << " max depth    : " << max_depth                                        << std::endl;
	std::cout << "#avg. leaf    : " << ( v0_num + v1_num + vx_num ) / float( 6 * m_nv ) << std::endl;
	std::cout << "#avg. depth   : " << sum_depth / float( 6 * m_nv )                    << std::endl;
}

//
template< class T >
inline col3 renderer::calc_radiance( const std::unique_ptr< enode >& node, const polygon& p, const T& brdf, const std::tuple< glm::mat3, float >& ltc, const uint8_t& face ) const
{
	if( node->is_leaf() ) {
		if( node->m_data == 0.f ) return col3();

		float e = 0.f;
		std::array< glm::vec3, polygon::max_size > pp;
		const auto invmat = std::get< 0 >( ltc );
		const auto norm   = std::get< 1 >( ltc );

		const auto wi = brdf.to_local( omega_i( p[ p.size() - 1 ].x, p[ p.size() - 1 ].y, face ) );
		pp[ p.size() - 1 ] = glm::normalize( invmat * glm::vec3( wi.x, wi.y, wi.z ) );
		for( uint8_t i = 0, n = p.size(); i < n; ++i ) {
			const uint8_t j = ( i == 0 ) ? p.size() - 1 : i - 1;
			const auto wi = brdf.to_local( omega_i( p[ i ].x, p[ i ].y, face ) );
			pp[ i ] = glm::normalize( invmat * glm::vec3( wi.x, wi.y, wi.z ) );
			const glm::vec3 cross_pij = glm::normalize( glm::cross( pp[ j ], pp[ i ] ) );
			if ( !std::isnan( cross_pij.z ) ) {
				e += acosf( std::clamp( glm::dot( pp[ j ], pp[ i ] ), -1.f, 1.f ) ) * cross_pij.z;
			}
		}

		e = std::abs( e ) * norm / ( 2.f * M_PI );
		return node->m_data * col3( e, e, e );

	} else {
		const std::array< float, 3 > split_line = { sin( node->m_param[ 0 ] ), - cos( node->m_param[ 0 ] ), node->m_param[ 1 ] };
//		const auto pp = vcl::experimental::split( p, split_line[ 0 ], split_line[ 1 ], split_line[ 2 ] );
		const auto pp = vcl::experimental::anon::split( p, split_line[ 0 ], split_line[ 1 ], split_line[ 2 ] );
//		if( pp[ 0 ].size() == 0 && pp[ 1 ].size() == 0 ) {
//			vcl::experimental::split( p, split_line[ 0 ], split_line[ 1 ], split_line[ 2 ] );
//		}
		col3 rad = {};
		if( pp[ 0 ].size() > 0 && node->m_child[ 0 ] != nullptr ) rad += calc_radiance( node->m_child[ 0 ], pp[ 0 ], brdf, ltc, face );
		if( pp[ 1 ].size() > 0 && node->m_child[ 1 ] != nullptr ) rad += calc_radiance( node->m_child[ 1 ], pp[ 1 ], brdf, ltc, face );

		return rad;
	}
}



// node : BSPVTのノードへのポインタ, p : 光源, mat : LTCの行列, face : cubemapの面(0~5), ni : inner nodeの探索回数, nl : 葉ノードの探索回数
inline float renderer::calc_irradiance( const std::unique_ptr< enode >& node, const polygon& p, const glm::mat3& mat, const uint8_t& face, uint16_t& ni, uint16_t& nl ) const
{
	if( node->is_leaf() ) {
		if( node->m_data == 0.f ) return 0.f;
		float e = 0.f;
		glm::vec3 p0, p1;
		p0 = glm::normalize( mat * omega( p[ p.size() - 1 ].x, p[ p.size() - 1 ].y, face ) );
		for( int i = 0, n = p.size(); i < n; ++i ) {
			p1 = glm::normalize( mat * omega( p[ i ].x, p[ i ].y, face ) );
			const glm::vec3 cross_pij = glm::normalize( glm::cross( p0, p1 ) );
			if ( !std::isnan( cross_pij.z ) ) {
				e += acosf( std::clamp( glm::dot( p0, p1 ), -1.f, 1.f ) ) * cross_pij.z;
			}
			p0 = p1;
		}
		nl++;
		return node->m_data * abs( e );
	} else {
		const auto pp = vcl::experimental::anon::split( p, sin( node->m_param[ 0 ] ), - cos( node->m_param[ 0 ] ), node->m_param[ 1 ] );
		float rad = 0.f;
		if( pp[ 0 ].size() > 2 && node->m_child[ 0 ] != nullptr ) rad += calc_irradiance( node->m_child[ 0 ], pp[ 0 ], mat, face, ni, nl );
		if( pp[ 1 ].size() > 2 && node->m_child[ 1 ] != nullptr ) rad += calc_irradiance( node->m_child[ 1 ], pp[ 1 ], mat, face, ni, nl );
		ni++;
		return rad;
	}
}


//多角形光源による輝度計算
// eye : 視点, polygon : 多角形光源の頂点列, le : 光源の輝度, ltc : LTCのデータ
inline void renderer::calc_radiance( const vec3& eye, const std::vector< vec3 >& polygon, const col3& le, ltc::LTC< float >& ltc )
{
	ns::in_parallel( m_visible_vertex_num, [&]( const size_t list_id ) {
		size_t i = m_visible_vertex_id[ list_id ];
		m_radiance[ i ] = col3();
		//if( m_visible_vertex_flag[ i ] == false ) return;
		const float alpha = m_ka[ m_mat[ i ] ].x;
		const vec3& v = m_vertex[ i ];
		const vec3 wo = normalize( eye - v );
		const float ct = dot( wo, m_normal[ i ] );
		if( ct < 0.f ) return;

		ggx brdf = { wo, m_normal[ i ], alpha };
		const float theta = std::acos( std::clamp( ct, -1.f, 1.f ) );
		const auto ltc_data = ltc.interpolate_Minv_and_Amplitude( theta, alpha );
		const glm::mat3 w2l = glm::transpose( glm::mat3( brdf.t().x, brdf.t().y, brdf.t().z, brdf.b().x, brdf.b().y, brdf.b().z,brdf.n().x, brdf.n().y, brdf.n().z ) );
		const glm::mat3 mat = std::get< 0 >( ltc_data ) * w2l;

		polygon3d poly;
		std::vector< vcl::vec3 > lp; //ローカル座標系に変換した多角形光源の座標
		for( auto &now_p : polygon ) {
			poly.emplace_back( now_p - v );
			lp.emplace_back( brdf.to_local( now_p - v ) );
		}
		const auto p = projection( frustum_clipping( poly ) );

		float irrad = 0.f;
		uint16_t ni = 0, nl = 0;
		for( uint8_t j = 0; j < 6; ++j ) {
			if( ( m_tree[ 6 * i + j ].root()->is_leaf() && m_tree[ 6 * i + j ].root()->m_data == 0.f ) || p[ j ].size() <= 2 ) continue;
			const auto e = calc_irradiance( m_tree[ 6 * i + j ].root(), p[ j ], mat, j, ni, nl );
			assert( std::isfinite( e ) );
			irrad += ( e * std::get< 1 >( ltc_data ) / ( 2.f * M_PI ) );
		}
		m_radiance[ i ] = ( irrad * le * m_kd[ m_mat[ i ] ] );
		m_ni[ i ] = ni;
		m_nl[ i ] = nl;

		if( m_tex_light_flag ) {
			const auto ltc_matrix   = ltc.interpolate_M( theta, alpha );
			const auto texcoord     = calc_light_texcoord( ltc_matrix, std::get< 0 >( ltc_data ), lp );
			const auto tex          = m_tex_light->sampleLOD_with_margin( texcoord[ 2 ], texcoord[ 0 ], texcoord[ 1 ] );
			if( !std::isfinite( tex[ 0 ] ) || !std::isfinite( tex[ 1 ] ) || !std::isfinite( tex[ 2 ] ) ) print( tex[ 0 ] );
			m_radiance[ i ] *= vcl::vec3( tex[ 0 ], tex[ 1 ], tex[ 2 ] );
		}

		//環境照明によるライティング
		if( m_envmap_flag ) {
			col3 rad;
			for( uint8_t j = 0; j < 6; ++ j ) {
				if( m_visible_domain[ i ][ j ].size() == 0 ) continue;
				const auto texcoord = calc_envmap_texcoord( ltc.interpolate_M( theta, alpha ), std::get< 0 >( ltc_data ), brdf, j );
				rad += calc_radiance_envmap( m_visible_domain[ i ][ j ], m_visible_domain_data[ i ][ j ], mat, std::get< 1 >( ltc_data ), texcoord, j );
			}
			m_radiance[ i ] += m_envmap_scale * le * rad * m_kd[ m_mat[ i ] ];
		}
	}, m_nt );
	//
	glBindBuffer( GL_ARRAY_BUFFER, m_vbo_id[ 2 ] );
	glBufferSubData( GL_ARRAY_BUFFER, 0, m_nv * sizeof( col3 ), m_radiance.get() );

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline std::array< polygon, 6 > renderer::projection( const std::array< polygon3d, 6 >& input ) const
{
	constexpr float eps = 0.f; //1e-5f;
	std::array< polygon, 6 > output;

	const auto f = []( const vec3 &v, const int face ) {
		float t;
		switch ( face ) {
		case 0: //NX
			t = -1.f / v.x;
			return vec2( t * v.z, -t * v.y );
		case 1:
			t = 1.f / v.x;
			return vec2( -t * v.z, -t * v.y );
		case 2:
			t = -1.f / v.y;
			return vec2( t * v.x, -t * v.z );
		case 3:
			t = 1.f / v.y;
			return vec2( t * v.x, t * v.z );
		case 4:
			t = -1.f / v.z;
			return vec2( -t * v.x, -t * v.y );
		case 5:
			t = 1.f / v.z;
			return vec2( t * v.x, -t * v.y );
		default:
			return vec2();
		}
	};


	for ( int i = 0; i < 6; ++i ) {
		std::array< vec2, 16 > p;
		//std::array< edge, 16 > ee;
		if( input[ i ].size() == 0 ) continue;
		const size_t n = input[ i ].size();
		for( size_t j = 0; j < n; ++j ) {
			//p[ j ] = f( input[ i ][ j ], i );
			output[ i ].emplace_back( f( input[ i ][ j ], i ) );
		}
		/*
		for ( size_t j = 0; j < n; ++j ) {
			const vec2 p0 = p[ j ];
			const vec2 p1 = ( j == n - 1 ) ? p[ 0 ] : p[ j + 1 ];
			if ( dot( p0 - p1, p0 - p1 ) > eps ) ee[ k++ ] = edge( p0, p1 );
		}
		for ( size_t j = 0; j < k; ++j ) {
			output[ i ].emplace_back( p[ j ] );
		}
		*/
	}
	return output;
}

//
inline void renderer::init_framebuffer()
{
	glGenFramebuffers( 1, &m_framebuffer_id );
	glBindFramebuffer( GL_FRAMEBUFFER, m_framebuffer_id );

	//color buffer
	glGenTextures( 1, &m_color_buffer_tex_id );
	glBindTexture( GL_TEXTURE_2D, m_color_buffer_tex_id );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_R32I, m_framebuffer_size_x, m_framebuffer_size_y, 0, GL_RED_INTEGER, GL_INT, 0 );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glFramebufferTexture( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_color_buffer_tex_id, 0 );

	//depth buffer
	glGenRenderbuffers( 1, &m_depth_buffer_tex_id );
	glBindRenderbuffer( GL_RENDERBUFFER, m_depth_buffer_tex_id );
	glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_framebuffer_size_x, m_framebuffer_size_y );
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth_buffer_tex_id );

	{
		std::unique_ptr< float [] > vb = std::make_unique< float [] >( 9 * m_nf );
		std::unique_ptr< int   [] > ib = std::make_unique< int   [] >( 3 * m_nf );

		for( size_t i = 0, n = m_nf; i < n; ++i ) {
			const auto id0 = m_index[ 3 * i + 0 ];
			const auto id1 = m_index[ 3 * i + 1 ];
			const auto id2 = m_index[ 3 * i + 2 ];
			vb[ 9 * i + 0 ] = m_vertex[ id0 ].x;
			vb[ 9 * i + 1 ] = m_vertex[ id0 ].y;
			vb[ 9 * i + 2 ] = m_vertex[ id0 ].z;
			vb[ 9 * i + 3 ] = m_vertex[ id1 ].x;
			vb[ 9 * i + 4 ] = m_vertex[ id1 ].y;
			vb[ 9 * i + 5 ] = m_vertex[ id1 ].z;
			vb[ 9 * i + 6 ] = m_vertex[ id2 ].x;
			vb[ 9 * i + 7 ] = m_vertex[ id2 ].y;
			vb[ 9 * i + 8 ] = m_vertex[ id2 ].z;
			ib[ 3 * i + 0 ] = ib[ 3 * i + 1 ] = ib[ 3 * i + 2 ] = static_cast< int >( i );
		}

		//VAO
		glGenVertexArrays( 1, &m_fb_vao_id );
		glBindVertexArray( m_fb_vao_id );

		//VBO
		glGenBuffers( 1, &m_fb_vbo_id );
		glBindBuffer( GL_ARRAY_BUFFER, m_fb_vbo_id );
		glBufferData( GL_ARRAY_BUFFER, size_t( m_nf * 3 * 3 * sizeof( float ) ), vb.get(), GL_STATIC_DRAW );

		//face index buffer
		glGenBuffers( 1, &m_fb_vbo_index_id );
		glBindBuffer( GL_ARRAY_BUFFER, m_fb_vbo_index_id );
		glBufferData( GL_ARRAY_BUFFER, size_t( m_nf * 3 * sizeof( int ) ), ib.get(), GL_STATIC_DRAW );

		glBindVertexArray( 0 );
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	}

	m_fb_shader = std::make_unique< vcl::glslprogram >( "../glsl/visible_vertex.vert", "../glsl/visible_vertex.frag" );
	m_fb_mvp = glGetUniformLocation( m_fb_shader->program_handle(), "mvp" ); if( m_fb_mvp < 0 ) std::cerr << "cannot get uniform location for mvp in m_fb_shader" << std::endl;
}


//framebuffer objectを用いた可視頂点の計算
inline void renderer::calc_visible_vertices( const glm::mat4x4& mvp )
{
	glBindFramebuffer( GL_FRAMEBUFFER, m_framebuffer_id );

	m_fb_shader->use();
	m_fb_shader->bind();

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glViewport( 0, 0, m_framebuffer_size_x, m_framebuffer_size_y );

	glUniformMatrix4fv( m_fb_mvp, 1, GL_FALSE, glm::value_ptr( mvp ) );

	glBindVertexArray( m_fb_vao_id );

	glEnableVertexAttribArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, m_fb_vbo_id );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, ( void* ) 0 );

	glEnableVertexAttribArray( 1 );
	glBindBuffer( GL_ARRAY_BUFFER, m_fb_vbo_index_id );
	glVertexAttribIPointer( 1, 1, GL_INT, 0, ( void* ) 0 );


	glDrawArrays( GL_TRIANGLES, 0, 3 * m_nf );

	glReadPixels( 0, 0, m_framebuffer_size_x, m_framebuffer_size_y, GL_RED_INTEGER, GL_INT, m_triangle_buffer.data() );

	//for( size_t i = 0; i < m_nv; ++i ) m_visible_vertex_flag[ i ] = false;
	std::fill( m_visible_vertex_flag.begin(), m_visible_vertex_flag.end(), false );

	for( size_t y = 0; y < m_framebuffer_size_y; ++y ) {
		for( size_t x = 0; x < m_framebuffer_size_x; ++x ) {
			const auto tid = m_triangle_buffer[ y * m_framebuffer_size_x + x ];
			m_visible_vertex_flag[ m_index[ 3 * tid + 0 ] ] = true;
			m_visible_vertex_flag[ m_index[ 3 * tid + 1 ] ] = true;
			m_visible_vertex_flag[ m_index[ 3 * tid + 2 ] ] = true;
		}
	}

	m_visible_vertex_num = 0;
	for( size_t i = 0; i < m_nv; ++i ) {
		m_ni[ i ] = 0;
		m_nl[ i ] = 0;
		if( m_visible_vertex_flag[ i ] ) m_visible_vertex_id[ m_visible_vertex_num++ ] = i;
	}
	print( m_visible_vertex_num );

	glDisableVertexAttribArray( 1 );
	glDisableVertexAttribArray( 0 );
	glBindVertexArray( 0 );

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	m_fb_shader->unbind();
}

//テクスチャ光源のテクスチャ座標を計算
// mat : LTCの行列
// mat_inv : LTCの逆行列
// p : ローカル座標系
inline std::array< float, 3 > renderer::calc_light_texcoord( const glm::mat3& mat, const glm::mat3& mat_inv, const std::vector< vcl::vec3 >& p ) const
{
	//polygonをlocal座標に変換&cos空間に変換
	std::array<vcl::vec3, 4> l_poly;
	std::array< glm::vec3, 4 > cos_poly;
	for( int i = 0; i < 4; ++i ){
		l_poly[ i ] = p[ i ];
		cos_poly[ i ] = mat_inv * glm::vec3( p[ i ].x, p[ i ].y, p[ i ].z );
	}
	//cos空間における平面の方程式を計算
	const glm::vec3 cos_poly_coef_abc = glm::cross(cos_poly[1] - cos_poly[0], cos_poly[3] - cos_poly[0]);
	const float cos_poly_coef_d = -(cos_poly_coef_abc.x * cos_poly[0].x + cos_poly_coef_abc.y * cos_poly[0].y + cos_poly_coef_abc.z * cos_poly[0].z);
	//平面に垂直な方向ベクトルと平面との交点を計算
	const float cos_t = -cos_poly_coef_d / (cos_poly_coef_abc.x * cos_poly_coef_abc.x + cos_poly_coef_abc.y * cos_poly_coef_abc.y + cos_poly_coef_abc.z * cos_poly_coef_abc.z);
	const glm::vec3 ortho_cos_v = cos_poly_coef_abc * cos_t;
	//交点座標をlocal座標に戻す
	const glm::vec3 ortho_lv_tmp = mat * ortho_cos_v;
	const vcl::vec3 ortho_lv( ortho_lv_tmp.x, ortho_lv_tmp.y, ortho_lv_tmp.z );
	//UV座標に変換
	const float poly_w = norm(l_poly[3] - l_poly[0] );
	const float poly_h = norm(l_poly[1] - l_poly[0] );
	const vcl::vec3 u_coord = normalize(l_poly[3] - l_poly[0] );
	const vcl::vec3 v_coord = normalize(l_poly[1] - l_poly[0] );
	const vcl::vec2 ortho_uv = vcl::vec2{dot(ortho_lv - l_poly[0], u_coord) / poly_w, dot(ortho_lv - l_poly[0], v_coord) / poly_h};

	//テクスチャライトの面積を計算
	const float light_area = glm::dot(cos_poly_coef_abc, cos_poly_coef_abc);

	//シェーディング点からテクスチャライトまでの距離を計算
	const float r = glm::dot( cos_poly_coef_abc, cos_poly[0]);

	//LODを計算
	const float LOD = log2(float( m_tex_light->w() ) * ( abs( r ) / pow( light_area, 0.75f ) ) ) + 1.f;
	return { ortho_uv.x, ortho_uv.y, LOD };
}

//環境マップ(キューブマップ)の可視領域(V>0)を計算する関数
inline void renderer::calc_visible_domain_for_envmap()
{
	if( not( m_envmap_flag ) ) return;
	ns::in_parallel( m_nv, [ & ]( const size_t i ){
		polygon p;
		p.emplace_back( vec2( - 1.f, - 1.f ) );
		p.emplace_back( vec2(   1.f, - 1.f ) );
		p.emplace_back( vec2(   1.f,   1.f ) );
		p.emplace_back( vec2( - 1.f,   1.f ) );
		for( size_t j = 0; j < 6; ++j )
			calc_visible_domain( m_tree[ 6 * i + j ].root(), p, m_visible_domain[ i ][ j ], m_visible_domain_data[ i ][ j ] );
	}, m_nt );

	size_t visible_domain_size = 0;
	size_t visible_domain_polygon_vertex_size = 0;
	for( size_t i = 0; i < m_nv; ++i ) {
		for( size_t j = 0; j < 6; ++j ) {
			visible_domain_size += m_visible_domain[ i ][ j ].size();
			for( const auto& p : m_visible_domain[ i ][ j ] ) {
				visible_domain_polygon_vertex_size += p.size();
			}
		}
	}
	std::cout << "average visible domain per-vertex : " << double( visible_domain_size )/ double( m_nv ) << std::endl;
	std::cout << "average visible domain polygon vertex size per-vertex : " << double( visible_domain_polygon_vertex_size )/ double( m_nv ) << std::endl;
}

//キューブマップの可視領域をsplit lineで分割する関数
inline void renderer::calc_visible_domain( const std::unique_ptr< enode >& node, const polygon& p, std::vector< polygon >& visible_domain, std::vector< float >& visible_domain_data )
{
	if( node->is_leaf() ) {
		if( node->m_data > 0.f ) {
			visible_domain.emplace_back( p );
			visible_domain_data.emplace_back( node->m_data );
		}
	} else {
		const std::array< float, 3 > split_line = { sin( node->m_param[ 0 ] ), - cos( node->m_param[ 0 ] ), node->m_param[ 1 ] };
		//const auto pp = vcl::experimental::split( p, split_line[ 0 ], split_line[ 1 ], split_line[ 2 ] );
		const auto pp = vcl::experimental::anon::split( p, split_line[ 0 ], split_line[ 1 ], split_line[ 2 ] );
		if( pp[ 0 ].size() > 0 ) calc_visible_domain( node->m_child[ 0 ], pp[ 0 ], visible_domain, visible_domain_data );
		if( pp[ 1 ].size() > 0 ) calc_visible_domain( node->m_child[ 1 ], pp[ 1 ], visible_domain, visible_domain_data );
	}
}

//
template< class T >
inline std::array< float, 3 > renderer::calc_envmap_texcoord( const glm::mat3& mat, const glm::mat3& mat_inv, const T& brdf, const uint8_t face ) const
{
	std::array<vcl::vec3, 4> l_poly;
	std::array<glm::vec3, 4> cos_poly;
	for( int i = 0; i < 4; ++i ){
		l_poly[ i ] = brdf.to_local( m_env_poly[ face ][ i ] );//localに変換
		cos_poly[ i ] = mat_inv * glm::vec3( l_poly[i].x, l_poly[i].y, l_poly[i].z );
	}
	//cos空間における平面の方程式を計算
	const glm::vec3 cos_poly_coef_abc = glm::cross( cos_poly[1] - cos_poly[0], cos_poly[3] - cos_poly[0] );
	const float cos_poly_coef_d = -(cos_poly_coef_abc.x * cos_poly[0].x + cos_poly_coef_abc.y * cos_poly[0].y + cos_poly_coef_abc.z * cos_poly[0].z);
	//平面に垂直な方向ベクトルと平面との交点を計算
	const float cos_t = -cos_poly_coef_d / (cos_poly_coef_abc.x * cos_poly_coef_abc.x + cos_poly_coef_abc.y * cos_poly_coef_abc.y + cos_poly_coef_abc.z * cos_poly_coef_abc.z);
	const glm::vec3 ortho_cos_v = cos_poly_coef_abc * cos_t;
	//交点座標をlocal座標に戻す
	const glm::vec3 ortho_lv_tmp = mat * ortho_cos_v;
	const vcl::vec3 ortho_lv = vcl::vec3( ortho_lv_tmp.x, ortho_lv_tmp.y, ortho_lv_tmp.z );
	//UV座標に変換
	const float poly_w = norm( l_poly[ 3 ] - l_poly[ 0 ] );
	const float poly_h = norm( l_poly[ 1 ] - l_poly[ 0 ] );
	const vcl::vec3 u_coord = normalize( l_poly[ 3 ] - l_poly[ 0 ] );
	const vcl::vec3 v_coord = normalize( l_poly[ 1 ] - l_poly[ 0 ] );
	const vcl::vec2 ortho_uv = vcl::vec2{ dot( ortho_lv - l_poly[ 0 ], u_coord ) / poly_w, dot(ortho_lv - l_poly[ 0 ], v_coord ) / poly_h };

	//テクスチャライトの面積を計算
	const float light_area = glm::dot(cos_poly_coef_abc, cos_poly_coef_abc);

	//シェーディング点からテクスチャライトまでの距離を計算
	const float r = glm::dot(cos_poly_coef_abc, cos_poly[0]);

	//LODを計算
	//+1.fは経験則に基づく補正
	const float LOD = log2(float( m_envmap[ face ].w() ) * (abs(r) / pow(light_area, 0.75f))) + 1.f;
	return { ortho_uv.x, ortho_uv.y, LOD };
}

//
inline col3 renderer::calc_radiance_envmap( const std::vector< polygon >& visp, const std::vector< float >& data, const glm::mat3& mat, const float norm, const std::array< float, 3 >& texcoord, const uint8_t& face )
{
	float irrad = 0.f;
	constexpr size_t polygon_size = 32;
	auto it = data.begin();
	for( auto iter = visp.begin(), end = visp.end(); iter != end; ++iter, ++it ) {
		std::array< glm::vec3, polygon_size > p;
		glm::vec3 p0, p1;
		float s0, s1;
		uint8_t size = 0;
		size_t n = iter->size();
		if( n <= 2 ) continue;
		p0 = mat * omega( ( *iter )[ n - 1 ].x, ( *iter )[ n - 1 ].y, face );
		s0 = p0.z;
		for( uint8_t i = 0; i < n; ++i ) {
			p1 = mat * omega( ( *iter )[ i ].x, ( *iter )[ i ].y, face );
			s1 = p1.z;
			if( s0 < 0.f && s1 >= 0.f ) {
				const float t = ( - s0 ) / ( s1 - s0 ); assert( 0.f <= t && t < 1.f );
				p[ size++ ] = glm::normalize( p0 + t * ( p1 - p0 ) );
			}
			if( s1 >= 0.f ) p[ size++ ] = glm::normalize( p1 );
			if( s0 >= 0.f && s1 < 0.f ) {
				const float t = s0 / ( s0 - s1 ); assert( 0.f <= t && t < 1.f );
				p[ size++ ] = glm::normalize( p0 + t * ( p1 - p0 ) );
			}
			p0 = p1;
			s0 = s1;
		}

		float e = 0.f;
		//size = iter->size();
		for( uint8_t i = 0; i < size; ++i ) {
			const uint8_t j = ( i == size - 1 ) ? 0 : i + 1;
			const glm::vec3 cross_pij = glm::normalize( glm::cross( p[ i ], p[ j ] ) );
			if ( !std::isnan( cross_pij.z ) ) {
				e += acosf( std::clamp( glm::dot( p[ i ], p[ j ] ), -1.f, 1.f ) ) * cross_pij.z;
			}
		}
		irrad += std::abs( e ) * ( *it );
	}
	const auto texel = m_envmap[ face ].sampleLOD_with_margin( texcoord[ 2 ], texcoord[ 0 ], texcoord[ 1 ] );
	return norm / ( 2.f * M_PI ) * vcl::col3( irrad * texel[ 0 ], irrad * texel[ 1 ], irrad * texel[ 2 ] );
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
