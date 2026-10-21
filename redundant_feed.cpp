#include <generator>
#include <iostream>
#include <map>
#include <vector>

struct Packet {
    char feed;
    long time;
    int seq;
};

using Feed = std::generator<Packet>;

Feed make_feed(std::vector<Packet> packets) {
    for (const Packet& p : packets)
        co_yield p;
}

std::vector<Packet> arbitrate(Feed a, Feed b) {
    std::map<int, Packet> map;
    std::vector<Packet> res;

    auto keepFastest = [&map](const Packet& p) {
        auto [it, inserted] = map.try_emplace(p.seq, p);

        if (!inserted && it->second.time > p.time ) {
            it->second = p;
        }
    };

    for (const auto& p: a) { keepFastest(p); };
    for (const auto& p: b) { keepFastest(p); };

    for (const auto& [_, p] : map) {
        res.push_back(p);
    }

    return res;
}

int main() {
    auto a = make_feed({
        {'A', 10, 1},
        {'A', 20, 2},
        {'A', 30, 3},
        {'A', 50, 5},
        {'A', 12, 5},
    });

    auto b = make_feed({
        {'B', 5, 4},
        {'B', 8, 1},
        {'B', 20, 2},
    });

    for (const Packet& p : arbitrate(std::move(a), std::move(b)))
        std::cout << p.feed << " seq=" << p.seq << " t=" << p.time << '\n';
}