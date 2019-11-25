#include "include/noise.hpp"

#define KERNEL_SIZE 5

void noise::_allc_noise_map(){
    _noise_map = new uint8_t*[_my];
    for(uint64_t i = 0; i < _my; i++){
        _noise_map[i] = new uint8_t[_mx];
    }
}

void noise::_free_noise_map(){
    for(uint64_t i = 0; i < _my; i++){
        delete [] _noise_map[i];
    }
    delete [] _noise_map;
}

void noise::_allc_kernel(){
    _kernel = new uint16_t*[KERNEL_SIZE];
    for(std::size_t i = 0; i < KERNEL_SIZE; i++){
        _kernel[i] = new uint16_t[KERNEL_SIZE];
    }
}

void noise::_free_kernel(){
    for(uint64_t i = 0; i < KERNEL_SIZE; i++){
        delete [] _kernel[i];
    }
    delete [] _kernel;
}

noise::noise(uint64_t mx, uint64_t my) : _mx(mx), _my(my){
    _allc_noise_map();
    _allc_kernel();

    // 7 x 7 kernel
    // default gauss blur
    _kernel[0][0] = 1; _kernel[0][1] =  4; _kernel[0][2] =  7; _kernel[0][3] =  4; _kernel[0][4] = 1;
    _kernel[1][0] = 4; _kernel[1][1] = 16; _kernel[1][2] = 26; _kernel[1][3] = 16; _kernel[1][4] = 4;
    _kernel[2][0] = 7; _kernel[2][1] = 26; _kernel[2][2] = 41; _kernel[2][3] = 26; _kernel[2][4] = 7;
    _kernel[3][0] = 4; _kernel[3][1] = 16; _kernel[3][2] = 26; _kernel[3][3] = 16; _kernel[3][4] = 4;
    _kernel[4][0] = 1; _kernel[4][1] =  4; _kernel[4][2] =  7; _kernel[4][3] =  4; _kernel[4][4] = 1;

    gen(std::time(NULL));

}

void noise::update(uint64_t mx, uint64_t my){
    _free_noise_map();

    _mx = mx;
    _my = my;

    _allc_noise_map();
}

noise::~noise(){
    _free_noise_map();
    _free_kernel();
}

void noise::gen(uint64_t seed){
    std::srand(seed);

// #pragma omp parallel for schedule(dynamic, 1) collapse(2)
    for(uint64_t i = 0; i < _my; i++){
        for(uint64_t j = 0; j < _mx; j++){
            if(std::rand() % 2 == 0){
                if(std::rand() % 4 == 0){
                    _noise_map[i][j] = 0xFF;
                }
                else{
                    _noise_map[i][j] = (uint8_t)(std::rand()% 256);
                }
            }
            else{
                _noise_map[i][j] = 0;
            }
        }
    }
}

void noise::smooth(){
    uint8_t **rough = nullptr;

    // the sum of all elements of the kernel
    uint64_t kernel_sum = 0;
    uint64_t kernel_off = KERNEL_SIZE / 2;

#define round_x(x, off) round((x), (off), true)
#define round_y(y, off) round((y), (off), false)

    auto round = [&](uint64_t x, int64_t off, bool _x) -> uint64_t {
        if(off < 0){
            return (uint64_t)std::abs(off) > x ? (_x ? _mx : _my) + off : x + off;
        }
        else if(off > 0){
            return (x + off) % (_x ? _mx : _my);
        }
        return x;
    };

    auto convolute = [&](uint64_t y, uint64_t x) -> uint8_t{
        uint64_t sum = 0;

        for(uint64_t i = 0; i < KERNEL_SIZE; i++){
            for(uint64_t j = 0; j < KERNEL_SIZE; j++){
                sum += rough[round_y(y, -kernel_off + i)][round_x(x, -kernel_off + j)] * _kernel[i][j];
            }
        }

        return sum / kernel_sum > 255 ? 255 : (uint8_t)(sum / kernel_sum);
    };

    // get the sum of the elements in the kernel
    for(uint64_t i = 0; i < KERNEL_SIZE; i++){
        for(uint64_t j = 0; j < KERNEL_SIZE; j++){
            kernel_sum += _kernel[i][j];
        }
    }

    // allocate the rough matrix
    rough = new uint8_t*[_my];
    for(std::size_t i = 0; i < _my; i++){
        rough[i] = new uint8_t[_mx];
    }

    std::swap(_noise_map, rough);
    // do the convolution
// #pragma omp parallel for schedule(dynamic, 1) collapse(2)
    for(uint64_t i = 0; i < _my; i++){
        for(uint64_t j = 0; j < _mx; j++){
            _noise_map[i][j] = convolute(i, j);
        }
    }

    // free the rough matrix
    for(uint64_t i = 0; i < _my; i++){
         delete[] rough[i];
    }
    delete[] rough;
}

uint8_t * noise::operator [] (uint64_t index) const {
    return _noise_map[index];
}
