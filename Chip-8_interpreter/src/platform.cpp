#include "platform.h"

Platform::Platform()
	: m_window {sf::VideoMode(Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT), "Chip-8"}
	, m_cycleDelay{sf::seconds(1.0f / Config::CYCLES_PER_SECOND)}
{
	m_displayTex.create(Config::DISPLAY_WIDTH, Config::DISPLAY_HEIGHT);
	m_displaySprite.setTexture(m_displayTex);
	m_displaySprite.setScale(Config::WINDOW_WIDTH / Config::DISPLAY_WIDTH, Config::WINDOW_HEIGHT / Config::DISPLAY_HEIGHT);
	m_displayImage.create(Config::DISPLAY_WIDTH, Config::DISPLAY_HEIGHT, sf::Color::Black);
}

void Platform::mainLoop(Chip8& chip8)
{
	while(m_window.isOpen())
	{
		sf::Time startTime = m_frameClock.getElapsedTime();

		sf::Event event;
		while(m_window.pollEvent(event))
		{
			if(event.type == sf::Event::Closed)
			{
				m_window.close();
				return;
			}
			chip8.handleInput(event);
		}

		chip8.cycle();
		draw(chip8);
		
		sf::Time endTime {m_frameClock.getElapsedTime()};
		sf::Time elapsed {endTime - startTime};
		if(elapsed < m_cycleDelay)
		{
			sf::sleep(m_cycleDelay - elapsed);
		}
		m_frameClock.restart();
	}
}

void Platform::draw(Chip8& chip8)
{
	for(int i {0}; i < Config::DISPLAY_WIDTH * Config::DISPLAY_HEIGHT; ++i)
	{
		Byte currentColor = chip8.getDisplay()[i] ? 255 : 0;
		m_displayImage.setPixel(i % Config::DISPLAY_WIDTH, i / Config::DISPLAY_WIDTH, sf::Color(currentColor, currentColor, currentColor));
	}

	m_displayTex.update(m_displayImage);

	m_window.clear();
	m_window.draw(m_displaySprite);
	m_window.display();
}