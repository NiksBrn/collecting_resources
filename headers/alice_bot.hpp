#pragma once

#include "bot.hpp"

class AliceBot : public IBot {
public:
    void run(Game& game, std::ostream& out) override;

private:
    void explore(Game& game, std::ostream& out);
    void returnToStart(Game& game, std::ostream& out);

    int  pickNextDestination(const Game& game) const;
    int  pickBestResource(const Game& game, int room) const;
    bool pathThroughVisited(const Game& game, int from, int to,
                            std::vector<int>& outPath) const;
    bool shortestReturnPath(const Game& game, int from,
                            std::vector<int>& outPath) const;
};
