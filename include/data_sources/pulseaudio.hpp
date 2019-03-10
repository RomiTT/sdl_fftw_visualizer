/**
 * Copyright 2019 w1d3m0d3
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef PULSEAUDIO_HPP
#define PULSEAUDIO_HPP

#include "../data_source.hpp"

#include <pulse/simple.h>

namespace visualize {
    struct pulseaudio_source : public data_source {
        pulseaudio_source(size_t buffer_len);
        ~pulseaudio_source();
        // disable copy
        pulseaudio_source(const pulseaudio_source&) = delete;
        pulseaudio_source &operator =(const pulseaudio_source &) = delete;
    private:
        bool do_grab_audio(double *output, size_t len) override;
        pa_simple *simple = nullptr;
        std::unique_ptr<int16_t[]> pulse_buffer;
    };
}

#endif // PULSEAUDIO_HPP
