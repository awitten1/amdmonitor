
#include "AMDTDefinitions.h"
#include "AMDTPowerProfileApi.h"
#include "AMDTPowerProfileDataTypes.h"
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

void InsertDevices(duckdb::Connection& conn) {
    for (int i = 1; i < AMDT_PWR_DEVICE_CNT; ++i) {
        auto type = AMDTDeviceType(i);
        conn.Query(fmt::format("INSERT OR IGNORE INTO devices VALUES({},'{}')", i, DeviceTypeToString(type)));
    }
}

void PrintCounters() {
    AMDTUInt32 num_counters;
    AMDTPwrCounterDesc* power_counter_desc;
    auto result = AMDTPwrGetSupportedCounters(&num_counters, &power_counter_desc);
    if (result != AMDT_STATUS_OK) {
        std::cerr << "failed to get supported counters" << std::endl;
        return;
    }

    for (int i = 0; i < num_counters; ++i) {
        auto& desc = power_counter_desc[i];
        std::cout << "name " << desc.m_name 
            << " description " << desc.m_description << std::endl << desc.m_devType << " "  << desc.m_deviceId << std::endl << std::endl;
            // power_counter_desc[i].m_deviceId <<  " " << power_counter_desc[i].m_devType << std::endl;
    }

}


void DDL(duckdb::Connection& con) {
    con.Query("CREATE TABLE IF NOT EXISTS devices(device_id BIGINT PRIMARY KEY, name VARCHAR);");

    InsertDevices(con);
}