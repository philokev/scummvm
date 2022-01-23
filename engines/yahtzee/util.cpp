/*
 * util.cpp
 *
 *  Created on: Dec 4, 2021
 *      Author: kevinlaurier
 */

#include "common/substream.h"
#include "common/system.h"
#include "common/file.h"

#include "graphics/font.h"
#include "graphics/fonts/ttf.h"
#include "graphics/fonts/winfont.h"

#include "image/bmp.h"

#include "yahtzee/controls.h"
#include "yahtzee/util.h"



int32 picaToPixels(int32 picas) {
	return picas / 15;
}

Image::BitmapDecoder* loadPicture(const Parser::Fileloc& picture) {
	Common::File file;

	//Image::BitmapDecoder image;
	auto image = new Image::BitmapDecoder();
	auto filename = picture.filename;

	if (!file.open(filename)) {
		error("sLoadPicture(): Error reading %s", filename.c_str());
	}
	if (picture.location > 0) {
		file.skip(picture.location);
	}
	auto file_size = file.readUint32LE();
	Common::SeekableSubReadStream mem(&file, file.pos(), file.pos() + file_size);

	if (!image->loadStream(mem)) {
		error("sLoadPicture(): Error loading image %s", filename.c_str());
	}
	file.close();
	return image;
}

uint32 msColorToRGB(const Graphics::PixelFormat& px, uint32 hexColor) {
	uint8 r = hexColor & 0xff;
	uint8 g = (hexColor & (0xff << 8)) >> 0x8;
	uint8 b = (hexColor & (0xff << 0x10)) >> 0x10;
	return px.RGBToColor(r, g, b);
}



Graphics::Font* buildTTFFont(const Common::String& fileName, double fontSize) {
	Common::File file;
	if (!file.open(fileName)) {
		error("sBuildTTFFont(): Error reading %s", fileName.c_str());
	}

	return Graphics::loadTTFFont(file, fontSize * 20 / 15);
}

Graphics::Font* buildWinFont(const Common::String& fileName, const Common::String& fontName, double fontSize) {
	Graphics::WinFont* font = new Graphics::WinFont();
	if (!font->loadFromFON(fileName, Graphics::WinFontDirEntry(fontName, fontSize))) {
		error("sBuildWinFont: Couldn't load %s", fileName.c_str());
	}
	return static_cast<Graphics::Font*>(font);
}


