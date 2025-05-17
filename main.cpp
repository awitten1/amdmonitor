#include <chrono>
#include <cstdlib>
#include <iostream>
#include "AMDProfileController.h"
#include "AMDTDefinitions.h"
#include "AMDTPowerProfileApi.h"
#include "sql.hpp"

int main() {
    auto result = AMDTPwrProfileInitialize(AMDT_PWR_MODE_TIMELINE_ONLINE);
    if (result != AMDT_STATUS_OK) {
        std::cerr << "failed to initialize power api " << result << std::endl;
        return EXIT_FAILURE;
    }
    auto conn = GetDuckdbConnection();
    DDL(conn);

    AMDTUInt32 counter_id;
    result = AMDTPwrGetCounterId(AMD_PWR_SOCKET_TEMPERATURE, &counter_id);
    if (result != AMDT_STATUS_OK) {
        std::cerr << "failed getting counter id for socket temperature" << std::endl;
        return EXIT_FAILURE;
    }

    result = AMDTPwrEnableCounter(counter_id);
    if (result != AMDT_STATUS_OK) {
        std::cerr << "failed enabling socket temperature counter" << std::endl;
        return EXIT_FAILURE;
    }

    auto sample_frequency = std::chrono::milliseconds(5000);

    result = AMDTPwrSetTimerSamplingPeriod(sample_frequency.count());
    if (result != AMDT_STATUS_OK) {
        std::cerr << "failed setting sample period" << std::endl;
        return EXIT_FAILURE;
    }

    result = AMDTPwrStartProfiling();
    if (result != AMDT_STATUS_OK) {
        std::cerr << "failed to start profiling" << std::endl;
        return EXIT_FAILURE;
    }

    

    
    return 0;
}