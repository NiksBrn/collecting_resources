// класс  интерфейса бота, который будет играть в игру
#pragma once 

#include <ostream>
#include "game.hpp"

class IBot {
public:
    virtual ~IBot() = default;
    virtual void run(Game& game, std::ostream& out) = 0;
};
