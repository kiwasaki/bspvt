
#include<iostream>
#include<cmath>
#include<vector>
#include<array>
#include<chrono>

#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtx/transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include"../math/vec2.hpp"
#include"../math/vec3.hpp"
//#include"../src/prt/edgetree/renderer.hpp"
//#include"../ltc/ltc.hpp"
#include"../ltc/ltc_data.hpp"
#include"../gl/glslprogram.hpp"
#include"../tgaloader/tgaloader.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include"../image/stb_image_write.h"

constexpr int window_size_x = 1280;
constexpr int window_size_y = 720;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//choose rendering scene
#define TEAPOT
//#define DRAGON
//#define LIVING_ROOM

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define USE_TEXTURE_LIGHT
#define USE_ENVMAP

//#define MEASURE_TIME
//#define DISPLAY_TRAVERSAL_COUNT


#ifdef USE_TEXTURE_LIGHT
const std::string texture_light_filename = "../texture_light/stained_glass2";
#else
const std::string texture_light_filename = "";
#endif

#ifdef USE_ENVMAP
#if  defined( _WIN32 )
	const std::string envmap_folder = "../environment_map/LancellottiChapel/";
	#else
	const std::string envmap_folder = "/Users/iwasaki/resource/environment_map/";
	#endif
#else
const std::string envmap_folder = "";
#endif

#ifndef BUFFER_OFFSET
#define BUFFER_OFFSET( bytes ) ( ( GLubyte * ) NULL + ( bytes ) )
#endif

#if defined( TEAPOT )

float camera_theta = 0.616f;
float camera_phi = 0.00600016f;
float camera_z = 2.2f;
float camera_target_z = -1.53f - 2.2f;
//float light_scale = 0.5;
float light_x = 7.45058e-09;
float light_y = -0.08;
float light_z = -0.44;
//const float light_w = 0.5;
const float light_w = 2;
const float light_h = 0.9;
const float light_d = 1;
float scale = 1.f;
//float le = 1.0f; //alpha = 0.10
//float le = 1.5f; //alpha = 0.25
//float le = 2.0f; //alpha = 0.50
float le = 3.5f; //alpha = 1.00
std::vector<vcl::vec3> light = { vcl::vec3(-1, 0.82, 0.06), vcl::vec3(-1, 0.82, -0.94), vcl::vec3(1, 0.82, -0.94), vcl::vec3(1, 0.82, 0.06), };
float cubemap_scale = 20.f;
float camera_x = 0.f;
float camera_y = 0.f;
glm::mat4 light_scale_matrix;
glm::mat4 light_rotate_matrix;
glm::mat4 light_translate_matrix;
glm::vec3 light_angle;
glm::vec3 light_scale = { 0.5f * 2.f, 1.f, 0.5f * 1.f };
glm::vec3 light_translate_min{ - 2.f, 0.f, - 2.f };
glm::vec3 light_translate_max{ + 2.f, +5.f, +2.f };
glm::vec3 light_scale_max{ 3.f, 1.f, 3.f };
#endif

#if defined( DRAGON )
// dragon
float camera_theta = 0.342f;
float camera_phi = -1.92f;
float camera_z = 2.5f;
float camera_target_z = - 1.23f - 2.5f;
//float light_scale = 0.5;
float light_x = -0.36;
float light_y = -0.04;
float light_z = 0.2;
const float light_w = 4;
const float light_h = 0.9;
const float light_d = 2;
float scale = 1;
std::vector<vcl::vec3> light = { vcl::vec3(-2.36, 0.86, 1.2), vcl::vec3(-2.36, 0.86, -0.8), vcl::vec3(1.64, 0.86, -0.8), vcl::vec3(1.64, 0.86, 1.2), };
//float le = 1.0f; //alpha = 0.10
float le = 1.0f; //alpha = 0.25
//float le = 1.0f; //alpha = 0.50
//float le = 1.5f; //alpha = 1.00
float cubemap_scale = 20.f;
float camera_x = 0.f;
float camera_y = 0.f;
glm::mat4 light_scale_matrix;
glm::mat4 light_rotate_matrix;
glm::mat4 light_translate_matrix;
glm::vec3 light_angle;
glm::vec3 light_scale = { 0.5f * 4.f, 1.f, 0.5f * 2.f };
glm::vec3 light_translate_min{ - 2.f, 0.f, - 2.f };
glm::vec3 light_translate_max{ + 2.f, +5.f, +2.f };
glm::vec3 light_scale_max{ 3.f, 1.f, 3.f };
/*
//another view
float camera_x = 0.f;
float camera_theta = 0.442475f; //0.342;
float camera_phi = -5.76135f; //0.442475 -1.92;
float camera_z = 2.5;
float camera_target_z = -1.23;
float light_scale = 0.5;
float light_x = -0.36;
float light_y = -0.04;
float light_z = -1.0f;
const float light_w = 4;
const float light_h = 0.9;
const float light_d = 2;
float scale = 1;
std::vector<vcl::vec3> light = { vcl::vec3(-2.36, 0.86, 1.2), vcl::vec3(-2.36, 0.86, -0.8), vcl::vec3(1.64, 0.86, -0.8), vcl::vec3(1.64, 0.86, 1.2), };

float le = 2.f; //alpha = 1.00
float cubemap_scale = 20.f;
*/
/*
float camera_theta = 0.413299f; //0.616;
float camera_phi = 6.91099f; //0.00600016;
float camera_z = 2.2;
float camera_target_z = -1.53;
float le = 10.f;
float light_scale = 0.5f;
float light_x = 7.45058e-09;
float light_y = -0.08;
float light_z = -0.44;
//const float light_w = 0.5;
const float light_w = 2;
const float light_h = 0.9;
const float light_d = 1;
std::vector<vcl::vec3> light = { vcl::vec3(-1, 0.82, 0.06), vcl::vec3(-1, 0.82, -0.94), vcl::vec3(1, 0.82, -0.94), vcl::vec3(1, 0.82, 0.06), };
*/
#endif

