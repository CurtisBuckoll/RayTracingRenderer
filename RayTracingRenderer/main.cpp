#include <SDL/SDL.h>
#include <stdlib.h>
#include <cstdlib>
#include <string>
#include <SDL/SDL_render.h>

#include "Window.h"
#include "SimpIO.h"
#include "Renderer.h"
#include "InputManager.h"
#include "Camera.h"
#include "FrameRateLimiter.h"
#include "RayTracer.h"
#include "utility.h"

// Global parameters
std::string FILEPATH = "./Cubes.simp";
const int WIN_WIDTH = 650;
const int WIN_HEIGHT = 650;
const int FPS = 25;
const int PRINT_FPS_FREQ = 25;
const double MOVESPEED = 0.10;

// Entry
int main(int argc, char** argv)
{
	if (argc == 2)
	{
		FILEPATH = std::string(argv[1]);
	}

	// Create window
	Window window(WIN_WIDTH, WIN_HEIGHT);
	window.init();

	// Vertex data
	PolygonList* polygonData = new PolygonList();
	Lighting* lightEngine = new Lighting();

	// Read data from simp
	SimpIO file(FILEPATH, lightEngine, polygonData);
	RenderArgs renderParams = file.Read();

	// Initialize renderer
	RayTracer rayTracer(&window, lightEngine, polygonData, renderParams.f_params);

	// Generate scene
	rayTracer.run();

	// Keep window displayed
	bool running = true;
	while (running) 
	{
		SDL_Event evnt;
		while (SDL_PollEvent(&evnt))
		{
			if (evnt.type == SDL_QUIT)
			{
				running = false;
			}
		}
	}

	// Exit
	SDL_Quit();
	return 0;
}