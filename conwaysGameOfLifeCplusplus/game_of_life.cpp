#include <SDL.h>
#include <memory>
#include <ctime>
#include <stdio.h>
#include <windows.h>


#define OFF_COLOUR 0x00
#define ON_COLOUR 0xFF

#define LIMIT_RATE 0 // Limit loop rate for visibility
#define TICK_RATE 50 // Tick-rate in milliseconds (if LIMIT_RATE == 1)


//The window we'll be rendering to
SDL_Window* window = NULL;
//The surface contained by the window
SDL_Surface* screenSurface = NULL;

// Width & Height of cell in pixels
// Influences screen size
unsigned int CELL_SIZE = 2;
// Randomisation seed
unsigned int seed;

unsigned CELLMAP_WIDTH = 500;
unsigned CELLMAP_HEIGHT = 500;

//Screen dimensions
unsigned int SCREEN_WIDTH = CELLMAP_WIDTH * CELL_SIZE;
unsigned int SCREEN_HEIGHT = CELLMAP_HEIGHT * CELL_SIZE;


class CellMap
{
public:
	CellMap(int w, int h);
	~CellMap();

	void SetCell(int x, int y);
	void ClearCell(int x, int y);
	void Init();
	int CellState(int x, int y);
	void NextGen();

private:
	char* cells;
	char* temp_cells;
	int width, height;
	int length;
};


void DrawCell(int x, int y, int colour)
{
	// Pointer to pixel array
	// Only stores values from 0-255
	// Multiply by 4 because pixel array stores RGBA
	// Each pixel thus uses 4 memory locations
	Uint8* pixel_ptr = (Uint8*)screenSurface->pixels + (y * CELL_SIZE * SCREEN_WIDTH + x * CELL_SIZE) * 4;

	for (unsigned int i = 0; i < CELL_SIZE; i++)
	{
		for (unsigned int j = 0; j < CELL_SIZE; j++)
		{
			// RGB setup
			*(pixel_ptr + j * 4) = colour;
			*(pixel_ptr + j * 4 + 1) = colour;
			*(pixel_ptr + j * 4 + 2) = colour;
		}
		// Getting to the next row of pixel array
		pixel_ptr += SCREEN_WIDTH * 4;
	}
}

// Constructor
// Using initializer list here also
CellMap::CellMap(int w, int h)
{
	width = w;
	height = h;
	length = w * h;
	cells = new char[length];
	temp_cells = new char[length];

	memset(cells, 0, length);
}

// Destructor
CellMap::~CellMap()
{
	// Deallocate
	delete[] cells;
	delete[] temp_cells;
}

void CellMap::SetCell(int x, int y)
{
	int w = width, h = height;
	int xleft, xright, ytop, ybottom;
	// Pointer to where we want to assign
	// This formula indexes a 2D array to 1D array
	char* cell_ptr = cells + (y * w) + x;
	

	// "Pacman edges" a.k.a. Calculate offsets


	*(cell_ptr) |= 0x01; // Set first bit as 1, "alive"

	// Left of screen
	if (x == 0)
		xleft = w - 1;
	else
		xleft = -1;

	// Right of screen
	if (x == (w - 1))
		xright = -(w - 1);
	else
		xright = 1;

	// Top of screen
	if (y == 0)
		ytop = length - w;
	else
		ytop = -w;

	// Bottom of screen
	if (y == (h - 1))
		ybottom = -(length - w);
	else
		ybottom = w;

	// Increment neighbour count by 1
	// Add 2 because we start from 1 bit, not 0 bit.
	*(cell_ptr + ytop + xleft) += 0x02;
	*(cell_ptr + ytop) += 0x02;
	*(cell_ptr + ytop + xright) += 0x02;
	*(cell_ptr + xleft) += 0x02;
	*(cell_ptr + xright) += 0x02;
	*(cell_ptr + ybottom + xleft) += 0x02;
	*(cell_ptr + ybottom) += 0x02;
	*(cell_ptr + ybottom + xright) += 0x02;
}

int CellMap::CellState(int x, int y)
{
	char* cell_ptr = cells + (y * width) + x;

	// The first bit of cell tells if it's "Alive" or "Dead"
	return *cell_ptr & 0x01;
}

