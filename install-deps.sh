

#!/bin/bash

set -eux

pushd "$(realpath $(dirname $0))"

install_apt() {
    echo "running with sudo for apt"
    sudo apt update && sudo apt install -y build-essential cmake wget unzip
}

DUCKDB_VERSION=v1.2.2
install_duckdb() {
    if [ -d duckdb_install ]; then
        return
    fi
    mkdir -p duckdb_install
    pushd duckdb_install
    local duckdb_file="libduckdb-linux-amd64.zip"
    local duckdb_url="https://github.com/duckdb/duckdb/releases/download/${DUCKDB_VERSION}/$duckdb_file"
    wget "$duckdb_url"
    unzip $duckdb_file
    popd
}

install_apt
install_duckdb

popd