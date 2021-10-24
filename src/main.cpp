// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <stdio.h>

// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>            // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>            // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>          // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
#include <glad/gl.h>            // Initialize with gladLoadGL(...) or gladLoaderLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/Binding.h>  // Initialize with glbinding::Binding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/glbinding.h>// Initialize with glbinding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

#include "main.hpp"

//
void display_gui( const std::unique_ptr< vcl::rndr::experimental::renderer >& rndr )
{
	static size_t material_id = 0;
	static std::array< float, 4 > color = { };
	static float alpha = rndr->alpha( 0 );
	static std::string current_material = std::string( "dummy" ); //material_list[ material_id ];


	ImGui::Begin( "material editor" );
	ImGui::Text( "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate );

	if( ImGui::TreeNode( "camera" ) ) {
		//ImGui::Begin( "camera");
		if( ImGui::SliderFloat( "theta", &camera_theta, 0.f, M_PI ) ) is_update_camera = true;
		if( ImGui::SliderFloat( "phi", &camera_phi, - M_PI, M_PI ) ) is_update_camera = true;
		if( ImGui::SliderFloat( "zoom", &camera_z, 0.f, 100.f ) ) is_update_camera = true;
		ImGui::TreePop();
		ImGui::Separator();
		//ImGui::End();
		update_camera();
	}

	if( ImGui::TreeNode( "material" ) ) {
		if ( ImGui::BeginCombo( "material", current_material.c_str() ) ) {
			for( size_t i = 0, n = material_list.size(); i < n; ++i ) {
				bool is_selected = ( current_material == material_list[ i ] ); // You can store your selection however you want, outside or inside your objects
				if ( ImGui::Selectable( material_list[ i ].c_str(), is_selected ) ) {
					material_id = i;
					current_material = material_list[ i ];
					const auto kd = rndr->kd( material_id );
					color[ 0 ] = kd.x;
					color[ 1 ] = kd.y;
					color[ 2 ] = kd.z;
					alpha = rndr->alpha( material_id );
				}
				if ( is_selected ) ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::ColorEdit3( "Color", color.data() );
		{
			rndr->set_kd( material_id, vcl::col3( color[ 0 ], color[ 1 ], color[ 2 ] ) );
		}

		ImGui::SliderFloat( "alpha", &alpha, 0.15f, 1.f );
		{
			rndr->set_ka( material_id, alpha );
		}
		ImGui::TreePop();
		ImGui::Separator();
	}

	is_update_light = false;
	if( ImGui::TreeNode( "light" ) ) {
		//ImGui::BeginChild( "light" );
		ImGui::SliderFloat( "Le", &le, 0.f, 100.f );
		if( ImGui::SliderFloat( "translate_x", &light_translate.x, light_translate_min.x, light_translate_max.x ) ) is_update_light = true;
		if( ImGui::SliderFloat( "translate_y", &light_translate.y, light_translate_min.y, light_translate_max.y ) ) is_update_light = true;
		if( ImGui::SliderFloat( "translate_z", &light_translate.z, light_translate_min.z, light_translate_max.z ) ) is_update_light = true;
		if( ImGui::SliderFloat3( "rotate", glm::value_ptr( light_angle ), 0.f, 360.f ) ) is_update_light = true;
		if( ImGui::SliderFloat( "scale_x", &light_scale.x, 1e-5f, light_scale_max.x ) ) is_update_light = true;
		if( ImGui::SliderFloat( "scale_z", &light_scale.z, 1e-5f, light_scale_max.z ) ) is_update_light = true;
		if( ImGui::Checkbox( "display light" , &is_display_light ) ) {}
		if( ImGui::Checkbox( "texture light", &texture_light_flag ) ) rndr->set_tex_light_flag( texture_light_flag );
		if( ImGui::Checkbox( "envmap light", &envmap_light_flag ) ) rndr->set_env_light_flag( envmap_light_flag );
		if( ImGui::SliderFloat( "envmap scale", &envmap_scale, 1e-5f, 1.f ) ) rndr->set_envmap_scale( envmap_scale );
		//ImGui::EndChild();
		ImGui::TreePop();
		ImGui::Separator();
	}

	ImGui::End();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main( int argc, char** argv )
{
	init_glfw();
	init_lookat_param();
	init_light_pos();
	init_cubemap();

	ltc::LTC< float > ltc( tabM, tabMinv, tabAmplitude, alpha );

	int frame_buffer_size_x, frame_buffer_size_y;
	glfwGetFramebufferSize( main_window, &frame_buffer_size_x, &frame_buffer_size_y );
	glfwSetKeyCallback( main_window, keyboard_function );
	glfwSetMouseButtonCallback( main_window, mouse_button_function );
	glfwSetCursorPosCallback( main_window, cursor_pos_function );

	#if defined( TEAPOT )
		const auto rndr = std::make_unique< vcl::rndr::experimental::renderer >( "../obj/", "teapot.obj", frame_buffer_size_x, frame_buffer_size_y, texture_light_filename, envmap_folder );
		rndr->load_edge_tree( "../data/teapot.bspvt" );
		rndr->calc_visible_domain_for_envmap();
		rndr->set_tex_light_flag( texture_light_flag );
		rndr->set_env_light_flag( envmap_light_flag );
		rndr->set_envmap_scale( envmap_scale );
	#endif

	#if defined( DRAGON )
		const auto rndr = std::make_unique< vcl::rndr::experimental::renderer >( "../obj/", "dragon.obj", frame_buffer_size_x, frame_buffer_size_y, texture_light_filename, envmap_folder );
		rndr->load_edge_tree( "../data/dragon.bspvt" );
			//rndr->load_edge_tree( "../cmake-build-release/dragon_ex_min_1e-4f.qtr" );
		rndr->calc_visible_domain_for_envmap();
		rndr->set_tex_light_flag( texture_light_flag );
		rndr->set_env_light_flag( envmap_light_flag );
		rndr->set_envmap_scale( envmap_scale );
	#endif

	#if defined( LIVING_ROOM )
		const auto rndr = std::make_unique< vcl::rndr::experimental::renderer >( "../obj/", "living_room.obj", frame_buffer_size_x, frame_buffer_size_y, texture_light_filename, envmap_folder );
		rndr->load_edge_tree( "../data/living_room.bspvt" );
		rndr->calc_visible_domain_for_envmap();
		rndr->set_tex_light_flag( texture_light_flag );
		rndr->set_env_light_flag( envmap_light_flag );
		rndr->set_envmap_scale( envmap_scale );
	#endif


	material_list.resize( rndr->material_size() );
	const auto& mat_name = rndr->material_name();
	for( size_t i = 0, n = material_list.size(); i < n; ++i ) {
		material_list[ i ] = mat_name[ i ];
	}


	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL( main_window, true);
	ImGui_ImplOpenGL3_Init( glsl_version );

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Read 'docs/FONTS.md' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != NULL);

	// Our state
	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	glViewport( 0, 0, frame_buffer_size_x, frame_buffer_size_y );

	while( !glfwWindowShouldClose( main_window ) ) {

		//glfwPollEvents();
		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

#if defined( MEASURE_TIME )
		const auto start = std::chrono::system_clock::now();
#endif
		display_gui( rndr );

		glm::mat4x4 mvp = projection * glm::lookAt( glm::vec3(camera[ 0 ], camera[ 1 ], camera[ 2 ] ), camera_target, camera_up );
		if( is_update_camera ) {
			rndr->calc_visible_vertices( mvp );
			is_update_camera = false;
		}
		if( is_update_light ) {
			update_light();
			is_update_light = false;
		}

		//const auto start = std::chrono::system_clock::now();
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glClearColor( 0.f, 0.f, 0.f, 0.f );
		//const auto start = std::chrono::system_clock::now();
		rndr->calc_radiance( camera, light, vcl::col3( le, le, le ), ltc );
		rndr->display( mvp );

		shader->bind();
		shader->use();

		glUniformMatrix4fv( mvp_param, 1, GL_FALSE, glm::value_ptr( mvp * light_translate_matrix * light_rotate_matrix * light_scale_matrix ) );
		if( is_display_light ) display_light();

		shader->unbind();
		//#if defined( USE_ENVMAP )
		display_cubemap( mvp );
		//#endif

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

		glfwSwapBuffers( main_window );
		glfwPollEvents();

#if defined( MEASURE_TIME )
		const auto end = std::chrono::system_clock::now();
		std::cout << "duration : " << std::chrono::duration_cast< std::chrono::milliseconds >( end - start ).count() << " ms" << std::endl;
#endif
#if defined( DISPLAY_TRAVERSAL_COUNT )

		size_t ni = 0, nl = 0;
		for( const auto& i : rndr->visible_vertex_list() ) {
			ni += rndr->inner_node_traversal()[ i ];
			nl += rndr->leaf_node_traversal()[ i ];
		}
		std::cout << "num of visible vertices   : " << rndr->visible_vertex_size() << std::endl;
		std::cout << "inner nodes               : " << ni << std::endl;
		std::cout << "leaf  nodes               : " << nl << std::endl;
		std::cout << "avg traversal             : " << ( ni + nl ) / float( rndr->visible_vertex_size() ) << std::endl;

#endif
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow( main_window );
	glfwTerminate();
	return 0;
}

