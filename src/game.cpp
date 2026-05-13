#include "game.hpp"

#include <algorithm>

const int   BASE_VALUE[4] = {7, 11, 23, 1};
const char* RES_NAME[4]   = {"iron", "gold", "gems", "exp"};

int resourceFromName(const std::string& s) {
    for (int i = 0; i < 4; ++i) {
        if (s == RES_NAME[i]) return i;
    }
    return R_NONE;
}

Game::Game(const Dungeon& d)
    : rooms_(d.rooms), // комнаты 
      collected_(d.rooms.size(), std::vector<bool>(4, false)), // собранные ресурсы в комнате
      visited_(d.rooms.size(), false), // посещенные комнаты
      n_(d.N), // общее количество комнат
      totalFood_(d.M), // количество еды
      target_(d.target), // таргет на игру
      current_(0), // текущая комната
      food_(d.M) // количество еды
{
    visited_[0] = true;
    for (int i = 0; i < 4; ++i) totalCollected_[i] = 0;
}
// удвоение ценности по таргету 
int Game::valueOf(int t) const {
    return BASE_VALUE[t] * (t == target_ ? 2 : 1);
}

// подсчет общей ценности собранных ресурсов
int Game::totalValue() const {
    int v = 0;
    for (int i = 0; i < 4; ++i) v += totalCollected_[i] * valueOf(i);
    return v;
}

// проверка, собран ли хотя бы один ресурс в комнате
bool Game::anyCollected(int r) const {
    for (int i = 0; i < 4; ++i) if (collected_[r][i]) return true;
    return false;
}

// вывод текущего состояния комнаты
void Game::outputState(std::ostream& out) const {
    // формат: state <номер_комнаты> <кол-во_ресурса_0> <кол-во_ресурса_1> <кол-во_ресурса_2> <кол-во_ресурса_3>
    out << "state " << current_;
    for (int i = 0; i < 4; ++i) {
        out << " ";
        if (collected_[current_][i]) out << "_";
        else out << rooms_[current_].counts[i];
    }
    out << "\n";
}

// перемещение в другую комнату
bool Game::move(int to, std::ostream& out) {
    const auto& nb = rooms_[current_].neighbors;
    if (std::find(nb.begin(), nb.end(), to) == nb.end()) return false;
    if (food_ <= 0) return false;

    --food_;
    current_ = to;
    visited_[to] = true;
    // формат: go <номер_комнаты>
    out << "go " << to << "\n";
    if (to != 0) outputState(out);
    return true;
}

// сбор ресурса в текущей комнате
bool Game::collect(int type, std::ostream& out) {
    if (type < 0 || type > 3) return false; // валидность типа ресурса
    if (collected_[current_][type]) return false; // ресурс уже собран
    if (rooms_[current_].counts[type] == 0) return false; // ресурс отсутствует в комнате

    bool isFirst = !anyCollected(current_); // если это не первый раз, тратим еду
    if (!isFirst) {
        if (food_ <= 0) return false;
        --food_;
    }

    out << "collect " << RES_NAME[type] << "\n";
    totalCollected_[type] += rooms_[current_].counts[type]; // обноление статистики 
    collected_[current_][type] = true;
    if (current_ != 0) outputState(out);
    return true;
}

// итоги игры
void Game::writeResult(std::ostream& out) const {
    // формат: result <кол-во_ресурса_0> <кол-во_ресурса_1> <кол-во_ресурса_2> <кол-во_ресурса_3> <общая_ценность>
    out << "result";
    for (int i = 0; i < 4; ++i) out << " " << totalCollected_[i];
    out << " " << totalValue() << "\n";
}
