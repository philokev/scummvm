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

#include "base/plugins.h"

#include "engines/advancedDetector.h"

#include "yahtzee/yahtzee.h"

namespace Yahtzee {
const char *YahtzeeGame::getGameId() const { return _gameDescription->gameId; }
Common::Platform YahtzeeGame::getPlatform() const { return _gameDescription->platform; }
} // End of namespace Yahtzee

class YahtzeeMetaEngine : public AdvancedMetaEngine {
	const char *getName() const override {
		return "yahtzee";
	}

    Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	bool hasFeature(MetaEngineFeature f) const override;
};

Common::Error YahtzeeMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc) {
		*engine = new Yahtzee::YahtzeeGame(syst, desc);
		return Common::kNoError;
	}

	return Common::kUnsupportedGameidError;
}

bool YahtzeeMetaEngine::hasFeature(MetaEngineFeature f) const {
	return false;
}

#if PLUGIN_ENABLED_DYNAMIC(YAHTZEE)
	REGISTER_PLUGIN_DYNAMIC(YAHTZEE, PLUGIN_TYPE_ENGINE, YahtzeeMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(YAHTZEE, PLUGIN_TYPE_ENGINE, YahtzeeMetaEngine);
#endif
