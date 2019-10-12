#ifndef NOISE_H
#define NOISE_H

#include <cstdint>
#include <cstdlib>
#include <iostream>

class noise{
private:
    uint8_t **noise_map;

    uint64_t mx;
    uint64_t my;

public:
    // mx and myin this order
    noise(uint64_t, uint64_t);
    ~noise();
    // generates a random noise_map based on the seed
    void gen(uint64_t);
    // convolutes the kernel on the generated map, to smooth it
    void smooth(uint16_t **, uint64_t);
    // get the noise value at position
    uint8_t at(uint64_t, uint64_t);
};

// kernel for gaussian blur
// k[0][0]=1;  k[0][1]=6;   k[0][2]=15;  k[0][3]=20;  k[0][4]=15;  k[0][5]=6;   k[0][6]=1;
// k[1][0]=6;  k[1][1]=36;  k[1][2]=90;  k[1][3]=120; k[1][4]=90;  k[1][5]=36;  k[1][6]=6;
// k[2][0]=15; k[2][1]=90;  k[2][2]=225; k[2][3]=300; k[2][4]=225; k[2][5]=90;  k[2][6]=15;
// k[3][0]=20; k[3][1]=120; k[3][2]=300; k[3][3]=400; k[3][4]=300; k[3][5]=120; k[3][6]=20;
// k[4][0]=15; k[4][1]=90;  k[4][2]=225; k[4][3]=300; k[4][4]=225; k[4][5]=90;  k[4][6]=15;
// k[5][0]=6;  k[5][1]=36;  k[5][2]=90;  k[5][3]=120; k[5][4]=90;  k[5][5]=36;  k[5][6]=6;
// k[6][0]=1;  k[6][1]=6;   k[6][2]=15;  k[6][3]=20;  k[6][4]=15;  k[6][5]=6;   k[6][6]=1;


#endif