#if defined( LIVING_ROOM )
float camera_theta      = 0.256333f;
float camera_phi        = - M_PI; //-1.89865f;
float camera_z          = 64.4765f;
float camera_target_z   = 60.7466f - 64.4765f;
float le = 20.f;
//float le = 5.f;
//float le = 25.f;
//float light_scale = 8;
float light_x = -20.f;
float light_y = 30.f;
float light_z = -11.f;
//const float light_w = 0.5;
const float light_w = 2;
const float light_h = 0.4;
const float light_d = 1;
std::vector<vcl::vec3> light = { vcl::vec3(-1, 0.82, 0.06), vcl::vec3(-1, 0.82, -0.94), vcl::vec3(1, 0.82, -0.94), vcl::vec3(1, 0.82, 0.06), };
float cubemap_scale = 200.f;
float camera_x = - 30.f;
float camera_y = 5.f;
glm::mat4 light_scale_matrix;
glm::mat4 light_rotate_matrix;
glm::mat4 light_translate_matrix;
glm::vec3 light_angle;
glm::vec3 light_scale = { 8.f * 2.f, 1.f, 8.f * 1.f };
glm::vec3 light_translate_min{ - 60.f,  0.f, - 60.f };
glm::vec3 light_translate_max{ +  0.f, 30.f, +  0.f };
//glm::vec3 light_translate_min{ - 30.f,  0.f, - 30.f };
//glm::vec3 light_translate_max{ + 30.f, 30.f, + 30.f };
glm::vec3 light_scale_max{ 30.f, 1.f, 30.f };
#endif

float alpha = 0.1;
const std::array<float, 4> scale_array = {1.f, 1.5f, 2.f, 3.5f};

vcl::vec3 camera = { camera_x, 0.f, camera_z };
glm::vec3 camera_up( 0.f, 1.f, 0.f );
glm::vec3 camera_target( camera_x, 0.f, camera_z + camera_target_z );
double mouse_x = 0.0;
double mouse_y = 0.0;

GLFWwindow *main_window;

std::unique_ptr< vcl::glslprogram > shader;
GLint mvp_param;
const glm::mat4 projection = glm::perspective( glm::radians( 45.0 ), double( window_size_x ) / double( window_size_y ), 1e-1, 1e3 );

enum struct camera_mode { noop, rotate, pan, walk, light_translate, light_scale, light_rotate_x, light_rotate_y, light_rotate_z };
enum struct mouse_state { up, down };
camera_mode cmode = camera_mode::noop; //camera_mode::rotate;
mouse_state mstate = mouse_state::up;

bool is_update_camera = true;
bool is_update_light  = true;
bool is_display_light = true;
bool texture_light_flag = false;
bool envmap_light_flag = false;
float envmap_scale = 0.5f;

// VBO and VAO for polygonal light
GLuint                      light_vao;
GLuint                      light_vbo[ 4 ];
GLuint                      light_vbo_index;
GLuint                      light_tex_id;
GLint                       light_tex_param;
glm::vec3                   light_translate;
std::vector< vcl::vec3 >    light_init_position;

//VBO an VAO for cubemap
GLuint                                  cubemap_texid[ 6 ];
GLuint                                  cubemap_vao;
GLuint                                  cubemap_vbo[ 4 ];
GLuint                                  cubemap_vbo_index;
std::unique_ptr< vcl::glslprogram >     cubemap_shader;
GLint                                   cubemap_shader_mvp;
GLint                                   cubemap_shader_tex0;

//variables for Dear ImGui
const char* glsl_version = "#version 410";
std::vector< std::string > material_list;

