//
// Created by canderson on 5/5/16.
//

#ifndef VERMILION_FRAMETILE_H
#define VERMILION_FRAMETILE_H


#include <cstdint>
#include "../../types/types.h"

namespace Vermilion {

    // struct of tile. Used for returned data.
    struct rTile {
        uint32_t mHeight;
        uint32_t mWidth;
        float4* mImage;
    };

    class frameTile {
        uint32_t mWidth;
        uint32_t mHeight;

        float4 *mImage;

    public:
        void setColor(uint32_t u, uint32_t v, float4 color);

        // Get a pointer to the grid data. DO NOT FREE THIS!!!
        rTile getTile();

        frameTile(uint32_t width, uint32_t height);

        ~frameTile();
    };
}

#endif //VERMILION_FRAMETILE_H
