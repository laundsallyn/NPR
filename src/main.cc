#include <GL/glew.h>
#include <dirent.h>

#include "bone_geometry.h"
#include "procedure_geometry.h"
#include "render_pass.h"
#include "config.h"
#include "gui.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "glm/ext.hpp" 
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/io.hpp>
#include <debuggl.h>

int window_width = 800, window_height = 600;
const std::string window_title = "Non-Photorealistic Rendering";

const char* vertex_shader =
#include "shaders/default.vert"
;

const char* geometry_shader =
#include "shaders/default.geom"
;

const char* fragment_shader =
#include "shaders/default.frag"
;

const char* floor_fragment_shader =
#include "shaders/floor.frag"
;

const char* bones_fragment_shader = 
#include "shaders/bones.frag"
;

const char* line_mesh_geometry_shader = 
#include "shaders/line_mesh.geom"
;

const char* cylinder_fragment_shader = 
#include "shaders/cylinder.frag"
;

const char* coordinate_fragment_shader = 
#include "shaders/coordinate.frag"
;

const char* screen_vertex_shader =
#include "shaders/screen.vert"
;

const char* screen_fragment_shader =
#include "shaders/screen.frag"
;

void ErrorCallback(int error, const char* description) {
	std::cerr << "GLFW Error: " << description << "\n";
}

GLFWwindow* init_glefw()
{
	if (!glfwInit())
		exit(EXIT_FAILURE);
	glfwSetErrorCallback(ErrorCallback);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	auto ret = glfwCreateWindow(window_width, window_height, window_title.data(), nullptr, nullptr);
	CHECK_SUCCESS(ret != nullptr);
	glfwMakeContextCurrent(ret);
	glewExperimental = GL_TRUE;
	CHECK_SUCCESS(glewInit() == GLEW_OK);
	glGetError();  // clear GLEW's error for it
	glfwSwapInterval(1);
	const GLubyte* renderer = glGetString(GL_RENDERER);  // get renderer string
	const GLubyte* version = glGetString(GL_VERSION);    // version as a string
	std::cout << "Renderer: " << renderer << "\n";
	std::cout << "OpenGL version supported:" << version << "\n";

	return ret;
}

struct ScreenQuad {
	ScreenQuad() {
		// vertices.push_back(glm::vec4(-1.0f,  1.0f, 0.0f, 1.0f));
		vertices.push_back(glm::vec2(-1.0f,  1.0f));
		tex_coords.push_back(glm::vec2(0.0f, 1.0f));

		// vertices.push_back(glm::vec4(-1.0f, -1.0f, 0.0f, 0.0f));
		vertices.push_back(glm::vec2(-1.0f, -1.0f));
		tex_coords.push_back(glm::vec2(0.0f, 0.0f));

		// vertices.push_back(glm::vec4( 1.0f, -1.0f, 1.0f, 0.0f));
		vertices.push_back(glm::vec2( 1.0f, -1.0f));
		tex_coords.push_back(glm::vec2(1.0f, 0.0f));

		// vertices.push_back(glm::vec4(-1.0f,  1.0f, 0.0f, 1.0f));
		vertices.push_back(glm::vec2(-1.0f,  1.0f));
		tex_coords.push_back(glm::vec2(0.0f, 1.0f));

		// vertices.push_back(glm::vec4( 1.0f, -1.0f, 1.0f, 0.0f));
		vertices.push_back(glm::vec2( 1.0f, -1.0f));
		tex_coords.push_back(glm::vec2(1.0f, 0.0f));

		// vertices.push_back(glm::vec4( 1.0f,  1.0f, 1.0f, 1.0f));
		vertices.push_back(glm::vec2( 1.0f,  1.0f));
		tex_coords.push_back(glm::vec2(1.0f, 1.0f));

	};
	~ScreenQuad() {};

	std::vector<glm::vec2> vertices;
	std::vector<glm::vec2> tex_coords;
};

