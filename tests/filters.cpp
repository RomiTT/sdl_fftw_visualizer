/**
 * Copyright 2019 w1d3m0d3
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <algorithm>
#include <filters/clip_filter.hpp>
#include <filters/peek_filter.hpp>
#include <filters/sagc_filter.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace {
    const double test_buffer[] = { 1.1, 1.0, 0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2 };
    void populate_buffer(double data[std::size(test_buffer)]) {
        std::copy(std::begin(test_buffer), std::end(test_buffer), data);
    }

    template<size_t BufSize>
    bool compare_buffers(const double (&buffer1)[BufSize], const double (&buffer2)[BufSize]) {
        return std::equal(std::cbegin(buffer1), std::cend(buffer1), std::cbegin(buffer2));
    }
} // namespace

TEST(filter_tests, clip_filter) {
    double buffer[] = { 1.1, 1.0, 0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2 };
    double buffer_expected[] = { 1.0, 1.0, 0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2 };
    visualize::clip_filter filter(std::size(buffer));
    filter.apply(buffer);
    ASSERT_TRUE(compare_buffers(buffer, buffer_expected));
}

TEST(filter_tests, sagc_filter) {
    double buffer[std::size(test_buffer)];
    double expected_buffer[std::size(buffer)];
    populate_buffer(buffer);
    std::transform(std::cbegin(buffer), std::cend(buffer), std::begin(expected_buffer),
                   [](auto &a) { return a * 0.85; });

    visualize::sagc_filter filter(std::size(buffer));
    filter.apply(buffer);
    filter.apply(buffer);
    ASSERT_TRUE(compare_buffers(buffer, expected_buffer)) << "Gain raising";

    std::fill(std::begin(buffer), std::end(buffer), 0.2);
    filter = visualize::sagc_filter(std::size(buffer));
    std::transform(std::cbegin(buffer), std::cend(buffer), std::begin(expected_buffer),
                   [](auto &a) { return a * 1.1; });
    filter.apply(buffer);
    filter.apply(buffer);
    ASSERT_TRUE(compare_buffers(buffer, expected_buffer)) << "Gain lowering";
}

template<size_t Size>
struct step {
    const double input_data[Size];
    const double expected_data[Size];
    const std::string step_name;
};

TEST(filter_tests, peek_filter) {
    double buffer[3];
    std::vector<step<std::size(buffer)>> steps {
        // clang-format off
        { { 1, 1, 1 }, {   1,   1,   1 }, "Initial setup" },
        { { 0, 0, 0 }, { 0.4, 0.4, 0.4 }, "1.0 -> 0.0"    },
        { { 0, 0, 0 }, { 0.3, 0.3, 0.3 }, "hold 0 pt 1"   },
        { { 0, 0, 0 }, { 0.2, 0.2, 0.2 }, "hold 0 pt 2"   },
        { { 0, 0, 0 }, { 0.1, 0.1, 0.1 }, "hold 0 pt 3"   },
        { { 0, 0, 0 }, {   0,   0,   0 }, "hold 0 pt 4"   }
        // clang-format on
    };
    visualize::peek_filter filter(std::size(buffer), 200);
    for (auto &istep : steps) {
        std::copy(std::cbegin(istep.input_data), std::cend(istep.input_data), std::begin(buffer));
        filter.apply(buffer);
        ASSERT_TRUE(std::equal(std::cbegin(buffer), std::cend(buffer), std::begin(istep.expected_data),
                               [](auto &a, auto &b) { return std::abs(a - b) < 0.01; }))
            << "Step " << istep.step_name;
    }
}
