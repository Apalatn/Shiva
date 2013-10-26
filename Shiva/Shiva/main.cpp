#include <iostream>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <SDL.h>

#ifdef __PORTABLE
#define GLEW_STATIC
#endif

#include <GL\glew.h>

#define GL_GLEXT_PROTOTYPES

#include <SDL_OpenGL.h>
#include "Utilities\ErrorUtils.h"
#include "Utilities\ShaderUtils.h"
#include "Utilities\FileUtils.h"

void render();
void init();
void cleanup();
void resize(int w, int h);

// shader program handle
GLuint shaderProgram;

// vertex position buffer handle
GLuint vertBufferObject;

// triangle index buffer handle
GLuint triIndexBufferObject;

// vertex array object handle
GLuint vao;

// loaded scene data
const struct aiScene *scene = 0;

int main(int argc, char *argv[])
{
	/* Request opengl 3.2 context.
     * SDL doesn't have the ability to choose which profile at this time of writing,
     * but it should default to the core profile */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	// initialize basic SDL with all the bells and whistles
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
		std::getchar();
		return 1;
	}

	// create the window for looking at things
	SDL_Window *win = SDL_CreateWindow("Hello World!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
		600, 600, SDL_WINDOW_SHOWN|SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
	if (win == nullptr)
	{
		std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		std::getchar();
		return 1;
	}

    /* Turn on double buffering with a 24bit Z buffer.
     * You may need to change this to 16 or 32 for your system */
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GLContext oglContext = SDL_GL_CreateContext(win);

	const char *error = SDL_GetError();
    if (*error != '\0')
	{
		std::cout << "SDL_GL_CreateContext Error: " << SDL_GetError() << std::endl;
		std::getchar();
		return 1;
	}

	/* This makes our buffer swap syncronized with the monitor's vertical refresh */
    SDL_GL_SetSwapInterval(1);

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		SEVERE("Glew failed to init.",-1);
	}

	// set up program
	init();

	// Main Loop
	SDL_Event e;
	bool quit = false;
	while (!quit){
		// catch events
		while (SDL_PollEvent(&e)){
			if (e.type == SDL_QUIT)
				quit = true;
			if (e.type == SDL_KEYDOWN)
				switch(e.key.keysym.sym)
				{
				case SDLK_q:
				case SDLK_ESCAPE:	  
					quit = true;
					break;
				default:
					break;
				}
			if (e.type == SDL_WINDOWEVENT)
			{
				if(e.window.event == SDL_WINDOWEVENT_RESIZED)
				{
					resize(e.window.data1, e.window.data2);
				}
			}
		}

		//Render the scene
		render();
		SDL_GL_SwapWindow(win);
	}

	cleanup();

	SDL_GL_DeleteContext(oglContext);
	SDL_DestroyWindow(win);
	SDL_Quit();
}

void initShaders()
{
	std::vector<GLuint> shaderList;

	shaderList.push_back(CompileShader(GL_VERTEX_SHADER, get_file_contents("Shaders\\VertexShader.glsl")));
	shaderList.push_back(CompileShader(GL_FRAGMENT_SHADER, get_file_contents("Shaders\\FragmentShader.glsl")));
	shaderList.push_back(CompileShader(GL_GEOMETRY_SHADER, get_file_contents("Shaders\\GeometryShader.glsl")));

	shaderProgram = LinkProgram(shaderList);

	std::for_each(shaderList.begin(), shaderList.end(), glDeleteShader);
}


GLint offsetUniformLocation;
GLint zNearUniformLocation;
GLint zFarUniformLocation;
GLint frustumScaleUniformLocation;

void init()
{
	initShaders();

	// get "pointers" to uniforms in the shaders
	offsetUniformLocation = glGetUniformLocation(shaderProgram, "offset");
	zNearUniformLocation = glGetUniformLocation(shaderProgram, "zNear");
	zFarUniformLocation = glGetUniformLocation(shaderProgram, "zFar");
	frustumScaleUniformLocation = glGetUniformLocation(shaderProgram, "frustumScale");

	std::vector<float> vertData;

	std::vector<unsigned int> triangleData;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	//glGenBuffers(1, &vertBufferObject);
	//glBindBuffer(GL_ARRAY_BUFFER, vertBufferObject);
	//glBufferData(GL_ARRAY_BUFFER, 0 * 8 * sizeof(float), &vertData[0], GL_STATIC_DRAW);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);

	//glGenBuffers(1, &triIndexBufferObject);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triIndexBufferObject);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0 * 3 * sizeof(int), &triangleData[0], GL_STATIC_DRAW);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	glEnable(GL_DEPTH_TEST);

	glUseProgram(shaderProgram);
	glUniform1f(zNearUniformLocation, 1.0f);
	glUniform1f(zFarUniformLocation, 6.0f);
	glUniform1f(frustumScaleUniformLocation, 1.0f);
	glUseProgram(0);
}

// grabbed from http://www.arcsynthesis.org/gltut/Positioning/Tutorial%2003.html
void ComputePositionOffsets(float &fXOffset, float &fYOffset)
{
    const float fLoopDuration = 5.0f;
    const float fScale = 3.14159f * 2.0f / fLoopDuration;
    
    float fElapsedTime = SDL_GetTicks() / 1000.0f;
    
    float fCurrTimeThroughLoop = fmodf(fElapsedTime, fLoopDuration);
    
    fXOffset = cosf(fCurrTimeThroughLoop * fScale) * 0.5f;
    fYOffset = sinf(fCurrTimeThroughLoop * fScale) * 0.5f;
}

void render()
{
	//glClearColor(1.0f,0.0f,0.0f,1.0f);
	glClearColor(0.5f,0.5f,0.5f,0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgram);

	float xOffset = 0.0f, yOffset = 0.0f;
    ComputePositionOffsets(xOffset, yOffset);

	glUniform2f(offsetUniformLocation, xOffset, yOffset);

	//glBindBuffer(GL_ARRAY_BUFFER, vertBufferObject);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triIndexBufferObject);
	//glEnableVertexAttribArray(0);
	//glEnableVertexAttribArray(1);
	//glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), 0);
	//glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (GLvoid *) (4*sizeof(GLfloat)));
	////glDrawArrays(GL_TRIANGLES, 3, 3);
	//glDrawElements(GL_TRIANGLES,0,GL_UNSIGNED_INT,0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glUseProgram(0);
}

void cleanup()
{
	glDeleteBuffers(1, &vertBufferObject);
	glDeleteBuffers(1, &triIndexBufferObject);
}

void resize(int w, int h)
{
	glViewport(0,0,(GLsizei) w, (GLsizei) h);
}