#include "include/noise.hpp"

#define KERNEL_SIZE 7

noise::noise(uint64_t mx, uint64_t my) : _mx(mx), _my(my){
    _noise_map = new uint8_t*[_my];
    for(uint64_t i = 0; i < _my; i++){
        _noise_map[i] = new uint8_t[_mx];
    }

    _kernel = new uint16_t*[KERNEL_SIZE];
    for(std::size_t i = 0; i < 7; i++){
        _kernel[i] = new uint16_t[KERNEL_SIZE];
    }

    // // 7 x 7 kernel
    // default gausina blur
    _kernel[0][0]=1;  _kernel[0][1]=6;   _kernel[0][2]=15;  _kernel[0][3]=20;  _kernel[0][4]=15;  _kernel[0][5]=6;   _kernel[0][6]=1;
    _kernel[1][0]=6;  _kernel[1][1]=36;  _kernel[1][2]=90;  _kernel[1][3]=120; _kernel[1][4]=90;  _kernel[1][5]=36;  _kernel[1][6]=6;
    _kernel[2][0]=15; _kernel[2][1]=90;  _kernel[2][2]=225; _kernel[2][3]=300; _kernel[2][4]=225; _kernel[2][5]=90;  _kernel[2][6]=15;
    _kernel[3][0]=20; _kernel[3][1]=120; _kernel[3][2]=300; _kernel[3][3]=400; _kernel[3][4]=300; _kernel[3][5]=120; _kernel[3][6]=20;
    _kernel[4][0]=15; _kernel[4][1]=90;  _kernel[4][2]=225; _kernel[4][3]=300; _kernel[4][4]=225; _kernel[4][5]=90;  _kernel[4][6]=15;
    _kernel[5][0]=6;  _kernel[5][1]=36;  _kernel[5][2]=90;  _kernel[5][3]=120; _kernel[5][4]=90;  _kernel[5][5]=36;  _kernel[5][6]=6;
    _kernel[6][0]=1;  _kernel[6][1]=6;   _kernel[6][2]=15;  _kernel[6][3]=20;  _kernel[6][4]=15;  _kernel[6][5]=6;   _kernel[6][6]=1;

}

noise::~noise(){
    for(uint64_t i = 0; i < _my; i++){
        delete [] _noise_map[i];
    }
    delete [] _noise_map;

    for(uint64_t i = 0; i < KERNEL_SIZE; i++){
        delete [] _kernel[i];
    }
    delete [] _kernel;
}

void noise::gen(uint64_t seed){
    std::srand(seed);

// #pragma omp parallel for schedule(dynamic, 1) collapse(2)
    for(uint64_t i = 0; i < _my; i++){
        for(uint64_t j = 0; j < _mx; j++){
            if(std::rand() % 3 == 0){
                _noise_map[i][j] = (uint8_t)(std::rand()% 256);
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
