#include "pwr_counters.hpp"
#include <vector>

std::string PwrCounter::DeviceTypeToString(AMDTDeviceType type) {
    switch (type) {
    case AMDT_PWR_DEVICE_PACKAGE:
        return "socket";
    case AMDT_PWR_DEVICE_CPU_COMPUTE_UNIT:
        return "cpu compute unit";
    case AMDT_PWR_DEVICE_CPU_CORE:
        return "cpu compute unit core";
    case AMDT_PWR_DEVICE_DIE:
        return "die";
    case AMDT_PWR_DEVICE_PHYSICAL_CORE:
        return "core";
    case AMDT_PWR_DEVICE_THREAD:
        return "thread";
    case AMDT_PWR_DEVICE_INTERNAL_GPU:
        return "integrated gpu";
    case AMDT_PWR_DEVICE_EXTERNAL_GPU:
        return "external gpu";
    case AMDT_PWR_DEVICE_SVI2:
        return "serial voltage interface";
    default:
        return "invalid";
    }
}

std::string PwrCounter::AggToString(AMDTPwrAggregation t) {
    switch (t) {
    case AMDT_PWR_VALUE_SINGLE:
        return "single";
    case AMDT_PWR_VALUE_CUMULATIVE:
        return "cumulative";
    case AMDT_PWR_VALUE_HISTOGRAM:
        return "historgram";
    default:
        return "invalid";
    }
}

std::string PwrCounter::UnitsToString(AMDTPwrUnit u) {
    switch (u) {

    case AMDT_PWR_UNIT_TYPE_COUNT:
        return "count";
    case AMDT_PWR_UNIT_TYPE_NUMBER:
        return "number";
    case AMDT_PWR_UNIT_TYPE_PERCENT:
        return "percent";
    case AMDT_PWR_UNIT_TYPE_RATIO:
        return "ratio";
    case AMDT_PWR_UNIT_TYPE_MILLI_SECOND:
        return "millisecond";
    case AMDT_PWR_UNIT_TYPE_JOULE:
        return "joule";
    case AMDT_PWR_UNIT_TYPE_WATT:
        return "watt";
    case AMDT_PWR_UNIT_TYPE_VOLT:
        return "volt";
    case AMDT_PWR_UNIT_TYPE_MILLI_AMPERE:
        return "ampere";
    case AMDT_PWR_UNIT_TYPE_MEGA_HERTZ:
        return "MHZ";
    case AMDT_PWR_UNIT_TYPE_CENTIGRADE:
        return "celsius";
    default:
        return "invalid";
    }
}

std::string PwrCounter::CategoryToString(AMDTPwrCategory c) {
    switch (c) {
    case AMDT_PWR_CATEGORY_POWER:
        return "power";
    case AMDT_PWR_CATEGORY_FREQUENCY:
        return "frequency";
    case AMDT_PWR_CATEGORY_TEMPERATURE:
        return "temperature";
    case AMDT_PWR_CATEGORY_VOLTAGE:
        return "voltage";
    case AMDT_PWR_CATEGORY_CURRENT:
        return "current";
    case AMDT_PWR_CATEGORY_PSTATE:
        return "pstate";
    case AMDT_PWR_CATEGORY_CSTATES_RESIDENCY:
        return "cstates_residency";
    case AMDT_PWR_CATEGORY_TIME:
        return "time";
    case AMDT_PWR_CATEGORY_ENERGY:
        return "energy";
    case AMDT_PWR_CATEGORY_CORRELATED_POWER:
        return "power";
    case AMDT_PWR_CATEGORY_CAC:
        return "cac";
    case AMDT_PWR_CATEGORY_CONTROLLER:
        return "controller";
    case AMDT_PWR_CATEGORY_DPM:
        return "dpm";
    default:
        return "invalid";
    }
}

std::vector<PwrCounter> PwrCounter::GetAllPwrCounters() {
    static std::vector<PwrCounter> counters;
    if (!counters.empty()) {
        return counters;
    }
    std::vector<PwrCounter> ret;
    AMDTUInt32 num_counters;
    AMDTPwrCounterDesc* power_counter_desc;
    auto result = AMDTPwrGetSupportedCounters(&num_counters, &power_counter_desc);
    if (result != AMDT_STATUS_OK) {
        std::ostringstream oss;
        oss << "failed to get supported counters " << std::hex << result << std::endl;
        throw std::runtime_error{oss.str()};
    }
    for (int i = 0; i < num_counters; ++i) {
        auto& desc = power_counter_desc[i];
        PwrCounter counter(&desc);
        ret.push_back(std::move(counter));
    }
    counters = std::move(ret);
    return counters;
}
