#include <chrono>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <numeric>
#include <thread>
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
    OnBlockExit obe(Cleanup);

    signal(SIGINT, stopProfiling);
    signal(SIGTERM, stopProfiling);

    auto result = AMDTPwrProfileInitialize(AMDT_PWR_MODE_TIMELINE_ONLINE);
    if (result != AMDT_STATUS_OK) {
        std::cerr << "failed to initialize power api " << std::hex << result << std::endl;
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

    auto frequency_counters = std::accumulate(counters.begin(), counters.end(), std::vector<PwrCounter>{},
    [](std::vector<PwrCounter> c, const PwrCounter& pwr_counter) {
        if (pwr_counter.category_ == AMDT_PWR_CATEGORY_FREQUENCY) {
            c.push_back(pwr_counter);
        }
        return c;
    });

    auto pstate_counters = std::accumulate(counters.begin(), counters.end(), std::vector<PwrCounter>{},
    [](std::vector<PwrCounter> c, const PwrCounter& pwr_counter) {
        if (pwr_counter.category_ == AMDT_PWR_CATEGORY_PSTATE) {
            c.push_back(pwr_counter);
        }
        return c;
    });

    auto power_counters = std::accumulate(counters.begin(), counters.end(), std::vector<PwrCounter>{},
    [](std::vector<PwrCounter> c, const PwrCounter& pwr_counter) {
        if (pwr_counter.category_ == AMDT_PWR_CATEGORY_POWER) {
            c.push_back(pwr_counter);
        }
        return c;
    });

    for (auto& counter : temperature_counters) {
        counter.EnableCounter();
    }
    for (auto& counter : frequency_counters) {
        counter.EnableCounter();
    }
    for (auto& counter : pstate_counters) {
        counter.EnableCounter();
    }
    for (auto& counter : power_counters) {
        counter.EnableCounter();
    }

    auto sample_interval = std::chrono::milliseconds(5000);


    result = AMDTPwrSetTimerSamplingPeriod(sample_interval.count());
    if (result != AMDT_STATUS_OK) {
        std::cerr << "failed setting sampling period " << std::hex << result << std::endl;
        return result;
    }

    result = AMDTPwrStartProfiling();
    if (result != AMDT_STATUS_OK) {
        std::cerr << "failed to start profiler " << std::hex << result << std::endl;
        return result;
    }

    bool stopProfiling = false;
    AMDTUInt32 nbrSamples = 0;

    while(isProfiling) {
        std::cout << "in profiling loop" << std::endl;
        std::this_thread::sleep_for(sample_interval + std::chrono::milliseconds(100));


        PwrCounter::ReadAndStoreEnabledCounters(conn);
    }





    return 0;
}