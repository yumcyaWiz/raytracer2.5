#ifndef TIMER_H
#define TIMER_H
#include <chrono>
#include <iostream>
#include <string>


class Timer {
    public:
        decltype(std::chrono::system_clock::now()) tstart;
        decltype(std::chrono::system_clock::now()) tend;

        Timer() {};
        ~Timer() {};

        void start() {
            tstart = std::chrono::system_clock::now();
        }
        void stop(const std::string& message = "") {
            tend = std::chrono::system_clock::now();
            auto dur = tend - tstart;
            auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
            std::cout << message << msec << "ms" << std::endl;
        };
};
#endif
