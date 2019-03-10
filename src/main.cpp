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

#include <SDL.h>
#include <algorithm>
#include <atomic>
#include <data_sources/pulseaudio.hpp>
#include <fftw3.h>
#include <iostream>
#include <math.h>
#include <mutex>
#include <thread>

namespace visualize {
    static std::atomic_bool run = true;
    //! Normalized (0.0-0.1) output
    static std::unique_ptr<double[]> buffer;
    static std::mutex buffer_lock;
    static struct {
        //! gravity shall be 1000 * peek_reduction_per_sample
        double gravity = 100.0 / 6.0;
        long barcount = 150;
    } config;

    void audio_thread(size_t complex_count, std::unique_ptr<data_source> src) {
        auto fftw_in = std::make_unique<double[]>(complex_count * 2);
        auto fftw_out = std::make_unique<fftw_complex[]>(complex_count + 1);

        // auto gain control
        double gain = 1;

        auto plan = fftw_plan_dft_r2c_1d(int(complex_count * 2), fftw_in.get(), fftw_out.get(), FFTW_MEASURE);

        // no need to recompute this value
        double gravity = config.gravity / 1000;
        while (run.load(std::memory_order_relaxed)) {
            src->grab_audio(fftw_in.get(), gain);
            fftw_execute(plan);
            // used for gain ctl
            double rms = 0;
            {
                std::unique_lock _(buffer_lock);
                for (size_t i = 0; i < complex_count + 1; i++) {
                    auto out = hypot(fftw_out[i][0], fftw_out[i][1]);
                    rms += out * out / complex_count;
                    out = std::min(out, 1.0);
                    auto &finalized = buffer[i];
                    // keep peeks
                    finalized = std::max(out, finalized);
                    // gravitating to make sure peeks arent horrible
                    if (finalized >= gravity) {
                        finalized -= gravity;
                    } else {
                        finalized = 0;
                    }
                }
            }

            // gain ctl
            if (rms > 0.5 * 0.5) {
                gain -= 0.1;
            } else if (rms < 0.3 * 0.3) {
                gain += 0.1;
            }
            if (gain > 1) {
                gain = 1;
            }
        }
        fftw_destroy_plan(plan);
    } // namespace visualize
} // namespace visualize

void rescale_rects(std::unique_ptr<SDL_Rect[]> &rects, int width) {
    auto barcount = int(visualize::config.barcount);
    int w = width / barcount;
    int cpos = width % barcount / 2;
    std::for_each(&rects[0], &rects[size_t(barcount)], [w, &cpos](auto &r) {
        r.w = w;
        r.x = cpos;
        r.y = 1;
        r.h = 10;
        cpos += w;
    });
}

int main() {
    size_t buffer_size = 2048;
    visualize::buffer = std::make_unique<double[]>(buffer_size * 2);
    std::thread audio_thread(visualize::audio_thread, buffer_size,
                             std::make_unique<visualize::pulseaudio_source>(buffer_size * 2));
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    auto window = SDL_CreateWindow("Visualizer", 100, 100, 800, 480, SDL_WINDOW_RESIZABLE);
    auto renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    auto bars = std::make_unique<double[]>(size_t(visualize::config.barcount));

    int width, height;
    auto rects = std::make_unique<SDL_Rect[]>(size_t(visualize::config.barcount));
    SDL_GetWindowSize(window, &width, &height);
    rescale_rects(rects, width);
    while (visualize::run.load(std::memory_order_relaxed)) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT: visualize::run.store(false, std::memory_order_relaxed); break;
            case SDL_WINDOWEVENT: {
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    width = event.window.data1;
                    height = event.window.data2;
                    rescale_rects(rects, width);
                }
                break;
            }
            case SDL_KEYDOWN: {
                auto sym = event.key.keysym;
                if (sym.sym == SDLK_q && !sym.mod) {
                    visualize::run.store(false, std::memory_order_relaxed);
                } else if (sym.sym == SDLK_F11 && !sym.mod) {
                    SDL_SetWindowFullscreen(window, ~SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN);
                }
                break;
            }
            }
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        std::fill_n(bars.get(), visualize::config.barcount, 0);
        {
            auto per_bar = buffer_size / size_t(visualize::config.barcount);
            std::unique_lock _(visualize::buffer_lock);
            for (size_t i = 0; i < per_bar * size_t(visualize::config.barcount); i++) {
                bars[i / per_bar] += visualize::buffer[i] / double(per_bar);
            }
        }

        for (size_t i = 0; i < size_t(visualize::config.barcount); i++) {
            rects[i].h = static_cast<int>(bars[i] * height);
            rects[i].y = height - rects[i].h;
        }

        if (SDL_RenderFillRects(renderer, rects.get(), int(visualize::config.barcount)) < 0) {
            std::cerr << SDL_GetError() << std::endl;
        }

        SDL_RenderPresent(renderer);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    audio_thread.join();
}
