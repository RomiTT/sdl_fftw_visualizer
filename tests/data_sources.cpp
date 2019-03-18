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
#include "data_source.hpp"
#include <cmath>
#include <gtest/gtest.h>

struct null_source : public visualize::data_source {
    null_source(size_t size) : visualize::data_source(size), size(size) {}
    ~null_source() override = default;

private:
    bool do_grab_audio(double *buf) override {
        if (next_null) {
            return false;
        }
        next_null = true;
        std::fill_n(buf, size, 1.0);
        return true;
    }

    bool next_null = false;
    size_t size;
};

double data[314];
double data_expected[std::size(data)];
TEST(data_source, null_source) {
    for (size_t i = 0; i < std::size(data); i++) {
        data_expected[i] = (1 + cos(i / std::size(data) * M_PI)) / 2;
    }
    null_source src(std::size(data));
    ASSERT_TRUE(src.grab_audio(data));
    std::equal(std::begin(data), std::end(data), std::begin(data_expected));
    ASSERT_FALSE(src.grab_audio(data));
}