// glfwの初期化
void init_glfw()
{
	if ( !glfwInit() ) {
		std::cerr << "cannot create glfw context.\n";
		exit( -1 );
	}

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
	glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

	main_window = glfwCreateWindow( window_size_x, window_size_y, "bspvt", nullptr, nullptr);
	if ( main_window == nullptr ) {
		std::cerr << "cannot create glfw window.\n";
		glfwTerminate();
		exit( -1 );
	}

	glfwMakeContextCurrent( main_window );
	glfwSwapInterval( 0 );
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LEQUAL );

	glewExperimental = GL_TRUE;
	if ( glewInit() != GLEW_OK ) {
		fprintf( stderr, "Failed to initialize GLEW\n" );
		getchar();
		glfwTerminate();
		exit( -1 );
	}

	shader = std::make_unique< vcl::glslprogram >( "../glsl/light_shader.vert", "../glsl/light_shader.frag" );
	mvp_param = glGetUniformLocation( shader->program_handle(), "mvp" ); if( mvp_param < 0 ) std::cerr << "cannot get uniform location for mvp" << std::endl;
	light_tex_param = glGetUniformLocation( shader->program_handle(), "tex0" ); if( light_tex_param < 0 ) std::cerr << "cannot get uniform location for light_tex_param" << std::endl;
}

//
void init_lookat_param()
{
	constexpr double cor = 1e-3 * 2.0;
	const float ct = cos( camera_theta );
	const float st = sin( camera_theta );
	const float cp = cos( camera_phi );
	const float sp = sin( camera_phi );
	camera[ 0 ] = camera_x + camera_z * ct * sp;
	camera[ 1 ] = camera_y + camera_z * st;
	camera[ 2 ] =            camera_z * ct * cp;
	camera_target[ 0 ] = camera_x + ( camera_z + camera_target_z ) * ct * sp;
	camera_target[ 1 ] = camera_y + ( camera_z + camera_target_z ) * st;
	camera_target[ 2 ] =            ( camera_z + camera_target_z ) * ct * cp;
	camera_up.x = -st * sp;
	camera_up.y = ct;
	camera_up.z = -st * cp;
}

//
void update_camera()
{
	const float ct = cos( camera_theta );
	const float st = sin( camera_theta );
	const float cp = cos( camera_phi );
	const float sp = sin( camera_phi );

	camera[ 0 ] = camera_x + camera_z * ct * sp;
	camera[ 1 ] = camera_y + camera_z * st;
	camera[ 2 ] =            camera_z * ct * cp;

	camera_target[ 0 ] = camera_x + ( camera_z + camera_target_z ) * ct * sp;
	camera_target[ 1 ] = camera_y + ( camera_z + camera_target_z ) * st;
	camera_target[ 2 ] =            ( camera_z + camera_target_z ) * ct * cp;

	camera_up.x = -st * sp;
	camera_up.y = ct;
	camera_up.z = -st * cp;
}



