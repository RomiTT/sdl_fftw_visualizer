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
#include "postprocessing.hpp"
#include <algorithm>
#include <gtest/gtest.h>

TEST(postprocessing, calculate_bars) {
    const double input[] = { 1, 1, 1, 0, 0.5, 0, 0.5, 1, 0.2, 1 };
    const double expected_out[] = { 1, 0.5, 0.25, 0.75, 0.6 };
    double output[std::size(expected_out)] = { 0 };
    visualize::calculate_bars(output, std::size(output), input, std::size(input));
    ASSERT_TRUE(std::equal(std::cbegin(expected_out), std::cend(expected_out), std::cbegin(output)));
}

TEST(postprocessing, buffer) {
    visualize::buffer buf(10);
    {
        auto [ptr, _] = buf.acquire();
        std::fill_n(ptr, buf.data_size, 1.0);
    }
    {
        auto [ptr, _] = buf.acquire();
        ASSERT_TRUE(std::all_of(ptr, &ptr[buf.data_size], [](auto &a) { return a == 1.0; }));
    }
}
