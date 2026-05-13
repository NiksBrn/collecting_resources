#include "parser.hpp"

#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <algorithm>
#include <climits>
#include <stdexcept>

namespace {

// парсинг числа
bool parseUInt(const std::string& s, int& out) {
    if (s.empty()) return false;
    for (char c : s) if (c < '0' || c > '9') return false;
    long long v = 0;
    for (char c : s) {
        v = v * 10 + (c - '0');
        if (v > INT_MAX) return false;
    }
    out = static_cast<int>(v);
    return true;
}

// парсинг соседей
// ожидаемый формат: 1,2,3,4
bool parseNeighbors(const std::string& s, std::vector<int>& out) {
    out.clear();
    if (s.empty()) return false;
    std::string cur;
    for (char c : s) {
        if (c >= '0' && c <= '9') {
            cur += c;
        } else if (c == ',') {
            int v;
            if (cur.empty() || !parseUInt(cur, v)) return false;
            out.push_back(v);
            cur.clear();
        } else {
            return false;
        }
    }
    int v;
    if (cur.empty() || !parseUInt(cur, v)) return false;
    out.push_back(v);
    return true;
}

bool tokensExact(std::istringstream& iss, std::vector<std::string>& toks, int expected) {
    toks.clear();
    std::string t;
    while (iss >> t) toks.push_back(t);
    return static_cast<int>(toks.size()) == expected;
}

} 


ParseResult parseInput(const std::string& filename) {
    try {
        ParseResult result;
        result.ok = false;

        // открытие файла
        std::ifstream in(filename);
        if (!in) throw std::runtime_error("cannot open input file");

        std::string line;

        // чтение кол-ва комнат N
        if (!std::getline(in, line)) throw std::runtime_error("");

        int N;
        {
            std::istringstream iss(line);
            std::vector<std::string> toks;
            if (!tokensExact(iss, toks, 1)) throw std::runtime_error(line);
            if (!parseUInt(toks[0], N) || N < 1 || N > 255) throw std::runtime_error(line);
        }

        Dungeon d;
        d.N = N;
        d.rooms.assign(N + 1, Room());
        std::vector<std::string> roomLines(N + 1);
        std::set<int> seen;

        // чтение комнат (N+1 строка: комната 0 -- выход)
        for (int i = 0; i < N + 1; ++i) {
            if (!std::getline(in, line)) throw std::runtime_error("");
            std::istringstream iss(line);
            std::vector<std::string> toks;
            std::string t;
            while (iss >> t) toks.push_back(t);
            // формат: <id> <соседи> [iron gold gems exp]
            if (toks.size() != 2 && toks.size() != 6) throw std::runtime_error(line);

            int id;
            if (!parseUInt(toks[0], id) || id < 0 || id > N) throw std::runtime_error(line);
            if (seen.count(id)) throw std::runtime_error(line); // дубликат id

            std::vector<int> neighbors;
            if (!parseNeighbors(toks[1], neighbors)) throw std::runtime_error(line);
            for (int n : neighbors) {
                if (n < 0 || n > N || n == id) throw std::runtime_error(line);
            }
            std::sort(neighbors.begin(), neighbors.end());
            neighbors.erase(std::unique(neighbors.begin(), neighbors.end()), neighbors.end());

            // чтение кол-ва ресурсов в комнате
            int counts[4] = {0, 0, 0, 0};
            if (toks.size() == 6) {
                for (int k = 0; k < 4; ++k) {
                    if (!parseUInt(toks[2 + k], counts[k]) || counts[k] < 0 || counts[k] > 255)
                        throw std::runtime_error(line);
                }
            }

            seen.insert(id);
            roomLines[id] = line;
            Room& r = d.rooms[id];
            r.id = id;
            r.neighbors = std::move(neighbors);
            for (int k = 0; k < 4; ++k) r.counts[k] = counts[k];
        }

        // чтение M и таргета
        if (!std::getline(in, line)) throw std::runtime_error("");
        {
            std::istringstream iss(line);
            std::vector<std::string> toks;
            if (!tokensExact(iss, toks, 2)) throw std::runtime_error(line);
            int M;
            if (!parseUInt(toks[0], M) || M < 2 || M > 255) throw std::runtime_error(line);
            int target = resourceFromName(toks[1]);
            if (target == R_NONE) throw std::runtime_error(line);
            d.M = M;
            d.target = target;
        }

        // проверка, что все id от 0 до N присутствуют
        for (int id = 0; id <= N; ++id) {
            if (!seen.count(id)) {
                std::ostringstream oss;
                oss << N;
                throw std::runtime_error(oss.str());
            }
        }

        // достройка двунаправленных рёбер
        for (int id = 0; id <= N; ++id) {
            for (int n : d.rooms[id].neighbors) {
                auto& nb = d.rooms[n].neighbors;
                if (std::find(nb.begin(), nb.end(), id) == nb.end()) {
                    nb.push_back(id);
                }
            }
        }
        for (int id = 0; id <= N; ++id) {
            auto& nb = d.rooms[id].neighbors;
            std::sort(nb.begin(), nb.end());
            nb.erase(std::unique(nb.begin(), nb.end()), nb.end());
        }

        // проверка связности графа (обход в глубину от комнаты 0)
        {
            std::vector<bool> reached(N + 1, false);
            std::vector<int>  stack;
            stack.push_back(0);
            reached[0] = true;
            while (!stack.empty()) {
                int u = stack.back();
                stack.pop_back();
                for (int v : d.rooms[u].neighbors) {
                    if (!reached[v]) {
                        reached[v] = true;
                        stack.push_back(v);
                    }
                }
            }
            for (int id = 0; id <= N; ++id) {
                if (!reached[id]) throw std::runtime_error(roomLines[id]);
            }
        }

        result.ok = true;
        result.dungeon = std::move(d);
        return result;

    } catch (const std::exception& e) {
        ParseResult r;
        r.ok = false;
        r.errorLine = e.what();
        return r;
    } catch (...) {
        ParseResult r;
        r.ok = false;
        r.errorLine = "unknown error";
        return r;
    }
}
