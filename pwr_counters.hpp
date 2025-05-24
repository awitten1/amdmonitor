
#include "AMDTDefinitions.h"
#include "AMDTPowerProfileApi.h"
#include "AMDTPowerProfileDataTypes.h"
#include <sstream>
#include <stdexcept>
#include <vector>
#include <iostream>

class PwrCounter {
public:
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
};