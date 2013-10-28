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

static struct RendererState {
	// orthographic projection matrix
	float ortho_l, ortho_r, ortho_t, ortho_b, ortho_n, ortho_f;
	float orthoProj[16];

	// vertex grid dimensions
	unsigned int gridHCount, gridVCount; // number of vertices horizontally and vertically
	float gridWidth, gridHeight;		 // scaled dimensions on the grid in world units

} g_state;

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

void populate(std::vector<float> &verts, std::vector<unsigned int> &tris)
{
	verts.clear();
	tris.clear();

	float xSep = 1.0f / (g_state.gridHCount-1);
	float ySep = 1.0f / (g_state.gridVCount-1);

	for(unsigned int x = 0; x < g_state.gridHCount; x++)
	{
		for(unsigned int y = 0; y < g_state.gridHCount; y++)
		{
			// position
			verts.push_back(x * xSep * g_state.gridWidth);
			verts.push_back(y * ySep * g_state.gridHeight);
			verts.push_back(0.5f);
			verts.push_back(1.0f);
			
			float color = 1 - (((x + y) % 2) | (x % 2));

			// color
			verts.push_back(color);
			verts.push_back(color);
			verts.push_back(color);
			verts.push_back(1.0f);
		}
	}

	// create triangle strips
	for(unsigned int row = 0; row < g_state.gridVCount-1; row++)
	{
		for(unsigned int x = 0; x < g_state.gridHCount; x++)
		{
			tris.push_back((row+1) * g_state.gridHCount + x);
			tris.push_back(row * g_state.gridHCount + x);
		}
	}
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


GLint projectionLocation;
GLint thresholdLocation;

void init()
{
	initShaders();

	// get "pointers" to uniforms in the shaders
	projectionLocation = glGetUniformLocation(shaderProgram, "projection");
	thresholdLocation = glGetUniformLocation(shaderProgram, "threshold");

	g_state.ortho_n = -1;
	g_state.ortho_f = 5;
	g_state.ortho_l = 0;
	g_state.ortho_r = 1;
	g_state.ortho_t = 0;
	g_state.ortho_b = 1;

	g_state.orthoProj[0] = 2.0f/(g_state.ortho_r - g_state.ortho_l);
	g_state.orthoProj[5] = 2.0f/(g_state.ortho_t - g_state.ortho_b);
	g_state.orthoProj[10] = -2.0f/(g_state.ortho_f - g_state.ortho_n);
	g_state.orthoProj[12] = (g_state.ortho_l + g_state.ortho_r)/(g_state.ortho_l - g_state.ortho_r);
	g_state.orthoProj[13] = (g_state.ortho_t + g_state.ortho_b)/(g_state.ortho_b - g_state.ortho_t);
	g_state.orthoProj[14] = (g_state.ortho_f + g_state.ortho_n)/(g_state.ortho_f - g_state.ortho_n);
	g_state.orthoProj[15] = 1;

	std::vector<float> vertData;
	std::vector<unsigned int> triangleData;

	g_state.gridVCount = g_state.gridHCount = 10;
	g_state.gridWidth = g_state.gridHeight = 1.0f;

	populate(vertData, triangleData);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vertBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, vertBufferObject);
	glBufferData(GL_ARRAY_BUFFER, vertData.size() * 8 * sizeof(float), &vertData[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &triIndexBufferObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triIndexBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangleData.size() * sizeof(int), &triangleData[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// enable backface culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	// enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);

	glUseProgram(shaderProgram);
	glUniformMatrix4fv(projectionLocation,1,GL_FALSE,g_state.orthoProj);
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

void ComputeThreshold(float &t)
{
    const float fLoopDuration = 20.0f;
    const float fScale = 3.14159f * 2.0f / fLoopDuration;
    
    float fElapsedTime = SDL_GetTicks() / 1000.0f;
    
    float fCurrTimeThroughLoop = fmodf(fElapsedTime, fLoopDuration);
    
    t = pow(cosf(fCurrTimeThroughLoop * fScale), 2);
}

void render()
{
	//glClearColor(1.0f,0.0f,0.0f,1.0f);
	glClearColor(0.5f,0.5f,0.5f,0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgram);

	float t;
	ComputeThreshold(t);
	glUniform1f(thresholdLocation, t);

	glBindBuffer(GL_ARRAY_BUFFER, vertBufferObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triIndexBufferObject);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (GLvoid *) (4*sizeof(GLfloat)));
	for(unsigned int row = 0; row < g_state.gridVCount - 1; row++)
	{
		glDrawElements(GL_TRIANGLE_STRIP,g_state.gridHCount * 2,GL_UNSIGNED_INT, (GLvoid *) (sizeof(GLint) * g_state.gridHCount * 2 * row)); 
	}

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