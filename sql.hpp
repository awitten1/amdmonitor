
#pragma once

#include "duckdb.hpp"


duckdb::Connection GetDuckdbConnection();
void DDL(duckdb::Connection& conn);