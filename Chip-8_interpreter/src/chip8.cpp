#include "chip8.h"

constexpr unsigned int START_ADDRESS {0x200};
constexpr int CHARACTERS {16};
constexpr int CHARACTERS_SIZE {5};
constexpr int FONTSET_SIZE {CHARACTERS * CHARACTERS_SIZE};

constexpr std::array<Byte, FONTSET_SIZE> fontset
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

constexpr std::array<Byte, CHARACTERS> fontsetAddresses
{
	0x50,
	0x55,
	0x5A,
	0x5F,
	0x64,
	0x69,
	0x6E,
	0x73,
	0x78,
	0x7D,
	0x82,
	0x87,
	0x8C,
	0x91,
	0x96,
	0x9B,
};


std::mt19937 Chip8::initializeRandEngine()
{
	std::random_device rd {};

	std::seed_seq ss {static_cast<std::seed_seq::result_type>(std::chrono::steady_clock::now().time_since_epoch().count()),
		rd(), rd(), rd(), rd(), rd(), rd(), rd()};

	return std::mt19937 {ss};
}

void Chip8::initialize60hzTimer()
{
	std::thread timerThread(&Chip8::Timer60hz, this);
	timerThread.detach();
}

void Chip8::Timer60hz()
{
	while(true)
	{
		std::this_thread::sleep_for(std::chrono::microseconds(16667));

		if(m_delayTimer > 0)
		{
			--m_delayTimer;
		}

		if(m_soundTimer > 0)
		{
			if(m_sound.getStatus() != sf::Sound::Playing) m_sound.play();
			if(--m_soundTimer == 0) m_sound.stop();
		}
	}
}

Chip8::Chip8()
	: m_opcode{}
	, m_memory{}
	, m_pc{}
	, m_V{}
	, m_I{}
	, m_stack{}
	, m_sp{}
	, m_delayTimer{}
	, m_soundTimer{}
	, m_keypad{}
	, m_display{}
	, m_soundBuffer{}
	, m_sound{}
	, m_randEngine {initializeRandEngine()}
{
	m_pc = START_ADDRESS;

	for(int i {0}; i < FONTSET_SIZE; ++i)
	{
		m_memory[0x50 + i] = fontset[i];
	}

	//make beep sound
	constexpr int sampleCount {48000};
	std::vector<sf::Int16> samples(sampleCount);

	constexpr float frequency = 440.0f;
	constexpr float amplitude = 32767;
	for(int i = 0; i < sampleCount; ++i) 
	{
		samples[i] = static_cast<sf::Int16>(amplitude * std::sin(2 * 3.1415 * frequency * i / sampleCount));
	}

	m_soundBuffer.loadFromSamples(samples.data(), sampleCount, 1, 44100);
	m_sound.setBuffer(m_soundBuffer);

	initialize60hzTimer();
}

void Chip8::loadRom(char const* filePath)
{
	std::ifstream file(filePath, std::ios::binary | std::ios::ate);

	if(file.is_open())
	{
		std::streamsize size {file.tellg()};
		if(size > 4096)
		{
			std::cerr << "File too large\n";
			return;
		}
		
		char* buffer {new char[size]};

		file.seekg(0, std::ios::beg);
		file.read(buffer, size);
		file.close();

		for(int i {0}; i < size; ++i)
		{
			m_memory[START_ADDRESS + i] = buffer[i];
		}

		delete[] buffer;
	}
	else std::cerr << "Failed to open file";
}

void Chip8::cycle()
{
	m_opcode = (m_memory[m_pc] << 8) | m_memory[m_pc+1];
	m_pc += 2;
	execute();
}

