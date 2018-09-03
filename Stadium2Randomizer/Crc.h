#pragma once

#include <fstream>
#include <stdint.h>

/*
 * n64 CRC is a regular CRC combined with some random fucking xors ops depending on something called cic.
 * no idea how it works, but luckily there is an online program to copy pasta. it
 * has one of these stupid ass license copy pastas, but its not like anybody is ever gonna read this.
 */


constexpr uint32_t CHECKSUM_START = 0x00001000;
constexpr uint32_t CHECKSUM_END =   0x00101000;

bool CalculateCrcs(uint32_t out[2], const char* romPath);
bool CalculateCrcs(uint32_t out[2], std::ifstream& stream);
bool CalculateCrcs(uint32_t out[2], uint8_t* romBuffer); //romBuffer needs to be from 0x0 to CHECKSUM_END