//多角形光源の設定
void init_light_pos()
{
	light_translate = glm::vec3( light_x, light_h + light_y, light_z );
	light_scale_matrix      = glm::scale( light_scale );
	light_rotate_matrix     = glm::mat4( 1.f );
	light_translate_matrix  = glm::translate( light_translate );
	const auto matrix = light_translate_matrix * light_rotate_matrix * light_scale_matrix;
	{
		const auto lv0 = matrix * glm::vec4( - 1.f, 0.f, + 1.f, 1.f );
		const auto lv1 = matrix * glm::vec4( - 1.f, 0.f, - 1.f, 1.f );
		const auto lv2 = matrix * glm::vec4( + 1.f, 0.f, - 1.f, 1.f );
		const auto lv3 = matrix * glm::vec4( + 1.f, 0.f, + 1.f, 1.f );
		light[ 0 ] = { lv0.x, lv0.y, lv0.z };
		light[ 1 ] = { lv1.x, lv1.y, lv1.z };
		light[ 2 ] = { lv2.x, lv2.y, lv2.z };
		light[ 3 ] = { lv3.x, lv3.y, lv3.z };
	}
	//std::cout << light[ 0 ].x << ", " << light[ 0 ].y << ", " << light[ 0 ].z << std::endl;
	/*
	light[ 0 ].x = -light_w * light_scale + light_x;
	light[ 0 ].y = +light_h + light_y;
	light[ 0 ].z = +light_d * light_scale + light_z;

	light[ 1 ].x = -light_w * light_scale + light_x;
	light[ 1 ].y = +light_h + light_y;
	light[ 1 ].z = -light_d * light_scale + light_z;

	light[ 2 ].x = +light_w * light_scale + light_x;
	light[ 2 ].y = +light_h + light_y;
	light[ 2 ].z = -light_d * light_scale + light_z;

	light[ 3 ].x = +light_w * light_scale + light_x;
	light[ 3 ].y = +light_h + light_y;
	light[ 3 ].z = +light_d * light_scale + light_z;
	*/


	light_init_position = light;
//	std::cout << light[ 0 ].x << ", " << light[ 0 ].y << ", " << light[ 0 ].z << std::endl;
	std::array< vcl::vec3, 4 > vertex = {
		vcl::vec3( - 1.f, 0.f, + 1.f ), vcl::vec3( - 1.f, 0.f, - 1.f ),
		vcl::vec3( + 1.f, 0.f, - 1.f ), vcl::vec3( + 1.f, 0.f, + 1.f )
	};
	std::array< vcl::vec3, 4 > normal = { vcl::vec3( 0.f, - 1.f, 0.f ),
	                                      vcl::vec3( 0.f, - 1.f, 0.f ),
	                                      vcl::vec3( 0.f, - 1.f, 0.f ),
	                                      vcl::vec3( 0.f, - 1.f, 0.f ) };
	std::array< vcl::vec3, 4 > color = { vcl::col3( 1.f, 1.f, 0.f ), vcl::col3( 1.f, 1.f, 0.f ), vcl::col3( 1.f, 1.f, 0.f ), vcl::col3( 1.f, 1.f, 0.f ) };

	std::array< vcl::vec2, 4 > texcoord = { vcl::vec2( 0.f, 0.f ), vcl::vec2( 0.f, 1.f ), vcl::vec2( 1.f, 1.f ), vcl::vec2( 1.f, 0.f ) };

	glBindVertexArray( 0 );
	glGenBuffers( 4, light_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, light_vbo[ 0 ] );
	glBufferData( GL_ARRAY_BUFFER, 4 * sizeof( vcl::vec3 ), vertex.data(), GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, light_vbo[ 1 ] );
	glBufferData( GL_ARRAY_BUFFER, 4 * sizeof( vcl::vec3 ), normal.data(), GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, light_vbo[ 2 ] );
	glBufferData( GL_ARRAY_BUFFER, 4 * sizeof( vcl::vec3 ), color.data(), GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, light_vbo[ 3 ] );
	glBufferData( GL_ARRAY_BUFFER, 4 * sizeof( vcl::vec2 ), texcoord.data(), GL_STATIC_DRAW );

	glGenVertexArrays( 1, &light_vao );
	glBindVertexArray( light_vao );

	glEnableVertexAttribArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, light_vbo[ 0 ] );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( 0 ) );

	glEnableVertexAttribArray( 1 );
	glBindBuffer( GL_ARRAY_BUFFER, light_vbo[ 1 ] );
	glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( 0 ) );

	glEnableVertexAttribArray( 2 );
	glBindBuffer( GL_ARRAY_BUFFER, light_vbo[ 2 ] );
	glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( 0 ) );

	glEnableVertexAttribArray( 3 );
	glBindBuffer( GL_ARRAY_BUFFER, light_vbo[ 3 ] );
	glVertexAttribPointer( 3, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( 0 ) );

	glBindVertexArray( 0 );

	std::array< GLuint, 6 > index = { 0, 1, 2, 0, 2, 3 };
	glGenBuffers( 1, &light_vbo_index );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, light_vbo_index );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, 2 * sizeof( GLuint ) * 3, index.data(), GL_STATIC_DRAW );

	//dummy texture
	GLubyte dummy[ 3 ] = { 255, 255, 255 };
	glGenTextures( 1, &light_tex_id );
	glBindTexture( GL_TEXTURE_2D, light_tex_id );
#ifdef USE_TEXTURE_LIGHT
	std::unique_ptr< TGAImage > img = std::make_unique< TGAImage >();
	img->LoadImage( ( texture_light_filename + ".tga" ).c_str() );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, img->w(), img->h(), 0, GL_RGB, GL_UNSIGNED_BYTE, img->data().get() );
//	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, img->w(), img->h(), 0, GL_RGB, GL_UNSIGNED_BYTE, img->data().get() );
#else
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, dummy );
#endif
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );//GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ); //GL_LINEAR );
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
}

//多角形光源を描画
void display_light()
{
	glBindVertexArray( light_vao );

	//vertex
	glEnableVertexAttribArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, light_vbo[ 0 ] );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( 0 ) );

	//normal
	glEnableVertexAttribArray( 1 );
	glBindBuffer( GL_ARRAY_BUFFER, light_vbo[ 1 ] );
	glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( 0 ) );

	//color
	glEnableVertexAttribArray( 2 );
	glBindBuffer( GL_ARRAY_BUFFER, light_vbo[ 2 ] );
	glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( 0 ) );

	//texcoord
	glEnableVertexAttribArray( 3 );
	glBindBuffer( GL_ARRAY_BUFFER, light_vbo[ 3 ] );
	glVertexAttribPointer( 3, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( 0 ) );

	if( texture_light_flag ) {
		glEnable( GL_TEXTURE_2D );
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, light_tex_id );
		glUniform1i( light_tex_param, 0 );
	}

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, light_vbo_index );
	glDrawElements( GL_TRIANGLES, GLsizei( 3 * 2 ), GL_UNSIGNED_INT, BUFFER_OFFSET( 0 ) );
	if( texture_light_flag ) {
		glDisable( GL_TEXTURE_2D );
	}
	glDisableVertexAttribArray( 3 );
	glDisableVertexAttribArray( 2 );
	glDisableVertexAttribArray( 1 );
	glDisableVertexAttribArray( 0 );

	glBindVertexArray( 0 );
}

