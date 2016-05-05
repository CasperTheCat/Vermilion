//
// Created by canderson on 5/5/16.
//

#include "frameTile.h"

/*
 * Constructor
 */
Vermilion::frameTile::frameTile(uint32_t width, uint32_t height) : mWidth(width), mHeight(height)
{
    // Allocate the image array based on the image size
    this->mImage = new float4[width * height];
}

/*
 * Destructor
 */
Vermilion::frameTile::~frameTile()
{
    if (mImage) delete mImage; // Delete on destruct
}

/*
 * setColor of a pixel in the pixel grid
 */
void Vermilion::frameTile::setColor(uint32_t u, uint32_t v, float4 color)
{
    this->mImage[v * mWidth + u] = color;
}

/*
 * get the tile back from rendering
 */

rTile Vermilion::frameTile::getTile()
{
    rTile ret;
    ret.mWidth = mWidth;
    ret.mHeight = mHeight;
    ret.mImage = mImage;
    return ret;
}

