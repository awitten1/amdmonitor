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
    return 0;
}