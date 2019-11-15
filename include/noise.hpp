#ifndef NOISE_H
#define NOISE_H

#include <cstdint>
#include <cstdlib>
#include <algorithm>

class noise{
private:
    uint8_t **_noise_map;

    uint64_t _mx;               // map size in x
    uint64_t _my;               // map size in y

    uint16_t **_kernel;

public:

    noise(uint64_t, uint64_t);
    ~noise();

    void gen(uint64_t);         // generates a random noise_map based on the seed

    void smooth();              // convolutes the kernel on the generated map, to smooth it

    uint8_t * operator[](uint64_t) const;
};

#endif