void capture()
{
	int buffer_width, buffer_height;
	glfwGetFramebufferSize( main_window, &buffer_width, &buffer_height );
	std::unique_ptr< unsigned char [] > buffer = std::make_unique< unsigned char [] >( 3 * buffer_width * buffer_height );
	std::unique_ptr< unsigned char [] > pixels = std::make_unique< unsigned char [] >( 3 * buffer_width * buffer_height );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glReadPixels( 0, 0, buffer_width, buffer_height, GL_RGB, GL_UNSIGNED_BYTE, buffer.get() );
	for( int h = 0; h < buffer_height; ++h ) {
		for( int w = 0; w < buffer_width; ++w ) {
			pixels[ 3 * ( h * buffer_width + w ) + 0 ] = buffer[ 3 * ( ( buffer_height - 1 - h ) * buffer_width + w ) + 0 ];
			pixels[ 3 * ( h * buffer_width + w ) + 1 ] = buffer[ 3 * ( ( buffer_height - 1 - h ) * buffer_width + w ) + 1 ];
			pixels[ 3 * ( h * buffer_width + w ) + 2 ] = buffer[ 3 * ( ( buffer_height - 1 - h ) * buffer_width + w ) + 2 ];
		}
	}
	const std::string filename = "output.png";
	stbi_write_png( filename.c_str(), buffer_width, buffer_height, 3, pixels.get(), 0 );
}

void update_light()
{
	const glm::vec3 x = { 1.f, 0.f, 0.f };
	const glm::vec3 y = { 0.f, 1.f, 0.f };
	const glm::vec3 z = { 0.f, 0.f, 1.f };
	const glm::vec4 p0 = { - 1.f, 0.f, + 1.f, 1.f };
	const glm::vec4 p1 = { - 1.f, 0.f, - 1.f, 1.f };
	const glm::vec4 p2 = { + 1.f, 0.f, - 1.f, 1.f };
	const glm::vec4 p3 = { + 1.f, 0.f, + 1.f, 1.f };
	light_rotate_matrix     = glm::rotate( glm::radians( light_angle.z ), z ) * glm::rotate( glm::radians( light_angle.y ), y ) * glm::rotate( glm::radians( light_angle.x ), x );
	light_scale_matrix      = glm::scale( light_scale );
	light_translate_matrix  = glm::translate( light_translate );
	const auto matrix = light_translate_matrix * light_rotate_matrix * light_scale_matrix;
	{
		const auto lv0 = matrix * p0;
		const auto lv1 = matrix * p1;
		const auto lv2 = matrix * p2;
		const auto lv3 = matrix * p3;
		light[ 0 ] = { lv0.x, lv0.y, lv0.z };
		light[ 1 ] = { lv1.x, lv1.y, lv1.z };
		light[ 2 ] = { lv2.x, lv2.y, lv2.z };
		light[ 3 ] = { lv3.x, lv3.y, lv3.z };
	}
}


