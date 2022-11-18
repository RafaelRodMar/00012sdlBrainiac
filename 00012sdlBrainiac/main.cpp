#include<SDL.h>
#include<sdl_ttf.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <list>
#include <cmath>
#include <fstream>
#include <sstream>
#include <time.h>
#include <random>
#include <chrono>
#include "game.h"

class Rnd {
public:
	std::mt19937 rng;

	Rnd()
	{
		std::mt19937 prng(std::chrono::steady_clock::now().time_since_epoch().count());
		rng = prng;
	}

	int getRndInt(int min, int max)
	{
		std::uniform_int_distribution<int> distribution(min, max);
		return distribution(rng);
	}

	double getRndDouble(double min, double max)
	{
		std::uniform_real_distribution<double> distribution(min, max);
		return distribution(rng);
	}
} rnd;

//la clase juego:
Game* Game::s_pInstance = 0;

Game::Game()
{
	m_pRenderer = NULL;
	m_pWindow = NULL;
}

Game::~Game()
{

}

SDL_Window* g_pWindow = 0;
SDL_Renderer* g_pRenderer = 0;

bool Game::init(const char* title, int xpos, int ypos, int width,
	int height, bool fullscreen)
{
	// almacenar el alto y ancho del juego.
	m_gameWidth = width;
	m_gameHeight = height;

	// attempt to initialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{
		int flags = 0;
		if (fullscreen)
		{
			flags = SDL_WINDOW_FULLSCREEN;
		}

		std::cout << "SDL init success\n";
		// init the window
		m_pWindow = SDL_CreateWindow(title, xpos, ypos,
			width, height, flags);
		if (m_pWindow != 0) // window init success
		{
			std::cout << "window creation success\n";
			m_pRenderer = SDL_CreateRenderer(m_pWindow, -1, 0);
			if (m_pRenderer != 0) // renderer init success
			{
				std::cout << "renderer creation success\n";
				SDL_SetRenderDrawColor(m_pRenderer,
					255, 255, 255, 255);
			}
			else
			{
				std::cout << "renderer init fail\n";
				return false; // renderer init fail
			}
		}
		else
		{
			std::cout << "window init fail\n";
			return false; // window init fail
		}
	}
	else
	{
		std::cout << "SDL init fail\n";
		return false; // SDL init fail
	}
	if (TTF_Init() == 0)
	{
		std::cout << "sdl font initialization success\n";
	}
	else
	{
		std::cout << "sdl font init fail\n";
		return false;
	}

	std::cout << "init success\n";
	m_bRunning = true; // everything inited successfully, start the main loop

	//Joysticks
	InputHandler::Instance()->initialiseJoysticks();

	//load images, sounds, music and fonts
	AssetsManager::Instance()->loadAssets();
	Mix_Volume(-1, 16); //adjust sound/music volume for all channels

	ReadHiScores();

	state = MENU;

	return true;
}

void Game::render()
{
	SDL_RenderClear(m_pRenderer); // clear the renderer to the draw color

	if (state == MENU)
	{
		AssetsManager::Instance()->Text("Menu \n press space to play", "font", 100, 100, SDL_Color({ 0,0,0,0 }), getRenderer());

		////Show hi scores
		int y = 350;
		AssetsManager::Instance()->Text("HiScores", "font", 580 - 50, y, SDL_Color({ 255,255,255,0 }), getRenderer());
		for (int i = 0; i < 5; i++) {
			y += 30;
			AssetsManager::Instance()->Text(std::to_string(vhiscores[i]), "font", 580, y, SDL_Color({ 255,255,255,0 }), getRenderer());
		}
	}

	if (state == GAME)
	{
		//draw the tiles
		int tileWidth = 132; //tilewidth
		int tileHeight = 127; //tileheight

		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				if (tileStates[i][j] || ((i == tile1.m_x) && (j == tile1.m_y)) || ((i == tile2.m_x) && (j == tile2.m_y)))
				{
					std::string aux = "tile" + to_string(tiles[i][j]);
					AssetsManager::Instance()->draw(aux, i * 132, j * 127, 132, 127, m_pRenderer);
				}
				else
				{
					AssetsManager::Instance()->draw("tileblank", i * 132, j * 127, 132, 127, m_pRenderer);
				}
			}
		}

		// Draw the score
		std::string sc = "Score: " + std::to_string(score);
		AssetsManager::Instance()->Text(sc, "font", 0, 0, SDL_Color({ 255,255,255,0 }), getRenderer());
	}

	if (state == END_GAME)
	{
		AssetsManager::Instance()->Text("End Game \n press space", "font", 100, 100, SDL_Color({ 0,0,0,0 }), Game::Instance()->getRenderer());
	}

	SDL_RenderPresent(m_pRenderer); // draw to the screen
}

void Game::quit()
{
	m_bRunning = false;
}

void Game::clean()
{
	WriteHiScores();
	std::cout << "cleaning game\n";
	InputHandler::Instance()->clean();
	AssetsManager::Instance()->clearFonts();
	TTF_Quit();
	SDL_DestroyWindow(m_pWindow);
	SDL_DestroyRenderer(m_pRenderer);
	Game::Instance()->m_bRunning = false;
	SDL_Quit();
}

