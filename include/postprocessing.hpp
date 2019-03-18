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
#ifndef POSTPROCESSING_HPP
#define POSTPROCESSING_HPP

#include "data_source.hpp"
#include "filter.hpp"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

namespace visualize {
    struct buffer {
        explicit buffer(size_t size);
        ~buffer() = default;
        std::pair<double *, std::unique_lock<std::mutex>> acquire();

        buffer(const buffer &other) = delete;
        buffer &operator=(const buffer &other) = delete;
        buffer(buffer &&) = delete;
        buffer &operator=(buffer &&) = delete;

        const size_t data_size;

    private:
        std::unique_ptr<double[]> data;
        std::mutex lock;
    };
    void calculate_bars(double *bars, size_t barcount, const double *buffer, size_t buffer_size);
} // namespace visualize

#endif // POSTPROCESSING_HPP
