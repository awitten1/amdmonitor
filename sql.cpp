
#include "AMDTDefinitions.h"
#include "AMDTPowerProfileApi.h"
#include "AMDTPowerProfileDataTypes.h"
#include "duckdb.hpp"
#include <fmt/core.h>
#include <iostream>
#include "sql.hpp"

static duckdb::DuckDB db;

duckdb::Connection GetDuckdbConnection() {
    duckdb::Connection conn(db);
    conn.Query("ATTACH 'measurements'; USE measurements;");
    return conn;
}

std::string DeviceTypeToString(AMDTDeviceType type) {
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

std::string AggToString(AMDTPwrAggregation t) {
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

std::string UnitsToSTring(AMDTPwrUnit u) {\
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

std::string CategoryToString(AMDTPwrCategory c) {
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

void InsertDevices(duckdb::Connection& conn) {
    for (int i = 1; i < AMDT_PWR_DEVICE_CNT; ++i) {
        auto type = AMDTDeviceType(i);
        conn.Query(fmt::format("INSERT OR IGNORE INTO devices VALUES({},'{}')", i, DeviceTypeToString(type)));
    }
}

void StoreCounters(duckdb::Connection& con) {
    AMDTUInt32 num_counters;
    AMDTPwrCounterDesc* power_counter_desc;
    auto result = AMDTPwrGetSupportedCounters(&num_counters, &power_counter_desc);
    if (result != AMDT_STATUS_OK) {
        std::cerr << "failed to get supported counters" << std::endl;
        return;
    }

    con.Query("PREPARE insert_into_counters AS INSERT OR IGNORE INTO pwr_counters VALUES($1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11,$12,$13)");
    for (int i = 0; i < num_counters; ++i) {
        auto& desc = power_counter_desc[i];

        auto res = con.Query(fmt::format("EXECUTE insert_into_counters({},'{}','{}',{},'{}',{},{},'{}','{}',{},{},'{}',{})", 
            desc.m_counterID,
            desc.m_name,
            desc.m_description,
            desc.m_deviceId,
            DeviceTypeToString(desc.m_devType),
            desc.m_devType,
            desc.m_devInstanceId,
            CategoryToString(desc.m_category),
            AggToString(desc.m_aggregation),
            desc.m_minValue,
            desc.m_maxValue,
            UnitsToSTring(desc.m_units),
            desc.m_isParentCounter
        ));
        if (res->HasError()) {
            std::cerr << res->GetError() << std::endl;
        }
    }

}

// typedef struct AMDTPwrCounterDesc
// {
//     AMDTUInt32           m_counterID;       /**< Counter index */
//     AMDTUInt32           m_deviceId;        /**< Device Id */
//     AMDTDeviceType       m_devType;         /**< Device type- compute unit/Core/ package/ dGPU */
//     AMDTUInt32           m_devInstanceId;   /**< Device instance id within the device type */
//     char*                m_name;            /**< Name of the counter */
//     char*                m_description;     /**< Description of the counter */
//     AMDTPwrCategory      m_category;        /**< Power/Freq/Temperature */
//     AMDTPwrAggregation   m_aggregation;     /**< Single/Histogram/Cumulative */
//     AMDTFloat64          m_minValue;        /**< Minimum possible counter value */
//     AMDTFloat64          m_maxValue;        /**< Maximum possible counter value */
//     AMDTPwrUnit          m_units;           /**< Seconds/MHz/Joules/Watts/Volt/Ampere */
//     bool                 m_isParentCounter; /**< If the counter has some child counters*/
// } AMDTPwrCounterDesc;
void DDL(duckdb::Connection& con) {
    con.Query(
        "CREATE TABLE IF NOT EXISTS pwr_counters( "
            "counter_id BIGINT PRIMARY KEY, " 
            "counter_name VARCHAR, " 
            "counter_description VARCHAR, "
            "device_id BIGINT, "
            "device_name VARCHAR, "
            "device_type VARCHAR, "
            "device_instance_id BIGINT, "
            "category VARCHAR, "
            "aggregation VARCHAR, "
            "minValue FLOAT, "
            "maxValue FLOAT, "
            "units VARCHAR, "
            "has_child_counter BOOLEAN "
        ") "
    );

    StoreCounters(con);
}