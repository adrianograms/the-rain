#ifndef NOISE_H
#define NOISE_H

#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <algorithm>

class noise{
    uint8_t **_noise_map;

    uint64_t _mx;               // map size in x
    uint64_t _my;               // map size in y

    uint16_t **_kernel;

    void _allc_noise_map();
    void _free_noise_map();

    void _allc_kernel();
    void _free_kernel();

public:
    noise(uint64_t, uint64_t);

    void update(uint64_t, uint64_t); // upadte the noise map size
    void gen(uint64_t);         // generates a random noise_map based on the seed
    void smooth();              // convolutes the kernel on the generated map, to smooth it

    uint8_t * operator[](uint64_t) const;

    ~noise();
};

#endif
