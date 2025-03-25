#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <iostream>
#include <fstream>
#include <random>
#include <chrono>
#include <thread>
#include <array>
#include <atomic>

using Byte = uint8_t;

namespace Config
{
	constexpr int DISPLAY_WIDTH {64};
	constexpr int DISPLAY_HEIGHT {32};
	constexpr int CYCLES_PER_SECOND {600};
}

class Chip8
{
public:
	Chip8();

	void loadRom(char const* filePath);
	void cycle();
	void handleInput(const sf::Event& event);

	Byte* getDisplay();

private:
	uint16_t m_opcode{};

	std::array<Byte, 4096> m_memory{};
	uint16_t m_pc{};
	std::array<Byte, 16> m_V{};
	uint16_t m_I{};

	std::array<uint16_t, 16> m_stack{};
	uint16_t m_sp{};

	std::atomic<Byte> m_delayTimer{};
	std::atomic<Byte> m_soundTimer{};

	std::array<Byte, 16> m_keypad{};
	Byte m_display[Config::DISPLAY_WIDTH * Config::DISPLAY_HEIGHT]{};

	sf::SoundBuffer m_soundBuffer{};
	sf::Sound m_sound{};
    
	std::mt19937 initializeRandEngine();
	std::mt19937 m_randEngine{};

	void initialize60hzTimer();
	void Timer60hz();

	void execute();

	Byte randByte();
	Byte getX();
	Byte getY();
	Byte getN();
	Byte getNN();
	uint16_t getNNN();

	void op_00E0();
	void op_00EE();
	void op_1NNN();
	void op_2NNN();
	void op_3XNN();
	void op_4XNN();
	void op_5XY0();
	void op_6XNN();
	void op_7XNN();
	void op_8XY0();
	void op_8XY1();
	void op_8XY2();
	void op_8XY3();
	void op_8XY4();
	void op_8XY5();
	void op_8XY6();
	void op_8XY7();
	void op_8XYE();
	void op_9XY0();
	void op_ANNN();
	void op_BXNN();
	void op_CXNN();
	void op_DXYN();
	void op_EXA1();
	void op_EX9E();
	void op_FX07();
	void op_FX0A();
	void op_FX15();
	void op_FX18();
	void op_FX1E();
	void op_FX29();
	void op_FX33();
	void op_FX55();
	void op_FX65();
};

