#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <cstdint>

int main() {
    size_t length = 1024; // Length of the memory area
    int protection = PROT_READ | PROT_WRITE; // Memory protection flags
    int flags = MAP_PRIVATE | MAP_ANONYMOUS; // Mapping type flags
    int fd = -1; // Since we are using MAP_ANONYMOUS, fd is set to -1
    off_t offset = 0;

    // Allocate a memory area
    void* addr = mmap(NULL, length, protection, flags, fd, offset);
    if (addr == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    // Now addr points to a memory area that can be read and written
    // Write to the memory area
    char* message = "Hello, mmap!";
    uint8_t* u8_message = reinterpret_cast<uint8_t*>(message);
    memcpy(addr, u8_message, strlen(message) + 1); // +1 to copy the null terminator

    // Read from the memory area and print
    printf("Read from memory1: %s\n", (char*)addr);
    printf("Addr1: %p\n", addr);

    // Optionally, verify that the write was successful
    if (strcmp((char*)addr, message) == 0) {
        printf("Memory write and read successful.\n");
    } else {
        printf("Memory write and read failed.\n");
    }

    // Clean up: Unmap the memory area
    if (munmap(addr, length) == -1) {
        perror("munmap failed");
        // Decide whether to return an error code here, depending on your needs
    }


    return 0;
}
