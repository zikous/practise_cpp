#include <string>
#include <vector>

template<typename... Other>
struct Candle : Other... {
    long long open;
    long long high;
    long long low;
    long long close;
};

struct Label {
    std::string Text;
};

struct Color {
    int Alpha;
};

using AppCandle = Candle<Label, Color>;

struct Chart {
    std::vector<AppCandle> candles;

    std::string getLabel() const {
        if (candles.empty()) return "";
        return candles[0].Text;
    }

    int getAlpha() const {
        if (candles.empty()) return -1;
        return candles[0].Alpha;
    }
};

int main() {
    Chart chart;

    AppCandle candle;
    candle.open = 100;
    candle.high = 120;
    candle.low = 90;
    candle.close = 100;

    candle.Text = "test";
    candle.Alpha = 255;

    chart.candles.push_back(candle);
}