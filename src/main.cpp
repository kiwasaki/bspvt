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

		ImGui::SliderFloat( "alpha", &alpha, 0.f, 1.f );
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
		rndr->load_edge_tree( "../cmake-build-release/dragon_ex_min_hemi.qtr" );
			//rndr->load_edge_tree( "../cmake-build-release/dragon_ex_min_1e-4f.qtr" );
		rndr->calc_visible_domain_for_envmap();
		rndr->set_tex_light_flag( texture_light_flag );
		rndr->set_env_light_flag( envmap_light_flag );
		rndr->set_envmap_scale( envmap_scale );
	#endif

	#if defined( LIVING_ROOM )
		//const auto rndr = std::make_unique< vcl::rndr::experimental::renderer >( "../obj/", "living_room_006.obj", frame_buffer_size_x, frame_buffer_size_y );
		//rndr->load_edge_tree( "../cmake-build-release/living_room_006_ex_min_.qtr" );
		const auto rndr = std::make_unique< vcl::rndr::experimental::renderer >( "../obj/", "living_room20210107.obj", frame_buffer_size_x, frame_buffer_size_y, texture_light_filename, envmap_folder );
		rndr->load_edge_tree( "../cmake-build-release/living_room_ex_min_hemi.qtr" );
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
		/*
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", &show_another_window);

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}
		*/
		//		const auto end = std::chrono::system_clock::now();
		//		std::cout << "duration : " << std::chrono::duration_cast< std::chrono::milliseconds >( end - start ).count() << " ms" << std::endl;

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
		#if defined( USE_ENVMAP )
		display_cubemap( mvp );
		#endif

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

/*
int main(int, char**)
{
	// Setup window
	glfwSetErrorCallback( glfw_error_callback );
	if ( !glfwInit() )
		return 1;

	// Decide GL+GLSL versions
#ifdef __APPLE__
	// GL 3.2 + GLSL 150
	const char* glsl_version = "#version 150";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1 );
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
	// GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

	// Create window with graphics context
	GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
	if (window == NULL)
		return 1;
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0 );
	//glfwSwapInterval(1); // Enable vsync

	// Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
	bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
	bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
	bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
	bool err = gladLoadGL(glfwGetProcAddress) == 0; // glad2 recommend using the windowing library loader instead of the (optionally) bundled one.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
	bool err = false;
    glbinding::Binding::initialize();
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
	bool err = false;
    glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)glfwGetProcAddress(name); });
#else
	bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
	if (err)
	{
		fprintf(stderr, "Failed to initialize OpenGL loader!\n");
		return 1;
	}

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

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

	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		glfwPollEvents();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", &show_another_window);

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		// 3. Show another simple window.
		if (show_another_window)
		{
			ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
				show_another_window = false;
			ImGui::End();
		}

		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
*/

