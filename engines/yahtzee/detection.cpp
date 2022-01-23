/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "yahtzee/yahtzee.h"

#include "base/plugins.h"

#include "engines/advancedDetector.h"

static const PlainGameDescriptor yahtzeeGames[] = {
	{"yahtzee", "Death to Arthur Yahtzee"},
	{0, 0}
};

namespace Yahtzee {

static const ADGameDescription gameDescriptions[] = {
	// Yahtzee PC version
	{
		"yahtzee",
		0,
		AD_ENTRY1s("FRIDAY.EXE", 0, 9638954),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Yahtzee

class YahtzeeMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	YahtzeeMetaEngineDetection() : AdvancedMetaEngineDetection(Yahtzee::gameDescriptions, sizeof(ADGameDescription), yahtzeeGames) {
	}

	const char *getEngineId() const override {
		return "yahtzee";
	}

	const char *getName() const override {
		return "Death to Arthur Yahtzee";
	}

	const char *getOriginalCopyright() const override {
		return "Death to Arthur Yahtzee (C) 1999 Ben Croshaw";
	}
};


REGISTER_PLUGIN_STATIC(YAHTZEE_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, YahtzeeMetaEngineDetection);
