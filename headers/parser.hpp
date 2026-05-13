#pragma once

#include <string>
#include "game.hpp"

struct ParseResult {
    bool ok;
    std::string errorLine;
    Dungeon dungeon;
};

ParseResult parseInput(const std::string& filename);
