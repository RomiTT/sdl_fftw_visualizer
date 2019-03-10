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
#include <atomic>
#include <cmath>
#include <data_sources/pulseaudio.hpp>
#include <fftw3.h>
#include <filter.hpp>
#include <filters/clip_filter.hpp>
#include <filters/peek_filter.hpp>
#include <filters/sagc_filter.hpp>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>

namespace visualize {
    struct color {
        uint8_t r, g, b;
    };

    struct buffer {
        buffer(size_t size) : data_size(size), data(std::make_unique<double[]>(size)) {}
        std::pair<double *, std::unique_lock<std::mutex>> acquire() {
            return std::make_pair(data.get(), std::unique_lock(lock));
        }

        buffer(const buffer &other) = delete;
        buffer &operator=(const buffer &other) = delete;

        const size_t data_size;

    private:
        std::unique_ptr<double[]> data;
        std::mutex lock;
    };

    static struct {
        //! gravity shall be 1000 * peek_reduction_per_sample
        double gravity = 100.0 / 6.0;
        int barcount = 160;
        size_t resolution = 2048;
        color background = { 0, 0, 0 };
        color foreground = { 255, 255, 255 };
    } config;

    void audio_thread(std::atomic_bool &run, buffer &buffer, std::function<std::unique_ptr<data_source>()> factory,
                      std::vector<std::unique_ptr<filter>> &filters) {
        auto src = factory();

        auto fftw_in = std::make_unique<double[]>(buffer.data_size * 2);
        auto fftw_out = std::make_unique<fftw_complex[]>(buffer.data_size + 1);
        auto plan = fftw_plan_dft_r2c_1d(int(buffer.data_size * 2), fftw_in.get(), fftw_out.get(), FFTW_MEASURE);

        while (run.load(std::memory_order_relaxed)) {
            if (!src->grab_audio(fftw_in.get())) {
                run.store(false, std::memory_order_relaxed);
                break;
            }
            fftw_execute(plan);
            {
                auto [data, _] = buffer.acquire();
                for (size_t i = 0; i < buffer.data_size; i++) {
                    data[i] = hypot(fftw_out[i][0], fftw_out[i][1]);
                }
                for (auto &filter : filters) {
                    filter->apply(data);
                }
            }
        }
        fftw_destroy_plan(plan);
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
} // namespace visualize

int main() {
    visualize::buffer buf(visualize::config.resolution);
    std::atomic_bool run = true;
    std::thread audio_thread([&buf, &run]() {
        std::vector<std::unique_ptr<visualize::filter>> filters;

        filters.emplace_back(new visualize::sagc_filter(buf.data_size));
        filters.emplace_back(new visualize::clip_filter(buf.data_size));
        filters.emplace_back(new visualize::peek_filter(buf.data_size, visualize::config.gravity));

        visualize::audio_thread(
            run, buf, [&buf]() { return std::make_unique<visualize::pulseaudio_source>(buf.data_size * 2); }, filters);
    });
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    auto window = SDL_CreateWindow("Visualizer", 100, 100, 800, 480, SDL_WINDOW_RESIZABLE);
    auto renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    auto bars = std::make_unique<double[]>(size_t(visualize::config.barcount));

    int width, height;
    auto rects = std::make_unique<SDL_Rect[]>(size_t(visualize::config.barcount));
    SDL_GetWindowSize(window, &width, &height);
    visualize::rescale_rects(rects, width);
    while (run.load(std::memory_order_relaxed)) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT: run.store(false, std::memory_order_relaxed); break;
            case SDL_WINDOWEVENT: {
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    width = event.window.data1;
                    height = event.window.data2;
                    visualize::rescale_rects(rects, width);
                }
                break;
            }
            case SDL_KEYDOWN: {
                auto sym = event.key.keysym;
                if (sym.sym == SDLK_q && !sym.mod) {
                    run.store(false, std::memory_order_relaxed);
                } else if (sym.sym == SDLK_F11 && !sym.mod) {
                    SDL_SetWindowFullscreen(window, ~SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN);
                }
                break;
            }
            }
        }
        auto [foreground, background] = std::tie(visualize::config.foreground, visualize::config.background);
        SDL_SetRenderDrawColor(renderer, background.r, background.g, background.b, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, foreground.r, foreground.g, foreground.b, 255);

        std::fill_n(bars.get(), visualize::config.barcount, 0);
        {
            auto per_bar = buf.data_size / size_t(visualize::config.barcount);
            auto [buffer, _] = buf.acquire();
            for (size_t i = 0; i < per_bar * size_t(visualize::config.barcount); i++) {
                bars[i / per_bar] += buffer[i] / double(per_bar);
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
    SDL_QuitSubSystem(SDL_INIT_EVERYTHING);
    SDL_Quit();
    audio_thread.join();
}