void Chip8::execute()  
{  
	switch(m_opcode & 0xF000) //decode instruction
	{  
	case 0x0000:  
		switch(m_opcode & 0x00FF)  
		{  
		case 0x00E0: op_00E0(); return;  
		case 0x00EE: op_00EE(); return;
		default: break;
		} break;
	case 0x1000: op_1NNN(); return;  
	case 0x2000: op_2NNN(); return;  
	case 0x3000: op_3XNN(); return;  
	case 0x4000: op_4XNN(); return;  
	case 0x5000: op_5XY0(); return;  
	case 0x6000: op_6XNN(); return;  
	case 0x7000: op_7XNN(); return;
	case 0x8000:
		switch(m_opcode & 0x000F)
		{
		case 0x0000: op_8XY0(); return;
		case 0x0001: op_8XY1(); return;
		case 0x0002: op_8XY2(); return;
		case 0x0003: op_8XY3(); return;
		case 0x0004: op_8XY4(); return;
		case 0x0005: op_8XY5(); return;
		case 0x0006: op_8XY6(); return;
		case 0x0007: op_8XY7(); return;
		case 0x000E: op_8XYE(); return;
		default: break;
		} break;
	case 0x9000: op_9XY0(); return;  
	case 0xA000: op_ANNN(); return;  
	case 0xB000: op_BXNN(); return;  
	case 0xC000: op_CXNN(); return;  
	case 0xD000: op_DXYN(); return;  
	case 0xE000:
		switch(m_opcode & 0x000F)
		{
		case 0x0001: op_EXA1(); return;
		case 0x000E: op_EX9E(); return;
		default: break;
		} break;
	case 0xF000:
		switch(m_opcode & 0x00FF)
		{
		case 0x0007: op_FX07(); return;
		case 0x000A: op_FX0A(); return;
		case 0x0015: op_FX15(); return;
		case 0x0018: op_FX18(); return;
		case 0x001E: op_FX1E(); return;
		case 0x0029: op_FX29(); return;
		case 0x0033: op_FX33(); return;
		case 0x0055: op_FX55(); return;
		case 0x0065: op_FX65(); return;
		default: break;
		} break;
	default: break;  
	}  
}

void Chip8::handleInput(const sf::Event& event)
{
	switch(event.type)
	{
	case sf::Event::KeyPressed:
	{
		switch(event.key.code)
		{
		case sf::Keyboard::Num1: m_keypad[1] = 1; break;
		case sf::Keyboard::Num2: m_keypad[2] = 1; break;
		case sf::Keyboard::Num3: m_keypad[3] = 1; break;
		case sf::Keyboard::Num4: m_keypad[0xC] = 1; break;
		case sf::Keyboard::Q: m_keypad[4] = 1; break;
		case sf::Keyboard::W: m_keypad[5] = 1; break;
		case sf::Keyboard::E: m_keypad[6] = 1; break;
		case sf::Keyboard::R: m_keypad[0xD] = 1; break;
		case sf::Keyboard::A: m_keypad[7] = 1; break;
		case sf::Keyboard::S: m_keypad[8] = 1; break;
		case sf::Keyboard::D: m_keypad[9] = 1; break;
		case sf::Keyboard::F: m_keypad[0xE] = 1; break;
		case sf::Keyboard::Z: m_keypad[0xA] = 1; break;
		case sf::Keyboard::X: m_keypad[0] = 1; break;
		case sf::Keyboard::C: m_keypad[0xB] = 1; break;
		case sf::Keyboard::V: m_keypad[0xF] = 1; break;
		}
	} break;
	case sf::Event::KeyReleased:
	{
		switch(event.key.code)
		{
		case sf::Keyboard::Num1: m_keypad[1] = 0; break;
		case sf::Keyboard::Num2: m_keypad[2] = 0; break;
		case sf::Keyboard::Num3: m_keypad[3] = 0; break;
		case sf::Keyboard::Num4: m_keypad[0xC] = 0; break;
		case sf::Keyboard::Q: m_keypad[4] = 0; break;
		case sf::Keyboard::W: m_keypad[5] = 0; break;
		case sf::Keyboard::E: m_keypad[6] = 0; break;
		case sf::Keyboard::R: m_keypad[0xD] = 0; break;
		case sf::Keyboard::A: m_keypad[7] = 0; break;
		case sf::Keyboard::S: m_keypad[8] = 0; break;
		case sf::Keyboard::D: m_keypad[9] = 0; break;
		case sf::Keyboard::F: m_keypad[0xE] = 0; break;
		case sf::Keyboard::Z: m_keypad[0xA] = 0; break;
		case sf::Keyboard::X: m_keypad[0] = 0; break;
		case sf::Keyboard::C: m_keypad[0xB] = 0; break;
		case sf::Keyboard::V: m_keypad[0xF] = 0; break;
		}
	} break;
	}
}

Byte* Chip8::getDisplay()
{
	return m_display;
}

uint8_t Chip8::randByte()
{
	return std::uniform_int_distribution<unsigned int>{0, 255u}(m_randEngine);
}

Byte Chip8::getX()
{
	return (m_opcode & 0x0F00) >> 8;
}

Byte Chip8::getY()
{
	return (m_opcode & 0x00F0) >> 4;
}

Byte Chip8::getN()
{
	return (m_opcode & 0x000F);
}

Byte Chip8::getNN()
{
	return (m_opcode & 0x00FF);
}

uint16_t Chip8::getNNN()
{
	return (m_opcode & 0x0FFF);
}

void Chip8::op_00E0()
{
	//std::cout << "op_00E0 executing" << '\n';
	std::fill(std::begin(m_display), std::end(m_display), 0);
}

