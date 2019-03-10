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
#ifndef AUDIO_SOURCE_HPP
#define AUDIO_SOURCE_HPP

#include <memory>
#include <stddef.h>
#include <stdint.h>

namespace visualize {
    //! Abstraction for data collection from various sources (e.g. sound servers)
    struct data_source {
        data_source(size_t buffer_len);
        virtual ~data_source() = default;
        /** \brief Collects audio
         *
         * \p grab_audio is defined to fill up \p output with audio data.
         * The data outputted by this function has \p gain applied to it, together with a windowing function.
         * This function shall print it's error message if applicable.
         * The numbers outputted by this function were normalized before gain was applied, unless gain is 1
         * normalization is not a guarantee
         *
         * \param output Output buffer. If the data retreived is stereo it will get mixed together before it's
         * outputted.
         * \returns \p false if retreival failed. The program is preticted to exit if data retreival fails.
         */
        bool grab_audio(double *output);

    private:
        /** \brief Synchronously grabs unprocessed audio from the server.
         *
         * Values produced by this function shall be normalized.
         *
         * \param output Target buffer
         * \param buffer_len Target buffer length, in elements
         * \return false on failure, prints the error message, if any.
         */
        virtual bool do_grab_audio(double *output) = 0;

        size_t buffer_len;
        std::unique_ptr<double[]> window_func_table;
        std::unique_ptr<double[]> unwindowed;
    };
} // namespace visualize

#endif // AUDIO_SOURCE_HPP
