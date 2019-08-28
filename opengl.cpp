#include <cstring>
#include <cstdio>
#include <thread>
#include <vector>
#include <GL/glew.h>
#include <GL/gl.h>

#include "simple/graphical/initializer.h"
#include "simple/interactive/initializer.h"
#include "simple/interactive/event.h"
#include "simple/graphical/software_window.h"
#include "simple/graphical/gl_window.h"
#include "simple/graphical/algorithm.h"

using namespace std::chrono_literals;
using namespace simple;
using namespace graphical::color_literals;

using graphical::int2;
using graphical::range2D;
using graphical::float2;
using graphical::anchored_rect;
using graphical::gl_window;

int main() try
{
	graphical::initializer graphics;
	interactive::initializer input;

	gl_window::global.require<gl_window::attribute::major_version>(2);
	gl_window::global.require<gl_window::attribute::minor_version>(1);
	gl_window::global.request<gl_window::attribute::stencil>(8);
	gl_window win("opengl", int2(600, 600), gl_window::flags::borderless);
	win.require_vsync(gl_window::vsync_mode::enabled);
	float2 win_size(win.size());

	glewInit();

	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	// Compile Vertex Shader
	char const * VertexSourcePointer = R"shader(
		#version 120
		#extension GL_ARB_explicit_attrib_location : require

		layout(location = 0) in vec3 vertexPosition_modelspace;

		void main()
		{
			gl_Position.xyz = vertexPosition_modelspace;
			gl_Position.w = 1.0;
		}
	)shader";
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , nullptr);
	glCompileShader(VertexShaderID);
	// Check Vertex Shader
	GLint Result = GL_FALSE;
	int InfoLogLength;
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, VertexShaderErrorMessage.data());
		printf("%s\n", VertexShaderErrorMessage.data());
	}

	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	// Compile Fragment Shader
	char const * FragmentSourcePointer = R"shader(
		#version 120
		#extension GL_ARB_explicit_attrib_location : require

		out vec3 color;

		void main()
		{
			color = vec3(1,0,0);
		}

	)shader";
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);
	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, FragmentShaderErrorMessage.data());
		printf("%s\n", FragmentShaderErrorMessage.data());
	}

	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);
	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, ProgramErrorMessage.data());
		printf("%s\n", ProgramErrorMessage.data());
	}

	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	glClearColor(0,0,0.4,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(ProgramID);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	auto vertex_data = geom::vector {
		geom::vector(-1.0f, -1.0f, 0.0f),
		geom::vector(1.0f, -1.0f, 0.0f),
		geom::vector(0.0f,  1.0f, 0.0f),
	};
	vertex_data /= 5.f;
	// This will identify our vertex buffer
	GLuint vertexbuffer;
	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexbuffer);
	// The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	// Give our vertices to OpenGL.

	bool done = false;
	const auto x = geom::vector(1.f, 0.f, 0.f);
	while(!done)
	{
		using namespace interactive;
		while(auto e = next_event())
		{
			std::visit(support::overloaded
			{
				[&](quit_request){ done = true; },
				[](auto&&){}
			}, *e);
		}
		glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), &vertex_data, GL_STATIC_DRAW);
		// 1st attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
				0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
				);
		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
		glDisableVertexAttribArray(0);

		if(vertex_data[2] >= x)
		{
			vertex_data -= x * 2;
		}
		vertex_data += x * 0.03;
		win.update();
		// std::this_thread::sleep_for(16ms);
	}

	glDeleteBuffers(1, &vertexbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);

	return 0;
}
catch(...)
{
	if(errno)
		std::puts(std::strerror(errno));
	throw;
}
