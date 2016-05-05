//
// Created by canderson on 5/5/16.
//

#ifndef VERMILION_FRAMETILE_H
#define VERMILION_FRAMETILE_H


#include <cstdint>

class frameTile {
    uint32_t mWidth;
    uint32_t mHeight;



public:
    frameTile(uint32_t mWidth, uint32_t mHeight);
    ~frameTile();
};


#endif //VERMILION_FRAMETILE_H
