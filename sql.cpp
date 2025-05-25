
#include "pwr_counters.hpp"
#include "AMDTPowerProfileDataTypes.h"
#include "duckdb.hpp"
#include <fmt/core.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "sql.hpp"

static duckdb::DuckDB db;

duckdb::Connection GetDuckdbConnection() {
    duckdb::Connection conn(db);
    conn.Query("ATTACH 'measurements'; USE measurements;");
    return conn;
}


void InsertDevices(duckdb::Connection& conn) {
    for (int i = 1; i < AMDT_PWR_DEVICE_CNT; ++i) {
        auto type = AMDTDeviceType(i);
        conn.Query(fmt::format("INSERT OR IGNORE INTO devices VALUES({},'{}')", i, PwrCounter::DeviceTypeToString(type)));
    }
}

void StoreCounters(duckdb::Connection& con) {
    auto pwr_counters = PwrCounter::GetAllPwrCounters();

    auto res = con.Query("PREPARE insert_into_counters AS "
        "INSERT OR IGNORE INTO pwr_counters BY NAME "
        "SELECT $counter_id as counter_id, "
        "$counter_name as counter_name, "
        "$counter_description as counter_description, "
        "$device_id as device_id, "
        "$device_name as device_name, "
        "$device_type as device_type, "
        "$device_instance_id as device_instance_id, "
        "$category as category, "
        "$aggregation as aggregation, "
        "$min_value as min_value, "
        "$max_value as max_value, "
        "$units as units, "
        "$has_child_counter as has_child_counter");
    for (int i = 0; i < pwr_counters.size(); ++i) {
        auto& pwr_counter = pwr_counters[i];

        std::ostringstream oss;
        oss << "EXECUTE insert_into_counters("
            << "counter_id := " << pwr_counter.counter_id_
            << ", counter_name := '" << pwr_counter.name_ << "'"
            << ", counter_description := '" << pwr_counter.description_ << "'"
            << ", device_id := " << pwr_counter.device_id_
            << ", device_name := '" << PwrCounter::DeviceTypeToString(pwr_counter.dev_type_) << "'"
            << ", device_type := '" << pwr_counter.dev_type_ << "'"
            << ", device_instance_id := " << pwr_counter.dev_instance_id_
            << ", category := '" << PwrCounter::CategoryToString(pwr_counter.category_) << "'"
            << ", aggregation := '" << PwrCounter::AggToString(pwr_counter.aggregation_) << "'"
            << ", min_value := " << pwr_counter.min_value_
            << ", max_value := " << pwr_counter.max_value_
            << ", units := '" << PwrCounter::UnitsToString(pwr_counter.units_) << "'"
            << ", has_child_counter := " << pwr_counter.is_parent_counter_ << ")";

        auto res = con.Query(oss.str());
        if (res->HasError()) {
            std::cerr << res->GetError() << std::endl;
        }
    }

}


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
            "min_value FLOAT, "
            "max_value FLOAT, "
            "units VARCHAR, "
            "has_child_counter BOOLEAN "
        ") "
    );

    StoreCounters(con);

    auto result = con.Query("CREATE TABLE IF NOT EXISTS measurements( "
            " counter_id BIGINT REFERENCES pwr_counters(counter_id), "
            " result FLOAT, "
            " ts TIMESTAMP "
            ") ");
    if (result->HasError()) {
        std::ostringstream oss;
        oss << "failed to create table measurements.  reason = " << result->GetError() << std::endl;
        std::cerr << oss.str();
        throw std::runtime_error{oss.str()};
    }
}