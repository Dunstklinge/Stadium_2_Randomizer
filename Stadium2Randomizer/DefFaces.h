#pragma once
#include "DefGenTable.h"
#include "Constants.h"

class DefFaces
{
public:
	DefFaces() = delete;
	DefFaces(const DefFaces& rhs) = delete;

	DefGenTable tables;

	uint8_t facesBuffer[1]; //rest
	

	struct Faces {
		uint32_t nFaces;

		uint32_t offsets[1]; //offsets point to pixels, not the start of the face struct
		//uint32_t offsets[nFaces];

		struct Face {
			uint16_t width;
			uint16_t height;
			uint32_t unknown;	//determines render type? all faces have 0x20000.
								//image looks garbled in different ways if 0x10000 or 0x30000, 0x40000 crashed
								//probably isnt even a uint32
			uint16_t pixels[1]; //width*height

			bool SetPixelsFromImage(CImage& img);
			inline void Curate(bool forth) {
				if (!forth) {
					for (int i = 0; i < width*height; i++) SwitchEndianness(pixels[i]);
					SwitchEndianness(width);
					SwitchEndianness(height);
					SwitchEndianness(unknown);
				}
				if (forth) {
					SwitchEndianness(width);
					SwitchEndianness(height);
					SwitchEndianness(unknown);
					for (int i = 0; i < width*height; i++) SwitchEndianness(pixels[i]);
				}
			}
			Face() = delete; Face(const Face&) = delete;
		};
		inline Face* face(int n) { return (Face*) (((uint8_t*)this) + offsets[n] - 8); }

		inline void Curate(bool forth) {
			if (!forth) {
				for (int i = 0; i < nFaces; i++) face(i)->Curate(forth);
				for (int i = 0; i < nFaces; i++) SwitchEndianness(offsets[i]);
				SwitchEndianness(nFaces);
			}
			if (forth) {
				SwitchEndianness(nFaces);
				for (int i = 0; i < nFaces; i++) SwitchEndianness(offsets[i]);
				for (int i = 0; i < nFaces; i++) face(i)->Curate(forth);
			}
		}

		Faces() = delete; Faces(const Faces&) = delete;
	};

	inline void Curate(bool forth) {
		if (!forth) {
			for (auto& it : *this) {
				it.Curate(forth);
			}
			tables.Curate(forth);
		}
		if (forth) {
			tables.Curate(forth);
			for (auto& it : *this) {
				it.Curate(forth);
			}
		}
	}

	typedef DefGenTable::Iterator<Faces, 0> FacesIterator;

	inline FacesIterator begin() {
		FacesIterator it;
		it.n = 0;
		it.tables = &tables;
		return it;
	}

	inline FacesIterator end() {
		FacesIterator it;
		it.n = tables.nTables;
		it.tables = &tables;
		return it;
	}

	static constexpr unsigned long offStart = 0x2268000;
	static constexpr unsigned long offEnd = 0x23a5000;
	static constexpr unsigned long segSize = offEnd - offStart;

	static DefFaces* FromFile(const char* file);
	static DefFaces* FromFileStream(std::istream& in);
};