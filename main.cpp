#include <chrono>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <vector>
#include "AMDProfileController.h"
#include "AMDTDefinitions.h"
#include "AMDTPowerProfileApi.h"
#include "AMDTPowerProfileDataTypes.h"
#include "sql.hpp"
#include "pwr_counters.hpp"



std::atomic<bool> isProfiling = true;

void stopProfiling(int signum) {
    std::cout << "got signal " << signum << std::endl;
    isProfiling = false;
}


void Cleanup() {
    std::cout << "stopping profiler" << std::endl;
    AMDTPwrStopProfiling();
    AMDTPwrProfileClose();
}

class OnBlockExit {
    std::function<void()> f_;
public:
    template<typename Func>
    OnBlockExit(Func&& func) : f_(std::forward<Func>(func)) {
    }

    ~OnBlockExit() {
        f_();
    }
};

int main() {
    auto result = AMDTPwrProfileInitialize(AMDT_PWR_MODE_TIMELINE_ONLINE);
    if (result != AMDT_STATUS_OK) {
        std::cerr << "failed to initialize power api " << result << std::endl;
        return EXIT_FAILURE;
    }
    auto conn = GetDuckdbConnection();
    DDL(conn);

    auto counters = PwrCounter::GetAllPwrCounters();

    auto temperature_counters = std::accumulate(counters.begin(), counters.end(), std::vector<PwrCounter>{},
        [](std::vector<PwrCounter> c, const PwrCounter& pwr_counter) {
        if (pwr_counter.category_ == AMDT_PWR_CATEGORY_TEMPERATURE) {
            c.push_back(pwr_counter);
        }
        return c;
    });

    for (auto& counter : temperature_counters) {
        result = AMDTPwrEnableCounter(counter.counter_id_);
        if (result != AMDT_STATUS_OK) {
            std::cerr << "failed enabling socket temperature counter" << std::endl;
            return EXIT_FAILURE;
        }
    }

    auto sample_frequency = std::chrono::milliseconds(5000);

    OnBlockExit obe(Cleanup);

    // result = AMDTPwrSetTimerSamplingPeriod(sample_frequency.count());
    // if (result != AMDT_STATUS_OK) {
    //     std::cerr << "failed setting sample period" << std::endl;
    //     return EXIT_FAILURE;
    // }

    // result = AMDTPwrStartProfiling();
    // if (result != AMDT_STATUS_OK) {
    //     std::cerr << "failed to start profiling" << std::endl;
    //     return EXIT_FAILURE;
    // }




    return 0;
}