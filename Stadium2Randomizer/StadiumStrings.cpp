#include "StadiumStrings.h"

#include <array>
#include <exception>
#include <stdexcept>

std::string Utf8ToStadiumString(const std::string& utf8Str) {
	//stadium 2 only understands some signs, they have weird numbers,
	//and if it doesnt understand things it crashes in battle (but not select screen).
	//i dont know which letters are fine, so i will make a conservative estimate based on 
	//things i saw in the game.
	//noteworthy are that <># have special meaning and %+* seem to be missing

	//single-char characters that are known to work and have the same value here as in the game
	constexpr char directKnown[] = " ():;[]1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz'-?!.&/,";
	constexpr char knownUtf8[] = u8"é¥♂♀×";
	constexpr uint8_t knownUtf8Values[] = { 0xE9, 0xA5, 0xA9, 0xBE ,0xD7 };

	std::string pushed;
	pushed.reserve(utf8Str.size());

	for (int i = 0; i < utf8Str.size(); i++) {
		char c = utf8Str[i];
		if (c & 0x80) {
			//multi-byte utf 8 char; look in utf8 list
			int offsetJ = -1;
			bool matched = false;
			for (int j = 0; j < _countof(knownUtf8); j++) {
				if ((knownUtf8[j] & 0xC0) != 0x80) {
					offsetJ++;
				}
				if (knownUtf8[j] == c) {
					int i2 = i;
					i2++, j++;
					while (true) {
						if (i2 == utf8Str.size() || j == _countof(knownUtf8) || ((knownUtf8[j] & 0xC0) != 0x80)) {
							matched = true;
							break;
						}
						if (utf8Str[i2] != knownUtf8[j]) break;
						i2++, j++;
					}
					if (matched) {
						pushed.push_back(knownUtf8Values[offsetJ++]);
						break;
					}
				}
			}
			if (!matched) {
				throw std::invalid_argument(std::string("untranslatable character in ") + utf8Str);
			}
		}
		else {
			//single-byte utf 8 char
			constexpr std::array<bool, 128> knownCharMap = ([&]() constexpr {
				std::array<bool, 128> arr{ false };
				for (int i = 0; i < _countof(directKnown); i++) arr[directKnown[i]] = true;
				return arr;
			})();
			if (!knownCharMap[c]) {
				throw std::invalid_argument(std::string("untranslatable character in ") + utf8Str);
			}
			pushed.push_back(c);
		}
	}

	return pushed;
}