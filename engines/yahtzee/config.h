#ifndef ENGINES_YAHTZEE_CONFIG_H
#define ENGINES_YAHTZEE_CONFIG_H

namespace Yahtzee {

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

const int REFRESH_DELAY_TIME = 10;

const Common::String FRIDAY_PREFIX = "FDAY";
const Common::String SATURDAY_PREFIX = "SDAY";
const Common::String YESTERDAY_PREFIX = "YDAY";

const Common::String RED_DWARF_1_PREFIX = "RDF1";
const Common::String RED_DWARF_2_PREFIX = "RDF2";
const Common::String RED_DWARF_3_PREFIX = "RDF3";
const Common::String RED_DWARF_4_PREFIX = "RDF4";
const Common::String RED_DWARF_5_PREFIX = "RDF5";

const Common::String DING_SOUND_FILENAME = "DING.WAV";
const Common::String YAHTZEE_DATA_FILENAME = "yahtzee.dat";

const Common::String TEST_BITMAP_FILENAME = "test.bmp";

const Common::String INVENTORY_FORM = "Form6";

const Common::String MS_SANS_SERIF_FONT_NAME = "MS Sans Serif";
const Common::String MS_SERIF_FONT_NAME = "MS Serif";

const Common::String DEFAULT_FONT_NAME = MS_SANS_SERIF_FONT_NAME;

const Common::String MS_SERIF_BOLD_FONT_FILE_NAME = "SERIFEB.FON";
const Common::String MS_SANS_SERIF_FONT_FILE_NAME = "SSERIFE.FON";
const Common::String MS_SANS_SERIF_BOLD_FONT_FILE_NAME = "SSERIFEB.FON";
const Common::String MS_TIMES_NEW_ROMAN_BOLD_ITALIC_FONT_FILE_NAME = "Times_New_Roman_Bold_Italic.ttf";

/*
const Common::String MS_SERIF_NAME = "MS Serif";
const Common::String MS_SANS_SERIF_NAME = "MS Sans Serif";
const Common::String MS_TIMES_NEW_ROMAN_NAME = "Times New Roman";
const Common::String DEFAULT_FONT_NAME = MS_SERIF_NAME;

const Common::String MS_SERIF_BOLD = "SERIFEB.FON";
const Common::String MS_SANS_SERIF = "SSERIFE.FON";
const Common::String MS_SANS_SERIF_BOLD = "SSERIFEB.FON";
const Common::String MS_TIMES_NEW_ROMAN_BOLD_ITALIC = "Times_New_Roman_Bold_Italic.ttf";

const double DEFAULT_FONT_SIZE = 12;
const double MS_SERIF_FONT_SIZE = 8.25;
const double MS_SANS_SERIF_FONT_SIZE = 11;
const double MS_TIMES_NEW_ROMAN_BOLD_FONT_SIZE = 24;
*/
}

#endif