void Game::handleEvents()
{
	InputHandler::Instance()->update();

	//HandleKeys
	if (state == MENU)
	{
		if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_SPACE))
		{
			state = GAME;
			score = 0;
			//clear the tile states and images
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					tileStates[i][j] = false;
					tiles[i][j] = 0;
				}
			}

			//initialize the tile images randomly
			for (int i = 0; i < 2; i++) {
				for (int j = 0; j < 9; j++) {
					int x = rnd.getRndInt(0, 3);
					int y = rnd.getRndInt(0, 3);
					while (tiles[x][y] != 0) {
						x = rnd.getRndInt(0, 3);
						y = rnd.getRndInt(0, 3);
					}
					tiles[x][y] = j;
				}
			}

			//initialize the tile selections and match/try count
			tile1.m_x = tile1.m_y = -1;
			tile2.m_x = tile2.m_y = -1;
			matches = tries = 0;
		}
	}

	if (state == GAME)
	{
		if (InputHandler::Instance()->getMouseButtonState(LEFT))
		{
			mouseClicked = true;
			mousepos.m_x = InputHandler::Instance()->getMousePosition()->m_x;
			mousepos.m_y = InputHandler::Instance()->getMousePosition()->m_y;
		}
	}

	if (state == END_GAME)
	{
		if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_SPACE)) state = MENU;
	}

}

void Game::update()
{
	if (state == GAME && mouseClicked == true)
	{
		//determine which tile was clicked
		int TileX = mousepos.m_x / 132; //tilewidth
		int TileY = mousepos.m_y / 127; //tileheight

		//make sure the tile hasn't already been matched
		if (!tileStates[TileX][TileY])
		{
			//see if this is the first tile selected
			if (tile1.m_x == -1)
			{
				//play sound for tile selection
				AssetsManager::Instance()->playSound("select", 0);

				//set the first tile selection
				tile1.m_x = TileX;
				tile1.m_y = TileY;
			}
			else if ((TileX != tile1.m_x) || (TileY != tile1.m_y))
			{
				if (tile2.m_x == -1)
				{
					//play a sound for the tile selection
					AssetsManager::Instance()->playSound("select", 0);

					//increase the number of tries
					tries++;

					//set the second tile selection
					tile2.m_x = TileX;
					tile2.m_y = TileY;

					//see if it's a match
					if (tiles[(int)tile1.m_x][(int)tile1.m_y] == tiles[(int)tile2.m_x][(int)tile2.m_y])
					{
						//play a sound for the tile match
						AssetsManager::Instance()->playSound("match", 0);
						score += 100;

						//set the tile state to indicate the match
						tileStates[(int)tile1.m_x][(int)tile1.m_y] = true;
						tileStates[(int)tile2.m_x][(int)tile2.m_y] = true;

						//clear the tile selections
						tile1.m_x = tile1.m_y = tile2.m_x = tile2.m_y = -1;

						//update the match count and check for winner
						if (++matches == 8)
						{
							//play a victory sound
							AssetsManager::Instance()->playSound("win", 0);
							UpdateHiScores(tries);
							state = END_GAME;
						}
					}
					else
					{
						//play a sound for the tile mismatch
						AssetsManager::Instance()->playSound("mismatch", 0);
					}
				}
				else
				{
					//clear the tile selections
					tile1.m_x = tile1.m_y = tile2.m_x = tile2.m_y = -1;
				}
			}
		}
		InputHandler::Instance()->setMouseButtonStatesToFalse();
		mouseClicked = false;
	}
}

void Game::UpdateHiScores(int newscore)
{
	//new score to the end
	vhiscores.push_back(newscore);
	//sort
	sort(vhiscores.rbegin(), vhiscores.rend());
	//remove the last
	vhiscores.pop_back();
}

void Game::ReadHiScores()
{
	std::ifstream in("hiscores.dat");
	if (in.good())
	{
		std::string str;
		getline(in, str);
		std::stringstream ss(str);
		int n;
		for (int i = 0; i < 5; i++)
		{
			ss >> n;
			vhiscores.push_back(n);
		}
		in.close();
	}
	else
	{
		//if file does not exist fill with 5 scores
		for (int i = 0; i < 5; i++)
		{
			vhiscores.push_back(0);
		}
	}
}

void Game::WriteHiScores()
{
	std::ofstream out("hiscores.dat");
	for (int i = 0; i < 5; i++)
	{
		out << vhiscores[i] << " ";
	}
	out.close();
}

const int FPS = 60;
const int DELAY_TIME = 1000.0f / FPS;

int main(int argc, char* args[])
{
	srand(time(NULL));

	Uint32 frameStart, frameTime;

	std::cout << "game init attempt...\n";
	if (Game::Instance()->init("SDLBrainiac", 100, 100, 528, 502,
		false))
	{
		std::cout << "game init success!\n";
		while (Game::Instance()->running())
		{
			frameStart = SDL_GetTicks(); //tiempo inicial

			Game::Instance()->handleEvents();
			Game::Instance()->update();
			Game::Instance()->render();

			frameTime = SDL_GetTicks() - frameStart; //tiempo final - tiempo inicial

			if (frameTime < DELAY_TIME)
			{
				SDL_Delay((int)(DELAY_TIME - frameTime)); //esperamos hasta completar los 60 fps
			}
		}
	}
	else
	{
		std::cout << "game init failure - " << SDL_GetError() << "\n";
		return -1;
	}
	std::cout << "game closing...\n";
	Game::Instance()->clean();
	return 0;
}