#pragma once

#include <vector>
#include <list>
#include <SDL.h>
#include <SDL_image.h>
#include "Vector2D.h"
#include "AssetsManager.h"
#include "InputHandler.h"

////Game States
enum GAMESTATES { SPLASH, MENU, LOAD_LEVEL, GAME, END_GAME };

class Game {
public:
	static Game* Instance()
	{
		if (s_pInstance == 0)
		{
			s_pInstance = new Game();
			return s_pInstance;
		}
		return s_pInstance;
	}

	SDL_Renderer* getRenderer() const { return m_pRenderer; }

	~Game();

	bool init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen);
	void render();
	void update();
	void handleEvents();
	void clean();
	void quit();

	bool running() { return m_bRunning; }

	int getGameWidth() const { return m_gameWidth; }
	int getGameHeight() const { return m_gameHeight; }

	void UpdateHiScores(int newscore);
	void ReadHiScores();
	void WriteHiScores();

private:
	Game();
	static Game* s_pInstance;
	SDL_Window* m_pWindow;
	SDL_Renderer* m_pRenderer;

	int state = -1;
	std::vector<int> vhiscores;
	int score = -1;

	bool tileStates[4][4];
	int tiles[4][4];
	Vector2D tile1, tile2;
	int matches, tries;
	bool mouseClicked = false;
	Vector2D mousepos;

	bool m_bRunning;
	int m_gameWidth;
	int m_gameHeight;
};
