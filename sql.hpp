
#pragma once

#include "AMDTDefinitions.h"
#include "duckdb.hpp"
#include <chrono>
#include <iostream>
#include <sstream>


duckdb::Connection GetDuckdbConnection();
void DDL(duckdb::Connection& conn);
inline void StoreMeasurement(duckdb::Connection& conn, AMDTUInt32 counter_id, double result, std::chrono::microseconds time_since_epoch) {
    std::ostringstream oss;
    oss << "INSERT INTO measurements BY NAME SELECT " << counter_id << " AS counter_id, "
        << result << " AS result, make_timestamp(" << time_since_epoch.count() << ") AS ts";
    auto qresult = conn.Query(oss.str());
    if (qresult->HasError()) {
        std::ostringstream oss;
        oss << "failed inserting row.  reason = " << qresult->GetError();
        std::cerr << oss.str();
    }
}