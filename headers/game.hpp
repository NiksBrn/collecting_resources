#pragma once

#include <vector>
#include <string>
#include <ostream>

// типы ресурсов
enum ResurceType{
    R_IRON = 0,
    R_GOLD = 1,
    R_GEMS = 2,
    R_EXP = 3,
    R_NONE = -1
};

extern const int   BASE_VALUE[4]; // базовые ценности ресурсов
extern const char* RES_NAME[4]; // имена ресурсов

int  resourceFromName(const std::string& s);

struct Room {
    int id = -1;
    std::vector<int> neighbors;
    int counts[4] = {0, 0, 0, 0};
};

struct Dungeon {
    int N = 0; // кол-во комнат
    int M = 0; // кол-во ресурсов
    int target = R_NONE; // таргет игры
    std::vector<Room> rooms; // комнаты
};

class Game {
public:
    Game(const Dungeon& d);
    
    int currentRoom() const { return current_; } // номер текущей комнаты
    int food() const { return food_; } // текущее кол-во еды
    int target() const { return target_; } // таргет игры
    int M() const { return totalFood_; } // кол-во еды в игре
    int N() const { return n_; } // кол-во комнат

    const std::vector<int>& neighbors(int r) const { return rooms_[r].neighbors; } // соседи комнаты
    int  count(int r, int t) const { return rooms_[r].counts[t]; } // кол-во ресурса в комнате
    bool isCollected(int r, int t) const { return collected_[r][t]; } // информация о ресурсе (собран или нет)
    bool isVisited(int r) const { return visited_[r]; } // информация о посещение комнаты
    bool anyCollected(int r) const; // был ли собран ресурс  

    int valueOf(int t) const; // подчет ценности ресурса (удвоение по таргету)
    int collected(int t) const { return totalCollected_[t]; } // кол-во собранного ресурса
    int totalValue() const; // итоговая ценность ресурсов

    bool dead() const { return food_ <= 0 && current_ != 0; } // проверка на смерть от нехвати еды

    bool move(int to, std::ostream& out); // перемещение в другую комнату
    bool collect(int type, std::ostream& out); // сборка ресурса
    void writeResult(std::ostream& out) const; // итоги игры

private:
    void outputState(std::ostream& out) const;

    std::vector<Room> rooms_;
    std::vector<std::vector<bool>> collected_;
    std::vector<bool> visited_;
    int totalCollected_[4];
    int n_;
    int totalFood_;
    int target_;
    int current_;
    int food_;
};