//
void keyboard_function( GLFWwindow *window, int key, int scancode, int action, int mods )
{
	if( key == GLFW_KEY_Q && action == GLFW_PRESS ) {
		glfwSetWindowShouldClose( window, GL_TRUE );
	} else if( key == GLFW_KEY_0 && action == GLFW_PRESS ) {
		cmode = camera_mode::noop;
	} else if( key == GLFW_KEY_1 && action == GLFW_PRESS ) {
		cmode = camera_mode::rotate;
	} else if( key == GLFW_KEY_2 && action == GLFW_PRESS ) {
		cmode = camera_mode::walk;
	} else if( key == GLFW_KEY_3 && action == GLFW_PRESS ) {
		cmode = camera_mode::light_rotate_x;
	} else if( key == GLFW_KEY_4 && action == GLFW_PRESS ) {
		cmode = camera_mode::light_rotate_y;
	} else if( key == GLFW_KEY_5 && action == GLFW_PRESS ) {
		cmode = camera_mode::light_rotate_z;
	} else if( key == GLFW_KEY_6 && action == GLFW_PRESS ) {
		cmode = camera_mode::light_translate;
	} else if( key == GLFW_KEY_7 && action == GLFW_PRESS ) {
		cmode = camera_mode::light_scale;
	} else if( key == GLFW_KEY_8 && action == GLFW_PRESS ) {

	} else if( key == GLFW_KEY_Z && action == GLFW_PRESS ) {

	} else if( key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS ) {
		std::cout << "camera(" << camera[ 0 ] << ", " << camera[ 1 ] << ", " << camera[ 2 ] << ")" << std::endl;
		std::cout << "camera_theta   : " << camera_theta << std::endl;
		std::cout << "camera_phi     : " << camera_phi << std::endl;
		std::cout << "camera_z       : " << camera_z << std::endl;
		std::cout << "camera_target_z: " << camera_target_z << std::endl;
		for( size_t i = 0; i < light.size(); ++i ){
			std::cout << "light[" << i << "](" << light[i].x << ", " << light[i].y << ", " << light[i].z << ")" << std::endl;
		}
		std::cout << std::endl;
	} else if( key == GLFW_KEY_C && action == GLFW_PRESS ) {
		capture();
	} else if( key == GLFW_KEY_RIGHT && (action == GLFW_REPEAT || action == GLFW_PRESS ) ) {
		//light_translate.x += 0.1f * light_scale;
		//for( size_t i = 0; i < light.size(); ++i ) { light[ i ].x = light_init_position[ i ].x + light_translate.x; }
	} else if( key == GLFW_KEY_LEFT && (action == GLFW_REPEAT || action == GLFW_PRESS ) ) {
		//light_translate.x -= 0.1f * light_scale;
		//for( size_t i = 0; i < light.size(); ++i ) { light[ i ].x = light_init_position[ i ].x + light_translate.x; }
	} else if( key == GLFW_KEY_UP && (action == GLFW_REPEAT || action == GLFW_PRESS ) ) {
		//light_translate.z += 0.1f * light_scale;
		//for( size_t i = 0; i < light.size(); ++i ) { light[ i ].z = light_init_position[ i ].z + light_translate.z; }
	} else if( key == GLFW_KEY_DOWN && (action == GLFW_REPEAT || action == GLFW_PRESS ) ) {
		//light_translate.z -= 0.1f * light_scale;
		//for( size_t i = 0; i < light.size(); ++i ) { light[ i ].z = light_init_position[ i ].z + light_translate.z; }
	} else if( key == GLFW_KEY_Y && (action == GLFW_REPEAT || action == GLFW_PRESS ) ) {
		//light_translate.y += 0.1f * light_scale;
		//for( size_t i = 0; i < light.size(); ++i ) { light[ i ].y = light_init_position[ i ].y + light_translate.y; }
	} else if( key == GLFW_KEY_H && (action == GLFW_REPEAT || action == GLFW_PRESS ) ) {
		//light_translate.y -= 0.1f * light_scale;
		//for( size_t i = 0; i < light.size(); ++i ) { light[ i ].y = light_init_position[ i ].y + light_translate.y; }
	}
}


void cursor_pos_function( GLFWwindow *window, double x, double y )
{
	if( mstate == mouse_state::down ) {
		if( cmode == camera_mode::rotate ){
			constexpr double cor = 1e-3 * 2.0;
			const double dx = x - mouse_x;
			const double dy = y - mouse_y;
			camera_theta += +dy * cor;
			camera_phi   += -dx * cor;
			const float ct = cos( camera_theta );
			const float st = sin( camera_theta );
			const float cp = cos( camera_phi );
			const float sp = sin( camera_phi );
			camera[ 0 ] = camera_x + camera_z * ct * sp;
			camera[ 1 ] = camera_z * st;
			camera[ 2 ] = camera_z * ct * cp;
			camera_target[ 0 ] = camera_x + camera_target_z * ct * sp;
			camera_target[ 1 ] = camera_target_z * st;
			camera_target[ 2 ] = camera_target_z * ct * cp;
			camera_up.x = -st * sp;
			camera_up.y = ct;
			camera_up.z = -st * cp;
			is_update_camera = true;
		} else if( cmode == camera_mode::walk ) {
			constexpr double cor = 1e-2;
			const double dy = y - mouse_y;
			camera_z += -dy * cor;
			camera_target_z += -dy * cor;
			const float ct = cos(camera_theta);
			const float st = sin(camera_theta);
			const float cp = cos(camera_phi);
			const float sp = sin(camera_phi);
			camera[0] = camera_x + camera_z * ct * sp;
			camera[1] = camera_z * st;
			camera[2] = camera_z * ct * cp;
			camera_target[0] = camera_x + camera_target_z * ct * sp;
			camera_target[1] = camera_target_z * st;
			camera_target[2] = camera_target_z * ct * cp;
			is_update_camera = true;
		} else if( cmode == camera_mode::light_rotate_x ) {
			constexpr float cor = 5e-2f;
			const auto dy = ( float ) ( y - mouse_y );
			light_angle.x += cor * dy;
			is_update_light = true;
		} else if( cmode == camera_mode::light_rotate_y ) {
			constexpr float cor = 5e-2f;
			const auto dy = ( float ) ( y - mouse_y );
			light_angle.y += cor * dy;
			is_update_light = true;
		} else if( cmode == camera_mode::light_rotate_z ) {
			constexpr float cor = 5e-2f;
			const auto dy = ( float ) ( y - mouse_y );
			light_angle.z += cor * dy;
			is_update_light = true;
		} else if( cmode == camera_mode::light_translate ) {
			constexpr float cor = 5e-2f;
			const auto dx = ( float ) ( x - mouse_x );
			const auto dz = ( float ) ( y - mouse_y );
			light_translate.x += cor * dx;
			light_translate.z += cor * dz;
			is_update_light = true;
		} else if( cmode == camera_mode::light_scale ) {
			constexpr float cor = 5e-2f;
			const auto dx = ( float ) ( x - mouse_x );
			const auto dz = ( float ) ( y - mouse_y );
			light_scale.x += cor * dx;
			light_scale.z += cor * dz;
			is_update_light = true;
		}
	}
	mouse_x = x;
	mouse_y = y;
}

