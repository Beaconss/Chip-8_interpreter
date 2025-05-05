#include "chip8.h"
#include "platform.h"

#include <SFML/Graphics.hpp>

#include <iostream>
#include <filesystem>

int main(int argc, const char* argv[])
{
    if(argv[1] == nullptr) 
    {
        std::cerr << "No rom path supplied";
        return 1;
    }

    std::filesystem::path filePath = argv[1];

    if(std::filesystem::exists(filePath)) std::cout << "Path exists: " << filePath << "\n";
    else
    {
        std::cout << "Path does not exist: " << filePath << "\n";
        return 1;
    }

    Chip8 chip8;
    chip8.loadRom(filePath.string().c_str());

    Platform platform;
    platform.mainLoop(chip8);
}