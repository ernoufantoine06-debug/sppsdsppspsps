#include <cmath>
#include <cstdint>

extern "C" void precompute_outcomes(
    int n,
    const double* open,
    const double* high,
    const double* low,
    const double* close,
    const double* atr_prev,
    int pcount,
    const int* stop_kind,       // 0=fixed pips, 1=ATR14
    const double* stop_value,   // pips if fixed, multiplier if ATR
    const double* rr,
    const int* max_bars,        // 0=unlimited; entry bar counts as 1
    std::int32_t* exit_index,   // [policy, direction, entry]
    double* gross_r             // [policy, direction, entry]
) {
    const double pip = 0.0001;
    for (int p = 0; p < pcount; ++p) {
        for (int dir = 0; dir < 2; ++dir) {
            const std::size_t base = (static_cast<std::size_t>(p) * 2 + dir) * n;
            for (int entry = 0; entry < n; ++entry) {
                exit_index[base + entry] = -1;
                gross_r[base + entry] = 0.0;
                double stop_dist = stop_kind[p] == 0
                    ? stop_value[p] * pip
                    : (entry > 0 ? atr_prev[entry] * stop_value[p] : 0.0);
                if (!(stop_dist > 0.0) || !std::isfinite(stop_dist) || !std::isfinite(open[entry])) continue;
                const double target_dist = stop_dist * rr[p];
                const double sl = dir == 0 ? open[entry] - stop_dist : open[entry] + stop_dist;
                const double tp = dir == 0 ? open[entry] + target_dist : open[entry] - target_dist;
                int last = n - 1;
                if (max_bars[p] > 0 && entry + max_bars[p] - 1 < last)
                    last = entry + max_bars[p] - 1;
                bool done = false;
                for (int t = entry; t <= last; ++t) {
                    bool hit_sl = dir == 0 ? low[t] <= sl : high[t] >= sl;
                    bool hit_tp = dir == 0 ? high[t] >= tp : low[t] <= tp;
                    double exit_price;
                    if (hit_sl) {
                        // If both levels are inside a candle, use the pessimistic SL-first path.
                        exit_price = sl;
                        exit_index[base + entry] = t;
                        gross_r[base + entry] = -1.0;
                        done = true;
                        break;
                    }
                    if (hit_tp) {
                        exit_price = tp;
                        exit_index[base + entry] = t;
                        gross_r[base + entry] = rr[p];
                        done = true;
                        break;
                    }
                    if (max_bars[p] > 0 && t == last && last == entry + max_bars[p] - 1) {
                        exit_price = close[t];
                        exit_index[base + entry] = t;
                        gross_r[base + entry] = dir == 0
                            ? (exit_price - open[entry]) / stop_dist
                            : (open[entry] - exit_price) / stop_dist;
                        done = true;
                        break;
                    }
                }
                if (!done) {
                    int t = n - 1;
                    exit_index[base + entry] = t;
                    gross_r[base + entry] = dir == 0
                        ? (close[t] - open[entry]) / stop_dist
                        : (open[entry] - close[t]) / stop_dist;
                }
            }
        }
    }
}
