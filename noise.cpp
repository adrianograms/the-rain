#include "include/noise.hpp"

noise::noise(uint64_t mx, uint64_t my) : mx(mx), my(my){
    this->noise_map = new uint8_t*[this->my];
    for(uint64_t i = 0; i < this->my; i++){
        this->noise_map[i] = new uint8_t[this->mx];
    }
}

noise::~noise(){
    for(uint64_t i = 0; i < this->my; i++){
        delete [] this->noise_map[i];
    }
    delete [] this->noise_map;
}

void noise::gen(uint64_t seed){
    // seed for std::rand()
    std::srand(seed);

// #pragma omp parallel for schedule(dynamic, 1) collapse(2)
    for(uint64_t i = 0; i < this->my; i++){
        for(uint64_t j = 0; j < this->mx; j++){
            if(std::rand() % 3 == 0){
                this->noise_map[i][j] = (uint8_t)(std::rand()% 256);
            }
            else{
                this->noise_map[i][j] = 0;
            }
        }
    }
}

void noise::smooth(uint16_t **kernel, uint64_t kernel_size){
    uint8_t **rough = nullptr;

    // the sum of all elemes of the kernel
    uint64_t kernel_sum = 0;
    uint64_t kernel_off = kernel_size / 2;

#define round_x(x, off) round((x), (off), true)
#define round_y(y, off) round((y), (off), false)

    auto round = [&](uint64_t x, int64_t off, bool _x) -> uint64_t {
        if(off < 0){
            return (uint64_t)std::abs(off) > x ? (_x ? this->mx : this->my) + off : x + off;
        }
        else if(off > 0){
            return (x + off) % (_x ? this->mx : this->my);
        }
        return x;
    };

    auto convolute = [&](uint64_t y, uint64_t x) -> uint8_t{
        uint64_t sum = 0;

        for(uint64_t i = 0; i < kernel_size; i++){
            for(uint64_t j = 0; j < kernel_size; j++){
                sum += rough[round_y(y, -kernel_off + i)][round_x(x, -kernel_off + j)] * kernel[i][j];
            }
        }

        return sum / kernel_sum > 255 ? 255 : (uint8_t)(sum / kernel_sum);
    };

    // get the sum of the elements in the kernel
    for(uint64_t i = 0; i < kernel_size; i++){
        for(uint64_t j = 0; j < kernel_size; j++){
            kernel_sum += kernel[i][j];
        }
    }

    // allocate the rough matrix
    rough = new uint8_t*[this->my];
    for(std::size_t i = 0; i < this->my; i++){
        rough[i] = new uint8_t[this->mx];
    }

    std::swap(this->noise_map, rough)
;
    // do the convolution
// #pragma omp parallel for schedule(dynamic, 1) collapse(2)
    for(uint64_t i = 0; i < this->my; i++){
        for(uint64_t j = 0; j < this->mx; j++){
            this->noise_map[i][j] = convolute(i, j);
        }
    }

    // free the rough matrix
    for(uint64_t i = 0; i < this->my; i++){
         delete[] rough[i];
    }
    delete[] rough;
}

uint8_t noise::at(uint64_t x, uint64_t y){
    return this->noise_map[y][x];
}
