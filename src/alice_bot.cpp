#include "alice_bot.hpp"

#include <algorithm>
#include <climits>
#include <queue>
#include <vector>
#include <iostream>

// запуск бота: исследование -> возврат -> итоги
void AliceBot::run(Game& game, std::ostream& out) {
    explore(game, out);
    returnToStart(game, out);
    if (game.currentRoom() == 0) {
        game.writeResult(out);
    }
}

// выбор наиболее ценного ресурса в комнате (ещё не собранного)
int AliceBot::pickBestResource(const Game& game, int room) const {
    int best = R_NONE;
    int bestVal = -1;
    for (int t = 0; t < 4; ++t) {
        if (game.isCollected(room, t) || game.count(room, t) == 0) continue;
        int v =  game.count(room, t) * game.valueOf(t);
        if (v > bestVal) { bestVal = v; best = t; }
    }
    return best;
}

// выбор следующей непосещённой комнаты для исследования
// сначала проверяем прямых соседей, затем BFS по посещённым
int AliceBot::pickNextDestination(const Game& game) const {
    int from = game.currentRoom();

    // ближайший непосещённый сосед
    std::vector<int> nbs = game.neighbors(from);
    std::sort(nbs.begin(), nbs.end());
    for (int n : nbs) {
        if (!game.isVisited(n)) return n;
    }

    // BFS по посещённым комнатам до ближайшей непосещённой
    int N = game.N();
    std::vector<int> dist(N + 1, INT_MAX);
    dist[from] = 0;
    std::queue<int> q;
    q.push(from);
    int minDist = INT_MAX;
    int target = -1;

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        if (dist[u] >= minDist) continue;
        for (int v : game.neighbors(u)) {
            if (game.isVisited(v)) {
                if (dist[v] == INT_MAX) {
                    dist[v] = dist[u] + 1;
                    q.push(v);
                }
            } else {
                int d = dist[u] + 1;
                if (d < minDist) { minDist = d; target = v; }
                else if (d == minDist && v < target) { target = v; }
            }
        }
    }
    return target;
}

// BFS-путь от from до to только через посещённые комнаты
bool AliceBot::pathThroughVisited(const Game& game, int from, int to,
                                  std::vector<int>& outPath) const {
    outPath.clear();
    if (from == to) return true;

    int N = game.N();
    std::vector<int> dist(N + 1, INT_MAX);
    dist[to] = 0;
    std::queue<int> q;
    q.push(to);
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        for (int v : game.neighbors(u)) {
            if (v != from && v != to && !game.isVisited(v)) continue;
            if (dist[v] == INT_MAX) {
                dist[v] = dist[u] + 1;
                q.push(v);
            }
        }
    }
    if (dist[from] == INT_MAX) return false;

    // восстановление пути по расстояниям
    int u = from;
    while (u != to) {
        int next = -1;
        for (int v : game.neighbors(u)) {
            if (v != to && !game.isVisited(v)) continue;
            if (dist[v] == dist[u] - 1) {
                if (next == -1 || v < next) next = v;
            }
        }
        if (next == -1) return false;
        outPath.push_back(next);
        u = next;
    }
    return true;
}

// кратчайший путь обратно в комнату 0 только через посещённые комнаты
bool AliceBot::shortestReturnPath(const Game& game, int from,
                                  std::vector<int>& outPath) const {
    outPath.clear();
    if (from == 0) return true;

    int N = game.N();
    std::vector<int> dist(N + 1, INT_MAX);
    dist[0] = 0;
    std::queue<int> q;
    q.push(0);
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        for (int v : game.neighbors(u)) {
            if (!game.isVisited(v)) continue;
            if (dist[v] == INT_MAX) {
                dist[v] = dist[u] + 1;
                q.push(v);
            }
        }
    }
    if (dist[from] == INT_MAX) return false;

    // восстановление пути по расстояниям
    int u = from;
    while (u != 0) {
        int next = -1;
        for (int v : game.neighbors(u)) {
            if (!game.isVisited(v)) continue;
            if (dist[v] == dist[u] - 1) {
                if (next == -1 || v < next) next = v;
            }
        }
        if (next == -1) return false;
        outPath.push_back(next);
        u = next;
    }
    return true;
}

// фаза исследования: тратим половину еды на обход непосещённых комнат
void AliceBot::explore(Game& game, std::ostream& out) {
    int budget = game.M() / 2;
    int spent  = 0;

    while (spent < budget) {
        int dest = pickNextDestination(game);
        if (dest == -1) break;

        std::vector<int> path;
        if (!pathThroughVisited(game, game.currentRoom(), dest, path)) break;

        for (int step : path) {
            if (spent >= budget) break;
            if (!game.move(step, out)) return;
            ++spent;
            // собираем лучший ресурс при достижении цели
            if (step == dest) {
                int best = pickBestResource(game, step);
                if (best != R_NONE) game.collect(best, out);
            }
        }
    }
}

// фаза возврата: идём к выходу, по пути собираем ресурсы на оставшуюю еду
void AliceBot::returnToStart(Game& game, std::ostream& out) {
    std::vector<int> path;
    if (!shortestReturnPath(game, game.currentRoom(), path)) return;
    // extra -- еда сверх необходимой для возврата
    int needed = static_cast<int>(path.size());
    int extra  = game.food() - needed;
    if (extra < 0) extra = 0;

    // сбор всех доступных ресурсов в текущей комнате
    auto collectExtras = [&](int room) {
        while (true) {
            int best = pickBestResource(game, room);
            if (best == R_NONE) break;
            bool isFirst = !game.anyCollected(room);
            if (!isFirst) {
                if (extra <= 0) break;
                --extra;
            }
            if (!game.collect(best, out)) break;
        }
    };

    collectExtras(game.currentRoom());

    for (int step : path) {
        if (!game.move(step, out)) return;
        if (game.currentRoom() == 0) break;
        collectExtras(game.currentRoom());
    }
}
