
#include "AMDTDefinitions.h"
#include "AMDTPowerProfileApi.h"
#include "AMDTPowerProfileDataTypes.h"
#include "duckdb.hpp"
#include "sql.hpp"
#include <chrono>
#include <ratio>
#include <sstream>
#include <stdexcept>
#include <map>
#include <vector>
#include <iostream>

class PwrCounter {

    static inline std::map<AMDTUInt32, const PwrCounter*> enabled_counters_;

public:

    ~PwrCounter() {
        EraseCounter(*this);
    }

    void EnableCounter() {
        auto result = AMDTPwrEnableCounter(counter_id_);
        if (result != AMDT_STATUS_OK) {
            std::ostringstream oss;
            oss << "failed to enable counter " << name_ << std::endl;
            std::cerr << oss.str();
            throw std::runtime_error{oss.str()};
        }
        enabled_ = true;
    }

    static void ReadAndStoreEnabledCounters(duckdb::Connection& conn) {
        AMDTUInt32 num_samples;
        AMDTPwrSample* samples;
        auto result = AMDTPwrReadAllEnabledCounters(&num_samples, &samples);
        if (result != AMDT_STATUS_OK) {
            std::ostringstream oss;
            oss << "reading counters not ok " << std::hex << result << std::endl;
            std::cerr << oss.str();
            throw std::runtime_error{oss.str()};
        }

        for (int i = 0; i < num_samples; ++i) {
            auto& sample = samples[i];
            if (!sample.m_counterValues) {
                continue;
            }
            for (int j = 0; j < sample.m_numOfCounter; ++j) {
                auto& counter_result = sample.m_counterValues[j];
                std::chrono::microseconds ts = std::chrono::seconds(sample.m_systemTime.m_second) +
                    std::chrono::microseconds(sample.m_systemTime.m_microSecond) +
                    std::chrono::milliseconds(sample.m_elapsedTimeMs);
                StoreMeasurement(conn, counter_result.m_counterID, counter_result.m_data, ts);
            }
        }
    }

    static std::string DeviceTypeToString(AMDTDeviceType type);
    static std::string AggToString(AMDTPwrAggregation t);
    static std::string UnitsToString(AMDTPwrUnit u);
    static std::string CategoryToString(AMDTPwrCategory c);

    static std::vector<PwrCounter> GetAllPwrCounters();

    AMDTUInt32 counter_id_;
    AMDTUInt32 device_id_;
    AMDTDeviceType dev_type_;
    AMDTUInt32 dev_instance_id_;
    std::string name_;
    std::string description_;
    AMDTPwrCategory category_;
    AMDTPwrAggregation aggregation_;
    AMDTFloat64          min_value_;
    AMDTFloat64          max_value_;
    AMDTPwrUnit          units_;
    bool is_parent_counter_;
    bool enabled_ = false;

private:

    PwrCounter(AMDTUInt32 counter_id) {
        AMDTPwrCounterDesc counter_desc;
        auto status = AMDTPwrGetCounterDesc(counter_id, &counter_desc);
        if (status != AMDT_STATUS_OK) {
            throw std::runtime_error{"failed to get counter desc"};
        }
    }

    PwrCounter(AMDTPwrCounterDesc* counter_desc) :
        counter_id_(counter_desc->m_counterID), device_id_(counter_desc->m_deviceId),
        dev_type_(counter_desc->m_devType),
        dev_instance_id_(counter_desc->m_devInstanceId),
        name_(counter_desc->m_name),
        description_(counter_desc->m_description),
        category_(counter_desc->m_category),
        aggregation_(counter_desc->m_aggregation),
        min_value_(counter_desc->m_minValue),
        max_value_(counter_desc->m_maxValue),
        units_(counter_desc->m_units),
        is_parent_counter_(counter_desc->m_isParentCounter)
         {

    }

    static void EraseCounter(const PwrCounter& counter) {
        enabled_counters_.erase(counter.counter_id_);
    }

    static void EnableCounter(const PwrCounter& counter) {
        enabled_counters_[counter.counter_id_] = &counter;
    }
};