void mouse_button_function( GLFWwindow *window, int button, int action, int mods )
{
	mstate = ( button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS ) ? mouse_state::down : mouse_state::up;
}

//
void init_cubemap()
{
	std::vector< float > vertex_buffer = {
		//nx
		-1.f, +1.f, +1.f,
		-1.f, -1.f, +1.f,
		-1.f, -1.f, -1.f,
		-1.f, +1.f, -1.f,
		//px
		+1.f, +1.f, -1.f,
		+1.f, -1.f, -1.f,
		+1.f, -1.f, +1.f,
		+1.f, +1.f, +1.f,
		//ny
		-1.f, -1.f, -1.f,
		-1.f, -1.f, +1.f,
		+1.f, -1.f, +1.f,
		+1.f, -1.f, -1.f,
		//py
		-1.f, +1.f, +1.f,
		-1.f, +1.f, -1.f,
		+1.f, +1.f, -1.f,
		+1.f, +1.f, +1.f,
		//nz
		-1.f, +1.f, -1.f,
		-1.f, -1.f, -1.f,
		+1.f, -1.f, -1.f,
		+1.f, +1.f, -1.f,
		//pz
		+1.f, +1.f, +1.f,
		+1.f, -1.f, +1.f,
		-1.f, -1.f, +1.f,
		-1.f, +1.f, +1.f,
	};

	std::vector< float > normal_buffer = {
		//nx
		+1.f, 0.f, 0.f,
		+1.f, 0.f, 0.f,
		+1.f, 0.f, 0.f,
		+1.f, 0.f, 0.f,
		//px
		-1.f, 0.f, 0.f,
		-1.f, 0.f, 0.f,
		-1.f, 0.f, 0.f,
		-1.f, 0.f, 0.f,
		//ny
		0.f, +1.f, 0.f,
		0.f, +1.f, 0.f,
		0.f, +1.f, 0.f,
		0.f, +1.f, 0.f,
		//py
		0.f, -1.f, 0.f,
		0.f, -1.f, 0.f,
		0.f, -1.f, 0.f,
		0.f, -1.f, 0.f,
		//nz
		0.f, 0.f, +1.f,
		0.f, 0.f, +1.f,
		0.f, 0.f, +1.f,
		0.f, 0.f, +1.f,
		//pz
		0.f, 0.f, -1.f,
		0.f, 0.f, -1.f,
		0.f, 0.f, -1.f,
		0.f, 0.f, -1.f,
	};

	std::vector< float > texcoord_buffer = {
		0.f, 0.f,
		0.f, 1.f,
		1.f, 1.f,
		1.f, 0.f,

		0.f, 0.f,
		0.f, 1.f,
		1.f, 1.f,
		1.f, 0.f,

		0.f, 0.f,
		0.f, 1.f,
		1.f, 1.f,
		1.f, 0.f,

		0.f, 0.f,
		0.f, 1.f,
		1.f, 1.f,
		1.f, 0.f,

		0.f, 0.f,
		0.f, 1.f,
		1.f, 1.f,
		1.f, 0.f,

		0.f, 0.f,
		0.f, 1.f,
		1.f, 1.f,
		1.f, 0.f,
	};

	std::vector< GLuint > index_buffer = {
		0,  1,  2,  0,  2,  3,
		4,  5,  6,  4,  6,  7,
		8,  9, 10,  8, 10, 11,
		12, 13, 14, 12, 14, 15,
		16, 17, 18, 16, 18, 19,
		20, 21, 22, 20, 22, 23,
	};

	glGenBuffers( 3, cubemap_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, cubemap_vbo[ 0 ] );
	glBufferData( GL_ARRAY_BUFFER, vertex_buffer.size() * sizeof( float ), vertex_buffer.data(), GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, cubemap_vbo[ 1 ] );
	glBufferData( GL_ARRAY_BUFFER, normal_buffer.size() * sizeof( float ), normal_buffer.data(), GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, cubemap_vbo[ 2 ] );
	glBufferData( GL_ARRAY_BUFFER, texcoord_buffer.size() * sizeof( float ), texcoord_buffer.data(), GL_STATIC_DRAW );

	glGenBuffers( 1, &cubemap_vbo_index );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, cubemap_vbo_index );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, index_buffer.size() * sizeof( GLuint ), index_buffer.data(), GL_STATIC_DRAW );

	glGenVertexArrays( 1, &cubemap_vao );
	glBindVertexArray( cubemap_vao );

	glEnableVertexAttribArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, cubemap_vbo[ 0 ] );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( 0 ) );

	glEnableVertexAttribArray( 1 );
	glBindBuffer( GL_ARRAY_BUFFER, cubemap_vbo[ 1 ] );
	glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( 0 ) );

	glEnableVertexAttribArray( 2 );
	glBindBuffer( GL_ARRAY_BUFFER, cubemap_vbo[ 2 ] );
	glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( 0 ) );

	glBindVertexArray( 0 );

	const std::array< std::string, 6 > cubemapfilename = {
#if defined( _WIN32 )
		"../environment_map/LancellottiChapel/negx1024.tga",
		"../environment_map/LancellottiChapel/posx1024.tga",
		"../environment_map/LancellottiChapel/negy1024.tga",
		"../environment_map/LancellottiChapel/posy1024.tga",
		"../environment_map/LancellottiChapel/negz1024.tga",
		"../environment_map/LancellottiChapel/posz1024.tga",
#else
		"/Users/iwasaki/resource/environment_map/negx1024.tga",
		"/Users/iwasaki/resource/environment_map/posx1024.tga",
		"/Users/iwasaki/resource/environment_map/negy1024.tga",
		"/Users/iwasaki/resource/environment_map/posy1024.tga",
		"/Users/iwasaki/resource/environment_map/negz1024.tga",
		"/Users/iwasaki/resource/environment_map/posz1024.tga",
#endif
	};

	glGenTextures( 6, cubemap_texid );
	for( size_t i = 0; i < 6; ++i ) {
		std::unique_ptr< TGAImage > img = std::make_unique< TGAImage >();
		img->LoadImage( cubemapfilename[ i ].c_str() );

		glBindTexture( GL_TEXTURE_2D, cubemap_texid[ i ] );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, img->w(), img->h(), 0, GL_RGB, GL_UNSIGNED_BYTE, img->data().get() );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
	}

	cubemap_shader      = std::make_unique< vcl::glslprogram >( "../glsl/cubemap_shader.vert", "../glsl/cubemap_shader.frag" );
	cubemap_shader_mvp  = glGetUniformLocation( cubemap_shader->program_handle(), "mvp"  ); if( cubemap_shader_mvp  < 0 ) std::cerr << "cannot get uniform location for mvp " << std::endl;
	cubemap_shader_tex0 = glGetUniformLocation( cubemap_shader->program_handle(), "tex0" ); if( cubemap_shader_tex0 < 0 ) std::cerr << "cannot get uniform location for tex0" << std::endl;
}

