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
#include "data_sources/pulseaudio.hpp"
#include <iostream>
#include <pulse/error.h>

namespace {
    const pa_sample_spec spec { PA_SAMPLE_S16NE, 44100, 2 };
}

visualize::pulseaudio_source::pulseaudio_source(size_t buffer_len) :
    data_source(buffer_len),
    pulse_buffer(std::make_unique<int16_t[]>(buffer_len)) {
    int err;
    simple = pa_simple_new(nullptr, "visualizer", PA_STREAM_RECORD, nullptr, "record", &spec, nullptr, nullptr, &err);
    if (!simple) {
        std::cerr << "Pulse connection error: " << pa_strerror(err) << std::endl;
    }
}

visualize::pulseaudio_source::~pulseaudio_source() {
    if (simple) {
        pa_simple_free(simple);
    }
}

bool visualize::pulseaudio_source::do_grab_audio(double *output, size_t len) {
    if (!simple) {
        return false;
    }
    int err;
    if (pa_simple_read(simple, pulse_buffer.get(), len, &err) < 0) {
        std::cerr << "Pulse read error: " << err << std::endl;
    }
    for (size_t i = 0; i < len; i++) {
        output[i] = static_cast<double>(pulse_buffer[i]) / std::numeric_limits<int16_t>::max();
    }
    return true;
}