int main(int argc, char* argv[])
{
	int last_bone = -1;
	if (argc < 2) {
		std::cerr << "Input model file is missing" << std::endl;
		std::cerr << "Usage: " << argv[0] << " <PMD file>" << std::endl;
		return -1;
	}
	GLFWwindow *window = init_glefw();
	GUI gui(window);

	std::vector<glm::vec4> floor_vertices;
	std::vector<glm::uvec3> floor_faces;
	create_floor(floor_vertices, floor_faces);

	// FIXME: add code to create bone and cylinder geometry

	Mesh mesh;
	mesh.loadpmd(argv[1]);
	std::cout << "Loaded object  with  " << mesh.vertices.size()
		<< " vertices and " << mesh.faces.size() << " faces.\n";

	glm::vec4 mesh_center = glm::vec4(0.0f);
	for (size_t i = 0; i < mesh.vertices.size(); ++i) {
		mesh_center += mesh.vertices[i];
	}
	mesh_center /= mesh.vertices.size();

	LineMesh line_mesh;
	// create_default(line_mesh);
	create_linemesh(line_mesh, mesh.skeleton);
	create_cylinder(mesh.cylinder, mesh.skeleton, 1);
	create_coordinate(mesh.coordinate,mesh.skeleton,1);

	// for(int i = 0; i < line_mesh.vertices.size(); ++i){
	// 	std::cout<<glm::to_string(line_mesh.vertices[i])<<std::endl;
	// }
	/*
	 * GUI object needs the mesh object for bone manipulation.
	 */
	gui.assignMesh(&mesh);

	glm::vec4 light_position = glm::vec4(0.0f, 100.0f, 0.0f, 1.0f);
	MatrixPointers mats; // Define MatrixPointers here for lambda to capture
	

	/*
	 * In the following we are going to define several lambda functions to bind Uniforms.
	 * 
	 * Introduction about lambda functions:
	 *      http://en.cppreference.com/w/cpp/language/lambda
	 *      http://www.stroustrup.com/C++11FAQ.html#lambda
	 */


	auto matrix_binder = [](int loc, const void* data) {
		glUniformMatrix4fv(loc, 1, GL_FALSE, (const GLfloat*)data);
	};
	// auto bone_matrix_binder = [&mesh](int loc, const void* data) {
	// 	auto nelem = mesh.getNumberOfBones();
	// 	glUniformMatrix4fv(loc, nelem, GL_FALSE, (const GLfloat*)data);
	// };
	auto vector_binder = [](int loc, const void* data) {
		glUniform4fv(loc, 1, (const GLfloat*)data);
	};
	auto vector3_binder = [](int loc, const void* data) {
		glUniform3fv(loc, 1, (const GLfloat*)data);
	};
	auto float_binder = [](int loc, const void* data) {
		glUniform1fv(loc, 1, (const GLfloat*)data);
	};
	/*
	 * These lambda functions below are used to retrieve data
	 */
	auto std_model_data = [&mats]() -> const void* {
		return mats.model;
	}; // This returns point to model matrix
	
	glm::mat4 bone_model_matrix = glm::mat4(1.0f);
	auto bone_model_data = [&bone_model_matrix]() -> const void* {
		return &bone_model_matrix[0][0];
	}; 
	glm::mat4 cylinder_model_matrix = glm::mat4(1.0f);
	auto cylinder_model_data = [&cylinder_model_matrix]() -> const void* {
		return &cylinder_model_matrix[0][0];
	}; 
	glm::mat4 coordinate_model_matrix = glm::mat4(1.0f);
	auto coordinate_model_data = [&coordinate_model_matrix]() -> const void* {
		return &coordinate_model_matrix[0][0];
	}; 

	glm::mat4 floor_model_matrix = glm::mat4(1.0f);
	auto floor_model_data = [&floor_model_matrix]() -> const void* {
		return &floor_model_matrix[0][0];
	}; // This return model matrix for the floor.

	auto std_view_data = [&mats]() -> const void* {
		return mats.view;
	};
	auto std_camera_data  = [&gui]() -> const void* {
		return &gui.getCamera()[0];
	};
	auto std_proj_data = [&mats]() -> const void* {
		return mats.projection;
	};
	auto std_light_data = [&light_position]() -> const void* {
		return &light_position[0];
	};
	auto alpha_data  = [&gui]() -> const void* {
		static const float transparet = 0.5; // Alpha constant goes here
		static const float non_transparet = 1.0;
		if (gui.isTransparent())
			return &transparet;
		else
			return &non_transparet;
	};

	// FIXME: add more lambdas for data_source if you want to use RenderPass.
	//        Otherwise, do whatever you like here
	ShaderUniform std_model = { "model", matrix_binder, std_model_data };
	ShaderUniform floor_model = { "model", matrix_binder, floor_model_data };
	ShaderUniform std_view = { "view", matrix_binder, std_view_data };
	ShaderUniform std_camera = { "camera_position", vector3_binder, std_camera_data };
	ShaderUniform std_proj = { "projection", matrix_binder, std_proj_data };
	ShaderUniform std_light = { "light_position", vector_binder, std_light_data };
	ShaderUniform object_alpha = { "alpha", float_binder, alpha_data };
	/*---------------LineMesh------------------------*/
	ShaderUniform line_mesh_model = {"model", matrix_binder, bone_model_data};
	ShaderUniform cylinder_mesh_model = {"model", matrix_binder, cylinder_model_data};
	ShaderUniform coordinate_mesh_model = {"model", matrix_binder, coordinate_model_data};

	// FIXME: define more ShaderUniforms for RenderPass if you want to use it.
	//        Otherwise, do whatever you like here

	std::vector<glm::vec2>& uv_coordinates = mesh.uv_coordinates;
	RenderDataInput object_pass_input;
	object_pass_input.assign(0, "vertex_position", nullptr, mesh.vertices.size(), 4, GL_FLOAT);
	object_pass_input.assign(1, "normal", mesh.vertex_normals.data(), mesh.vertex_normals.size(), 4, GL_FLOAT);
	object_pass_input.assign(2, "uv", uv_coordinates.data(), uv_coordinates.size(), 2, GL_FLOAT);
	object_pass_input.assign_index(mesh.faces.data(), mesh.faces.size(), 3);
	object_pass_input.useMaterials(mesh.materials);
	RenderPass object_pass(-1,
			object_pass_input,
			{
			  vertex_shader,
			  geometry_shader,
			  fragment_shader
			},
			{ std_model, std_view, std_proj,
			  std_light,
			  std_camera, object_alpha },
			{ "fragment_color" }
			);

	// FIXME: Create the RenderPass objects for bones here.
	//        Otherwise do whatever you like.
	RenderDataInput bone_pass_input;
	bone_pass_input.assign(0,"vertex_position", nullptr, line_mesh.vertices.size(),4, GL_FLOAT);
	bone_pass_input.assign_index(line_mesh.bone_lines.data(), line_mesh.bone_lines.size(),2);
	RenderPass bone_pass(-1,
			bone_pass_input,
			{
				vertex_shader,
				line_mesh_geometry_shader,
				bones_fragment_shader
			},
			{ line_mesh_model, std_view, std_proj,
			  std_light},
			{ "fragment_color" }
			);

	RenderDataInput cylinder_pass_input;
	cylinder_pass_input.assign(0,"vertex_position", nullptr, mesh.cylinder.vertices.size(),4, GL_FLOAT);
	cylinder_pass_input.assign_index(mesh.cylinder.bone_lines.data(), mesh.cylinder.bone_lines.size(),2);
	RenderPass cylinder_pass(-1,
			cylinder_pass_input,
			{
				vertex_shader,
				line_mesh_geometry_shader,
				cylinder_fragment_shader
			},
			{ cylinder_mesh_model, std_view, std_proj,
			  std_light},
			{ "fragment_color" }
			);

	RenderDataInput coordinate_pass_input;
	coordinate_pass_input.assign(0,"vertex_position", nullptr, mesh.coordinate.vertices.size(),4, GL_FLOAT);
	coordinate_pass_input.assign(1,"color", mesh.coordinate.color.data(), mesh.coordinate.color.size(),4,GL_FLOAT);
	coordinate_pass_input.assign_index(mesh.coordinate.bone_lines.data(), mesh.coordinate.bone_lines.size(),2);
	RenderPass coordinate_pass(-1,
			coordinate_pass_input,
			{
				vertex_shader,
				line_mesh_geometry_shader,
				coordinate_fragment_shader
			},
			{ coordinate_mesh_model, std_view, std_proj,
			  std_light},
			{ "fragment_color" }
			);

	RenderDataInput floor_pass_input;
	floor_pass_input.assign(0, "vertex_position", floor_vertices.data(), floor_vertices.size(), 4, GL_FLOAT);
	floor_pass_input.assign_index(floor_faces.data(), floor_faces.size(), 3);
	RenderPass floor_pass(-1,
			floor_pass_input,
			{ vertex_shader, geometry_shader, floor_fragment_shader},
			{ floor_model, std_view, std_proj, std_light },
			{ "fragment_color" }
			);
	float aspect = 0.0f;
	std::cout << "center = " << mesh.getCenter() << "\n";

	bool draw_floor = false;
	bool draw_skeleton = true;
	bool draw_object = true;
	bool draw_cylinder = true;

	GLfloat quadVertices[] = {   // Vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // Positions   // TexCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };	

    // TODO: @@@@@@@@@@@@@@@@@@@@@@@@@@@@@2
	GLuint quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
    glBindVertexArray(0);

    // setup screen shader program
    GLuint screen_shader_vert_id = 0;
    CHECK_GL_ERROR(screen_shader_vert_id = glCreateShader(GL_VERTEX_SHADER));
    CHECK_GL_ERROR(glShaderSource(screen_shader_vert_id, 1, &screen_vertex_shader, nullptr));
    glCompileShader(screen_shader_vert_id);
    CHECK_GL_SHADER_ERROR(screen_shader_vert_id);

    GLuint screen_shader_frag_id = 0;
    CHECK_GL_ERROR(screen_shader_frag_id = glCreateShader(GL_FRAGMENT_SHADER));
    CHECK_GL_ERROR(glShaderSource(screen_shader_frag_id, 1, &screen_fragment_shader, nullptr));
    glCompileShader(screen_shader_frag_id);
    CHECK_GL_SHADER_ERROR(screen_shader_frag_id);

    GLuint screen_program_id = 0;
    CHECK_GL_ERROR(screen_program_id = glCreateProgram());
    CHECK_GL_ERROR(glAttachShader(screen_program_id, screen_shader_vert_id));
    CHECK_GL_ERROR(glAttachShader(screen_program_id, screen_shader_frag_id));
    glLinkProgram(screen_program_id);
    CHECK_GL_PROGRAM_ERROR(screen_program_id);

	//Framebuffers
	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	GLuint tex_color_buffer;
	glGenTextures(1, &tex_color_buffer);
	glBindTexture(GL_TEXTURE_2D, tex_color_buffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width, window_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_color_buffer, 0);

	GLuint rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window_width, window_height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR WITH FRAMEBUFFER" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	while (!glfwWindowShouldClose(window)) {
		// Setup some basic window stuff.
		glfwGetFramebufferSize(window, &window_width, &window_height);

		// First pass
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glViewport(0, 0, window_width, window_height);
		//glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClearColor(1.0f, 0.65f, 0.257f, 1.0f);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDepthFunc(GL_LESS);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glCullFace(GL_BACK);

		gui.updateMatrices();
		mats = gui.getMatrixPointers();

		int current_bone = gui.getCurrentBone();
#if 1
		draw_cylinder = (current_bone != -1 && gui.isTransparent());
#else
		draw_cylinder = true;
#endif

		// FIXME: Draw bones first.
		if(gui.isTransparent()){
			create_linemesh(line_mesh, mesh.skeleton);
			
			bone_pass.updateVBO(0, line_mesh.vertices.data(), line_mesh.vertices.size());
			bone_pass.setup();
			CHECK_GL_ERROR(glDrawElements(GL_LINES, line_mesh.bone_lines.size()*2, GL_UNSIGNED_INT, 0));
		}
		if(draw_cylinder ){
			// std::cout<<"Drawing!!!!!"<<std::endl;
			create_cylinder(mesh.cylinder, mesh.skeleton, current_bone);
			create_coordinate(mesh.coordinate,mesh.skeleton,current_bone);

			cylinder_pass.updateVBO(0, mesh.cylinder.vertices.data(), mesh.cylinder.vertices.size());
			coordinate_pass.updateVBO(0, mesh.coordinate.vertices.data(), mesh.coordinate.vertices.size());

			cylinder_pass.setup();
			CHECK_GL_ERROR(glDrawElements(GL_LINES, mesh.cylinder.bone_lines.size()*2, GL_UNSIGNED_INT, 0));
			coordinate_pass.setup();
			CHECK_GL_ERROR(glDrawElements(GL_LINES, mesh.coordinate.bone_lines.size()*2, GL_UNSIGNED_INT, 0));
		}

		// Then draw floor.
		if (draw_floor) {
			floor_pass.setup();
			// Draw our triangles.
			CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, floor_faces.size() * 3, GL_UNSIGNED_INT, 0));
		}
		if (draw_object) {
			if (gui.isPoseDirty()) {
				mesh.updateAnimation();
				object_pass.updateVBO(0,
						      mesh.animated_vertices.data(),
						      mesh.animated_vertices.size());
#if 0
				// For debugging if you need it.
				for (int i = 0; i < 4; i++) {
					std::cerr << " Vertex " << i << " from " << mesh.vertices[i] << " to " << mesh.animated_vertices[i] << std::endl;
				}
#endif
				gui.clearPose();
			}
			object_pass.setup();
			int mid = 0;
			while (object_pass.renderWithMaterial(mid))
				mid++;
#if 0	
			// For debugging also
			if (mid == 0) // Fallback
				CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, mesh.faces.size() * 3, GL_UNSIGNED_INT, 0));
#endif
		}
		last_bone = current_bone;

		// Second pass
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);

		// screen_quad_pass.setup();
		glUseProgram(screen_program_id);
		glBindVertexArray(quadVAO);
        glBindTexture(GL_TEXTURE_2D, tex_color_buffer);	// Use the color attachment texture as the texture of the quad plane
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

		// Poll and swap.
		glfwPollEvents();
		glfwSwapBuffers(window);
	}
	glDeleteFramebuffers(1, &fbo);
	glfwDestroyWindow(window);
	glfwTerminate();
#if 0
	for (size_t i = 0; i < images.size(); ++i)
		delete [] images[i].bytes;
#endif
	exit(EXIT_SUCCESS);
}

