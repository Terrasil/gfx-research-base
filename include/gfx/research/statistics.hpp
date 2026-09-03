#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <span>
#include <vector>

namespace gfx::research {
    struct Statistics {
        std::size_t count = 0;
        double minimum = 0.0;
        double maximum = 0.0;
        double mean = 0.0;
        double median = 0.0;
        double standard_deviation = 0.0;
        double p95 = 0.0;
    };

    inline double percentile(std::span<const double> values, double p) {
        if (values.empty()) return 0.0;
        std::vector<double> sorted(values.begin(), values.end());
        std::sort(sorted.begin(), sorted.end());
        p = std::clamp(p, 0.0, 1.0);
        const double position = p * static_cast<double>(sorted.size() - 1);
        const std::size_t lower = static_cast<std::size_t>(std::floor(position));
        const std::size_t upper = static_cast<std::size_t>(std::ceil(position));
        if (lower == upper) return sorted[lower];
        const double fraction = position - static_cast<double>(lower);
        return sorted[lower] * (1.0 - fraction) + sorted[upper] * fraction;
    }

    inline Statistics summarize(std::span<const double> values) {
        Statistics result;
        if (values.empty()) return result;

        result.count = values.size();
        result.minimum = std::numeric_limits<double>::infinity();
        result.maximum = -std::numeric_limits<double>::infinity();
        double sum = 0.0;
        for (const double value : values) {
            result.minimum = std::min(result.minimum, value);
            result.maximum = std::max(result.maximum, value);
            sum += value;
        }
        result.mean = sum / static_cast<double>(values.size());

        if (values.size() > 1) {
            double squared_error = 0.0;
            for (const double value : values) {
                const double delta = value - result.mean;
                squared_error += delta * delta;
            }
            result.standard_deviation = std::sqrt(squared_error / static_cast<double>(values.size() - 1));
        }

        result.median = percentile(values, 0.5);
        result.p95 = percentile(values, 0.95);
        return result;
    }
}
