#include "MMU.h"

bool MMU::loadROM(std::string& filename) {
    std::ifstream file(filename);
    file.seekg(0, std::ifstream::end);
    long length = file.tellg();
    file.seekg(0, std::ifstream::beg);

    if (length == -1) {
        printf("Failed to open file\n");
        return false;
    }

    char buffer[MEMORY_SIZE];
    if (length > MEMORY_SIZE) {
        length = MEMORY_SIZE;
    }
    file.read(&buffer[0], length);

    printf("Read file %s, size=%li\n", filename.substr(filename.find_last_of('/')+1, filename.length()).data(), length);
    for (int i=0; i<length; i++) {
        memory[i] = (u8) buffer[i];
    }
    return true;
}

u8* MMU::getMemory() {
    return memory;
}
