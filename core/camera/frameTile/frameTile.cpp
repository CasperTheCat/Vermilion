//
// Created by canderson on 5/5/16.
//

#include "frameTile.h"

/*
 * Constructor
 */
Vermilion::frameTile::frameTile(uint32_t width, uint32_t height, uint32_t oX, uint32_t oY) : mWidth(width), mHeight(height), renderOffsetX(oX), renderOffsetY(oY)
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

Vermilion::float4 *Vermilion::frameTile::getTile() const
{
	return mImage;
}

