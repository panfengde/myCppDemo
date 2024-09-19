//
// Created by 潘峰 on 2024/7/4.
//

#ifndef TIMECOUNT_H
#define TIMECOUNT_H

#include <iostream>
#include <chrono>
#include <unordered_map>
#include <string>

long long getTimeMillSeconds() {
    // 获取当前时间点
    auto now = std::chrono::system_clock::now();
    // 将当前时间点转换为自纪元以来的毫秒数
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    // 获取毫秒数
    long long milliseconds = duration.count();

    return milliseconds;
}

class AccumulatingTimer {
public:
    void start() {
        if (countIndex == 0) {
            start_time = getTimeMillSeconds();
        }
        countIndex++;
    }

    void stop() {
        countIndex--;
        if (countIndex == 0) {
            auto end_time = getTimeMillSeconds();
            accumulated_time += end_time - start_time;
        }
    }

    [[nodiscard]] long long getAccumulatedTime() const {
        return accumulated_time;
    }

private:
    long long start_time;
    int countIndex;
    long long accumulated_time = 0.0;
};

std::unordered_map<std::string, AccumulatingTimer> timer_map;

void showTimeMNap() {
    if (timer_map.empty()) {
        return;
    }
    for (auto &oneTime: timer_map) {
        std::cout << "time for " << oneTime.first
                << ": " << oneTime.second.getAccumulatedTime() << " ms" << std::endl;
    }
}

#ifdef TIMECOUNT
#define START_TIMER(id) do { \
timer_map[id].start(); \
} while(0)
#else
#define START_TIMER(id) ((void)0)
#endif

#ifdef TIMECOUNT
#define STOP_TIMER(id) do { \
timer_map[id].stop(); \
} while(0)
#else
#define STOP_TIMER(id) ((void)0)
#endif

#ifdef TIMECOUNT
#define PRINT_ACCUMULATED_TIME() do { \
showTimeMNap(); \
} while(0)
#else
#define PRINT_ACCUMULATED_TIME() ((void)0)
#endif


#ifdef TIMECOUNT
#define CLEAR_ACCUMULATED_TIME() do { \
timer_map={}; \
} while(0)
#else
#define CLEAR_ACCUMULATED_TIME() ((void)0)
#endif

#endif //DTIMECOUNT_H
