//
// Created by canderson on 5/5/16.
//

#ifndef VERMILION_FRAMETILE_H
#define VERMILION_FRAMETILE_H


#include <cstdint>
#include "../../types/types.h"

namespace Vermilion
{
	class frameTile
	{
		uint32_t mWidth;
		uint32_t mHeight;

		uint32_t renderOffsetX;
		uint32_t renderOffsetY;

		float4 *mImage;

	public:
		// Dummy Testing function
		void setColor(uint32_t u, uint32_t v, float4 color);

		// Get a pointer to the grid data. DO NOT FREE THIS!!!
		float4* getTile() const;

		frameTile(uint32_t width, uint32_t height, uint32_t oX, uint32_t oY);

		~frameTile();
	};
}

#endif //VERMILION_FRAMETILE_H