void CellMap::Init()
{
	unsigned int x, y, init_length;

	// Get seed; random if 0
	// Makes sure we don't always draw the same game
	seed = (unsigned)time(NULL);

	// Randomly initialise cell map with ~50% on pixels
	printf("Initializing\n");

	srand(seed);

	//for (int i = 0; i < length * 0.5; i++)
	init_length = (width * height) / 2;
	do
	{
		x = rand() % (width - 1);
		y = rand() % (height - 1);
		// Set cell alive ONLY if it's dead
		if (!CellState(x, y))
			SetCell(x, y);
	} while (--init_length);
}


void CellMap::ClearCell(int x, int y)
{
	int w = width, h = height;
	int xleft, xright, ytop, ybottom;

	char* cell_ptr = cells + (y * w) + x;



	*(cell_ptr) &= ~0x01; // Set first bit as 0, "dead"

	// Left of screen
	if (x == 0)
		xleft = w - 1;
	else
		xleft = -1;

	// Right of screen
	if (x == (w - 1))
		xright = -(w - 1);
	else
		xright = 1;

	// Top of screen
	if (y == 0)
		ytop = length - w;
	else
		ytop = -w;

	// Bottom of screen
	if (y == (h - 1))
		ybottom = -(length - w);
	else
		ybottom = w;

	// Increment neighbour count by 1
	// Add 2 because we start from 1 bit, not 0 bit.
	*(cell_ptr + ytop + xleft) -= 0x02;
	*(cell_ptr + ytop) -= 0x02;
	*(cell_ptr + ytop + xright) -= 0x02;
	*(cell_ptr + xleft) -= 0x02;
	*(cell_ptr + xright) -= 0x02;
	*(cell_ptr + ybottom + xleft) -= 0x02;
	*(cell_ptr + ybottom) -= 0x02;
	*(cell_ptr + ybottom + xright) -= 0x02;
}

void CellMap::NextGen()
{
	unsigned int x, y, live_neighbours;
	unsigned int h = height, w = width;
	char* cell_ptr;

	memcpy(temp_cells, cells, length);

	cell_ptr = temp_cells;

	for (int y = 0; y < h; y++)
	{
		x = 0;
		// do & while loop because we have to skip cells that are dead and have no neighbours
		do
		{
			// Skipping
			while (*cell_ptr == 0)
			{
				cell_ptr++; // Goes to next item

				if (++x >= w) goto NextRow;
			}
			// Remember:
			// 0, 0, 0, 1, 0, 0, 0, 1
			//                   ^  ^
			//     neighbout_count  cell_state
			live_neighbours = *cell_ptr >> 1;

			// Check if cell is on
			if (*cell_ptr & 0x01)
			{
				// Check if cell should stay on
				// In Conway's Game of Life, a cell can ONLY be alive if it has either 2 or 3 neighbours
				if ((live_neighbours != 2) && (live_neighbours != 3))
				{
					// Turn cell off
					ClearCell(x, y);
					DrawCell(x, y, OFF_COLOUR);
				}
			}
			else
			{
				// If cell is off, Check if cell should turn on
				if (live_neighbours == 3)
				{
					// Turn cell on
					SetCell(x, y);
					DrawCell(x, y, ON_COLOUR);
				}
			}

			cell_ptr++;


		} while (++x < w);

	// Have a label to skip to
	NextRow:;
	}
}

int main(int argc, char* argv[])
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
	}
	else
	{
		//Create window
		window = SDL_CreateWindow("Conway's Game of Life", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
			SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

		if (window == NULL)
		{
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		}
		else
		{
			//Get window surface
			screenSurface = SDL_GetWindowSurface(window);

			// Generation counter
			long generation = 0;

			// SDL Event Handler
			SDL_Event event;

			// Init map
			CellMap map(CELLMAP_WIDTH, CELLMAP_HEIGHT);
			map.Init();

			// Rendering loop
			bool quit = false;
			while (!quit)
			{
				while (SDL_PollEvent(&event) != 0)
					if (event.type == SDL_QUIT) quit = true;

				generation++;

				// Recalculate and draw next generation
				map.NextGen();

				// Update frame buffer
				SDL_UpdateWindowSurface(window);

#if LIMIT_RATE
				SDL_Delay(TICK_RATE);
#endif
			}

			// Clean up SDL
			SDL_DestroyWindow(window);
			SDL_Quit();

			//cout << "Total Generations: " << generation<< "\nSeed: " << SEED << endl;

			printf("Total Generations: %f\nSeed:%u\n", generation, seed);

			////Hack to get window to stay up
			//SDL_Event e; bool quit = false; while (quit == false) { while (SDL_PollEvent(&e)) { if (e.type == SDL_QUIT) quit = true; } }
		}
	}

	return 0;
}