#include "Crc.h"

#include <array>
#include <stdint.h>

#include "Util.h"




//from http://n64dev.org/n64crc.html
//slightly modified cause its a c program so it kinda looks like ass
/* snesrc - SNES Recompiler
 *
 * Mar 23, 2010: addition by spinout to actually fix CRC if it is incorrect
 *
 * Copyright notice for this file:
 *  Copyright (C) 2005 Parasyte
 *
 * Based on uCON64's N64 checksum algorithm by Andreas Sterbenz
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
namespace {

	constexpr uint32_t N64_HEADER_SIZE = 0x40;
	constexpr uint32_t N64_BC_SIZE = (0x1000 - N64_HEADER_SIZE);

	constexpr std::array<uint32_t, 256> CalcCrcTable() {
		std::array<uint32_t, 256> ret = {};

		constexpr unsigned int poly = 0xEDB88320;//standard IEEE crc 32 polynomal
		for (int i = 0; i < 256; i++) {
			unsigned int crc = i;
			for (int j = 0; j < 8; j++) {
				if (crc & 1) crc = (crc >> 1) ^ poly;
				else crc >>= 1;
			}
			ret[i] = crc;
		}
		return ret;
	}
	constexpr std::array<unsigned int, 256> CrcTable = CalcCrcTable();

	uint32_t Crc32(unsigned char* data, int len) {
		uint32_t crc = 0xFFFFFFFF;
		for (int i = 0; i < len; i++) {
			crc = (crc >> 8) ^ CrcTable[(crc ^ data[i]) & 0xFF];
		}

		return ~crc;
	}


	//should always be 6103 for stadium 2
	int GetN64Cic(unsigned char *data) {
		switch (Crc32(&data[N64_HEADER_SIZE], N64_BC_SIZE)) {
		case 0x6170A4A1: return 6101;
		case 0x90BB6CB5: return 6102;
		case 0x0B050EE0: return 6103;
		case 0x98BC2C86: return 6105;
		case 0xACC8580A: return 6106;
		}

		return 6105;
	}
	uint32_t GetN64CicChecksum(int cic) {
		constexpr uint32_t CHECKSUM_CIC6102 = 0xF8CA4DDC;
		constexpr uint32_t CHECKSUM_CIC6103 = 0xA3886759;
		constexpr uint32_t CHECKSUM_CIC6105 = 0xDF26F436;
		constexpr uint32_t CHECKSUM_CIC6106 = 0x1FEA617A;
		switch (cic) {
		case 6101:
		case 6102:
			return CHECKSUM_CIC6102;
			break;
		case 6103:
			return CHECKSUM_CIC6103;
			break;
		case 6105:
			return CHECKSUM_CIC6105;
			break;
		case 6106:
			return CHECKSUM_CIC6106;
			break;
		default:
			return 0;
		}
	}

	bool N64CalcCRC(unsigned int crc[2], unsigned char* data) {
		int bootcode = GetN64Cic(data);
		unsigned int seed = GetN64CicChecksum(bootcode);
		if (seed == 0) return false;

		uint32_t t1, t2, t3, t4, t5, t6;;
		t1 = t2 = t3 = t4 = t5 = t6 = seed;

		//security through obscurity i guess
		for (unsigned int i = CHECKSUM_START; i < CHECKSUM_END; i += 4) {
			uint32_t d = *((uint32_t*)&data[i]);
			SwitchEndianness(d);
			if ((t6 + d) < t6) t4++;
			t6 += d;
			t3 ^= d;
			uint32_t r = d;
			Shl(r, (r & 0x1F));
			t5 += r;
			if (t2 > d) t2 ^= r;
			else t2 ^= t6 ^ d;

			if (bootcode == 6105) {
				uint32_t val = *((uint32_t*)&data[N64_HEADER_SIZE + 0x0710 + (i & 0xFF)]);
				SwitchEndianness(val);
				t1 += val ^ d;
			}
			else t1 += t5 ^ d;
		}


		if (bootcode == 6103) {
			crc[0] = (t6 ^ t4) + t3;
			crc[1] = (t5 ^ t2) + t1;
		}
		else if (bootcode == 6106) {
			crc[0] = (t6 * t4) + t3;
			crc[1] = (t5 * t2) + t1;
		}
		else {
			crc[0] = t6 ^ t4 ^ t3;
			crc[1] = t5 ^ t2 ^ t1;
		}

		return true;
	}

}

bool CalculateCrcs(uint32_t out[2], const char* romPath) {
	std::ifstream in(romPath, std::ios::binary);
	if (!in.good()) return false;

	uint8_t (&buffer)[CHECKSUM_END] = (uint8_t(&)[CHECKSUM_END])*(new uint8_t[CHECKSUM_END]);
	in.read((char*)&buffer, CHECKSUM_END);
	bool suc = CalculateCrcs(out, buffer);
	delete[] &buffer;
	return suc;
}

bool CalculateCrcs(uint32_t out[2], std::ifstream& in) {
	in.seekg(0);
	uint8_t(&buffer)[CHECKSUM_END] = (uint8_t(&)[CHECKSUM_END])*(new uint8_t[CHECKSUM_END]);
	in.read((char*)&buffer, CHECKSUM_END);
	bool suc = CalculateCrcs(out, buffer);
	delete[] & buffer;
	return suc;
}

bool CalculateCrcs(uint32_t out[2], uint8_t* romBuffer) {
	return N64CalcCRC(out, romBuffer);
}