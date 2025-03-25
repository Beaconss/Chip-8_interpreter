#pragma once

#include "chip8.h"

#include <SFML/Graphics.hpp>

namespace Config
{
	constexpr int WINDOW_WIDTH {800};
	constexpr int WINDOW_HEIGHT {600};
}

class Platform
{
public:
	Platform();

	void mainLoop(Chip8& chip8);

private:
	void draw(Chip8& chip8);

	sf::RenderWindow m_window;
	sf::Texture m_displayTex;
	sf::Sprite m_displaySprite;
	sf::Image m_displayImage;
	sf::Clock m_frameClock;
	sf::Time m_cycleDelay;
};