void Chip8::op_00EE()
{
	//std::cout << "op_00EE: Return to 0x" << std::hex << m_stack[sp] << "\n";
	m_pc = m_stack[--m_sp];
}

void Chip8::op_1NNN()
{
	uint16_t NNN = getNNN();
	//std::cout << "op_1NNN: Jump to 0x" << std::hex << NNN << " | m_pc = 0x" << m_pc << "\n";

	m_pc = NNN;
}

void Chip8::op_2NNN()
{
	//std::cout << "op_2NNN executing" << '\n';
	uint16_t NNN = getNNN();

	m_stack[m_sp++] = m_pc;
	m_pc = NNN;
}

void Chip8::op_3XNN()
{
	Byte X = getX();
	Byte NN = getNN();
	//std::cout << "op_3XNN: m_V[" << static_cast<int>(X) << "] = " << static_cast<int>(m_V[X]) << " m_Vs NN = " << static_cast<int>(NN) << '\n';

	if(m_V[X] == NN) m_pc += 2;
}

void Chip8::op_4XNN()
{
	//std::cout << "op_4XNN executing" << '\n';
	Byte X = getX();
	Byte NN = getNN();

	if(m_V[X] != NN) m_pc += 2;
}

void Chip8::op_5XY0()
{
	//std::cout << "op_5XY0 executing" << '\n';
	Byte X = getX();
	Byte Y = getY();

	if(m_V[X] == m_V[Y]) m_pc += 2;
}

void Chip8::op_6XNN()
{
	//std::cout << "op_6XNN executing" << '\n';
	Byte X = getX();
	Byte NN = getNN();

	m_V[X] = NN;
}

void Chip8::op_7XNN()
{
	Byte X = getX();
	Byte NN = getNN();
	//std::cout << "op_7XNN: m_V[" << static_cast<int>(X) << "] += " << static_cast<int>(NN) << " | m_pc = 0x" << m_pc << "\n";

	m_V[X] = m_V[X] + NN;
}

void Chip8::op_8XY0()
{
	//std::cout << "op_8XY0 executing" << '\n';
	Byte X = getX();
	Byte Y = getY();

	m_V[X] = m_V[Y];
}

void Chip8::op_8XY1()
{
	//std::cout << "op_8XY1 executing" << '\n';
	Byte X = getX();
	Byte Y = getY();

	m_V[X] = m_V[X] | m_V[Y];
}

void Chip8::op_8XY2()
{
	//std::cout << "op_8XY2 executing" << '\n';
	Byte X = getX();
	Byte Y = getY();

	m_V[X] = m_V[X] & m_V[Y];
}

void Chip8::op_8XY3()
{
	//std::cout << "op_8XY3 executing" << '\n';
	Byte X = getX();
	Byte Y = getY();

	m_V[X] = m_V[X] ^ m_V[Y];
}

void Chip8::op_8XY4()
{
	//std::cout << "op_8XY4 executing" << '\n';
	Byte X = getX();
	Byte Y = getY();

	uint16_t result = m_V[X] + m_V[Y];
	m_V[X] = result;

	m_V[0xF] = result > 255;
}

void Chip8::op_8XY5()
{
	//std::cout << "op_8XY5 executing" << '\n';
	Byte X = getX();
	Byte Y = getY();

	Byte m_VF = m_V[X] >= m_V[Y];

	if(X == 0xF) 
	{
		m_V[0xF] = m_VF;
	}
	else 
	{
		m_V[X] = m_V[X] - m_V[Y];
		m_V[0xF] = m_VF;
	}
}

void Chip8::op_8XY6()
{
	//std::cout << "op_8XY6 executing" << '\n';
	Byte X = getX();
	Byte Y = getY();
	
	m_V[0xF] = m_V[X] & 0x01;

	if(X != 0xF)
	{
		//m_V[X] = m_V[Y]; //CONFIGURABLE
		m_V[0xF] = m_V[X] & 0x01;
		m_V[X] = m_V[X] >> 1;
	}
}

void Chip8::op_8XY7()
{
	//std::cout << "op_8XY7 executing" << '\n';
	Byte X = getX();
	Byte Y = getY();

	Byte m_VF = m_V[Y] >= m_V[X];

	if(X == 0xF)
	{
		m_V[0xF] = m_VF;
	}
	else
	{
		m_V[X] = m_V[Y] - m_V[X];
		m_V[0xF] = m_VF;
	}
}

void Chip8::op_8XYE()
{
	//std::cout << "op_8XYE executing" << '\n';
	Byte X = getX();
	Byte Y = getY();

	m_V[0xF] = (m_V[X] & 0x80) >> 7;

	if(X != 0xF)
	{
		//m_V[X] = m_V[Y]; //CONFIGURABLE
		m_V[0xF] = (m_V[X] & 0x80) >> 7;
		m_V[X] = m_V[X] << 1;
	}
}

