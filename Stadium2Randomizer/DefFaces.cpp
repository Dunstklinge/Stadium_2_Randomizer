#include "stdafx.h"
#include "DefFaces.h"

DefFaces * DefFaces::FromFile(const char * file)
{
	std::ifstream stream(file, std::ifstream::binary);

	DefFaces* defRoster = FromFileStream(stream);

	return defRoster;
}

DefFaces * DefFaces::FromFileStream(std::istream & in)
{
	if (in.bad()) return nullptr;

	DefFaces* defRoster = (DefFaces*)new uint8_t[segSize];
	in.seekg(offStart);
	in.read((char*)defRoster, segSize);

	return defRoster;
}

bool DefFaces::Faces::Face::SetPixelsFromImage(CImage& img)
{
	int bpp = img.GetBPP();
	int pitch = img.GetPitch();
	
	if (img.GetWidth() % 4 != 0) {
		throw std::invalid_argument(std::string("Images in Stadium must have a width divisible by 4 due to weird shuffling rules; "
									"the supplied image had a size of " + img.GetWidth()));
	}

	if (bpp == 32 || bpp == 24) {
		int bytePP = bpp / 8;
		uint8_t* imgPixels = (uint8_t*)img.GetBits();
		uint16_t* pixels = this->pixels;
		for (int y = 0; y < img.GetHeight(); y++) {
			uint8_t* linePtr = (uint8_t*)imgPixels;
			uint16_t* pixelLinePtr = pixels;
			for (int x = 0; x < img.GetWidth(); x++) {
				uint8_t r, g, b;
				r = linePtr[x*bytePP+2], g = linePtr[x*bytePP+1], b = linePtr[x*bytePP+0];
				//pixel values are shuffled in this weird way, which also kinda assumes alignment by 4 bytes
				int xShuffle = y % 2 == 0 ? 0 : (x % 4 < 2 ? 2 : -2);
				//bits in stadium pixels are rrrrrgggggbbbbba
				pixelLinePtr[x + xShuffle] =  (r >> 3 & 0x1F) << 11
											| (g >> 3 & 0x1F) << 6
											| (b >> 3 & 0x1F) << 1
											| 1;
			}
			imgPixels += pitch;
			pixels += this->width;
		}
	}
	else {
		throw std::invalid_argument(std::string("Cant interpret images with bpp != 32 or 24; bpp was " + bpp));
	}
	return false;
}
