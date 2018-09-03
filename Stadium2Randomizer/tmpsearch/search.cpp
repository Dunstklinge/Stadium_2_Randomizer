#include <iostream>
#include <fstream>
#include <set>

const std::set<unsigned char> cNums{0x4E, 0x4F, 0x60, 0x53, 0x54, 0xAD};

std::set<unsigned char> nums = cNums;

unsigned char buffer[0x4000000];

int main() {
    std::ifstream in("Pokemon Stadium 2 (U) [!].z64", std::ifstream::binary);
    in.read((char*)buffer, sizeof buffer);
    unsigned int index = 0;
    for(index; index < 0x4000000; index++) {
        unsigned char c = buffer[index];
        auto it = nums.find(c);
        if(it != nums.end()) {
            nums.erase(it);
            if(nums.size() == 0) {
                nums = cNums;
                std::cout << index << std::endl;
            }
        }
        else if(nums.size() != cNums.size()) {
            int diff = nums.size() - cNums.size();
            nums = cNums;
            index -= diff;
        }
    }

    return 0;
}