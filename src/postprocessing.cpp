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

void visualize::calculate_bars(double *bars, size_t barcount, const double *buffer, size_t buffer_size) {
    std::fill_n(bars, barcount, 0);
    auto per_bar = buffer_size / barcount;
    for (size_t i = 0; i < per_bar * barcount; i++) {
        bars[i / per_bar] += buffer[i] / double(per_bar);
    }
}

std::pair<double *, std::unique_lock<std::mutex>> visualize::buffer::acquire() {
    return std::make_pair(data.get(), std::unique_lock(lock));
}

visualize::buffer::buffer(size_t size) : data_size(size), data(std::make_unique<double[]>(size)) {}
