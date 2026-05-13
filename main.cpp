#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "alice_bot.hpp"
#include "game.hpp"
#include "parser.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << (argc ? argv[0] : "task") << " <input_file>\n";
        return 1;
    }

    std::ofstream out("result.txt");
    if (!out) {
        std::cerr << "Cannot open result.txt for writing\n";
        return 1;
    }

    ParseResult parsed = parseInput(argv[1]);
    if (!parsed.ok) {
        out << parsed.errorLine << "\n";
        return 0;
    }

    Game game(parsed.dungeon);

    std::unique_ptr<IBot> bot = std::make_unique<AliceBot>();
    bot->run(game, out);

    return 0;
}