//
void display_cubemap( const glm::mat4x4& mvp )
{
	const auto m = glm::scale( glm::vec3( cubemap_scale, cubemap_scale, cubemap_scale ) );

	cubemap_shader->bind();
	cubemap_shader->use();

	glUniformMatrix4fv( cubemap_shader_mvp, 1, GL_FALSE, glm::value_ptr( mvp * m ) );

	glBindVertexArray( cubemap_vao );

	//vertex
	glEnableVertexAttribArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, cubemap_vbo[ 0 ] );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( 0 ) );

	//normal
	glEnableVertexAttribArray( 1 );
	glBindBuffer( GL_ARRAY_BUFFER, cubemap_vbo[ 1 ] );
	glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( 0 ) );

	//texcoord
	glEnableVertexAttribArray( 2 );
	glBindBuffer( GL_ARRAY_BUFFER, cubemap_vbo[ 2 ] );
	glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( 0 ) );

	glEnable( GL_TEXTURE_2D );
	for( size_t i = 0; i < 6; ++i ) {
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, cubemap_texid[ i ] );
		glUniform1i( cubemap_shader_tex0, 0 );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, cubemap_vbo_index );
		glDrawElements( GL_TRIANGLES, GLsizei( 3 * 2 ), GL_UNSIGNED_INT, BUFFER_OFFSET( 3 * 2 * sizeof( GLsizei ) * i ) );
	}
	glDisable( GL_TEXTURE_2D );

	glDisableVertexAttribArray( 2 );
	glDisableVertexAttribArray( 1 );
	glDisableVertexAttribArray( 0 );

	glBindVertexArray( 0 );

	cubemap_shader->unbind();
}


