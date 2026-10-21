#include <iostream>
#include <string>
#include <vector>

struct Event {
    enum class Type { Begin, End };

    double ts;
    std::string caller;
    Type type;
};

struct Sample {
    double ts;
    std::vector<std::string> stack;
};

class Profiler {
public:
    std::vector<Event> process(const std::vector<Sample> &samples) {
        std::vector<Event> events;

        for (size_t i = 0; i < samples.size(); ++i) {
            const auto &current = samples[i];

            if (i == 0) {
                for (const auto &caller : current.stack) {
                    events.push_back({current.ts, caller, Event::Type::Begin});
                }
                continue;
            }

            const auto &previous = samples[i - 1];

            size_t j = 0;

            while (j < previous.stack.size() && j < current.stack.size() &&
                   previous.stack[j] == current.stack[j]) {
                ++j;
            }

            for (size_t k = previous.stack.size(); k > j; --k) {
                events.push_back(
                    {current.ts, previous.stack[k - 1], Event::Type::End});
            }

            for (size_t k = j; k < current.stack.size(); ++k) {
                events.push_back(
                    {current.ts, current.stack[k], Event::Type::Begin});
            }
        }

        return events;
    }
};

int main() {
    std::vector<Sample> samples = {{1.0, {"main"}},
                                   {2.0, {"main", "foo"}},
                                   {3.0, {"main", "foo", "bar"}},
                                   {4.0, {"main"}}};

    Profiler profiler;
    const auto events = profiler.process(samples);

    for (const auto &event : events) {
        std::cout << (event.type == Event::Type::Begin ? "Begin" : "End") << "("
                  << event.ts << ", " << event.caller << ")\n";
    }

    return 0;
}