void Chip8::op_9XY0()
{
	//std::cout << "op_9XY0 executing" << '\n';
	Byte X = getX();
	Byte Y = getY();

	if(m_V[X] != m_V[Y]) m_pc += 2;
}

void Chip8::op_ANNN()
{
	//std::cout << "op_ANNN executing" << '\n';
	uint16_t NNN = getNNN();

	m_I = NNN;
}

void Chip8::op_BXNN()
{
	//std::cout << "op_BXNN executing" << '\n';
	Byte X = getX();
	uint16_t XNN = getNNN();

	m_pc = XNN + m_V[X];
}

void Chip8::op_CXNN()
{
	//std::cout << "op_CXNN executing" << '\n';
	Byte X = getX();
	Byte NN = getNN();

	m_V[X] = randByte() & NN;
}

void Chip8::op_DXYN()
{
	//std::cout << "op_DXYN executing" << '\n';
	Byte X = getX();
	Byte Y = getY();
	Byte N = getN();

	Byte Xcoord = m_V[X] % Config::DISPLAY_WIDTH;
	Byte Ycoord = m_V[Y] % Config::DISPLAY_HEIGHT;
	m_V[0xF] = 0;

	for(int row {0}; row < N; ++row)
	{
		Byte spriteData = m_memory[m_I + row];
		Xcoord = m_V[X] % Config::DISPLAY_WIDTH;

		for(int col {0}; col < 8; ++col)
		{
			unsigned int index = Xcoord + Ycoord * Config::DISPLAY_WIDTH;
			Byte bit = (spriteData >> (7 - col)) & 0x01;

			if(bit && m_display[index])
			{
				m_display[index] = 0;
				m_V[0xF] = 1;
			}
			else if(bit && !m_display[index])
			{
				m_display[index] = 1;
			}
			++Xcoord;

			if(Xcoord >= Config::DISPLAY_WIDTH) break;
		}

		++Ycoord;
		if(Ycoord >= Config::DISPLAY_HEIGHT) break;
	}
}

void Chip8::op_EXA1()
{
	//std::cout << "op_EXA1 executing" << '\n';
	Byte X = getX();

	if(!m_keypad[m_V[X]]) m_pc += 2;
}

void Chip8::op_EX9E()
{
	//std::cout << "op_EX9E executing" << '\n';
	Byte X = getX();

	if(m_keypad[m_V[X]]) m_pc += 2;
}

void Chip8::op_FX07()
{
	//std::cout << "op_FX07 executing" << '\n';
	Byte X = getX();

	//std::lock_guard<std::mutex> lock(timerMutex);
	m_V[X] = m_delayTimer;
}

void Chip8::op_FX0A()
{
	//std::cout << "op_FX0A executing" << '\n';
	Byte X = getX();

	bool keyPressed {false};

	for(Byte i {0}; i < 16; ++i)
	{
		if(m_keypad[i])
		{
			m_V[X] = i;
			keyPressed = true;
			break;
		}
	}

	if(!keyPressed) m_pc -= 2;
}

void Chip8::op_FX15()
{
	//std::cout << "op_FX15 executing" << '\n';
	Byte X = getX();

	m_delayTimer = m_V[X];
}

void Chip8::op_FX18()
{
	//std::cout << "op_FX18 executing" << '\n';
	Byte X = getX();

	m_soundTimer = m_V[X];
}

void Chip8::op_FX1E()
{
	//std::cout << "op_FX1E executing" << '\n';
	Byte X = getX();

	if(m_I + m_V[X] >= 0x1000) m_V[0xF] = 1;
	m_I = m_I + m_V[X];
}

void Chip8::op_FX29()
{
	//std::cout << "op_FX29 executing" << '\n';
	Byte X = getX();

	m_I = fontsetAddresses[m_V[X]];
}

void Chip8::op_FX33()
{
	//std::cout << "op_FX33 executing" << '\n';
	Byte X = getX();
	
	Byte m_Value = m_V[X];

	m_memory[m_I] = m_Value / 100;
	m_memory[m_I + 1] = (m_Value % 100) / 10;
	m_memory[m_I + 2] = m_Value % 10;
}

void Chip8::op_FX55()
{
	//std::cout << "op_FX55 executing" << '\n';
	Byte X = getX();

	for(int i {0}; i <= X; ++i)
	{
		m_memory[m_I + i] = m_V[i];
	}
}

void Chip8::op_FX65()
{
	//std::cout << "op_FX65 executing" << '\n';
	Byte X = getX();

	for(int i {0}; i <= X; ++i)
	{
		m_V[i] = m_memory[m_I + i];
	}
}