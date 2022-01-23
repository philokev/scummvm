/*
 * util.h
 *
 *  Created on: Dec 4, 2021
 *      Author: kevinlaurier
 */

#ifndef ENGINES_YAHTZEE_UTIL_H_
#define ENGINES_YAHTZEE_UTIL_H_

#include "yahtzee/controls.h"

namespace Parser {
struct Fileloc;
class Form;
class Image;
class Label;
}

namespace Image {
class BitmapDecoder;
}

int32 picaToPixels(int32);
uint32 msColorToRGB(const Graphics::PixelFormat&, uint32);
Graphics::Font* buildTTFFont(const Common::String&, double);
Graphics::Font* buildWinFont(const Common::String&, const Common::String&, double);
Image::BitmapDecoder* loadPicture(const Parser::Fileloc&);






#endif /* ENGINES_YAHTZEE_UTIL_H_ */
