# AMD observability tools

## Building

First, install AMDuProf.  For me (on ubuntu with the .deb installer) AMDuProf gets installed to /opt/AMDuProf_5.0-1479.

Then run (your amduprof location may vary)
```
cmake -B build -G Ninja -DAMD_UPROF_ROOT=/opt/AMDuProf_5.0-1479
cmake --build build
```

## Troubleshooting

Might need to run
```
/opt/AMDuProf_5.0-1479/bin/AMDPowerProfilerDriver.sh install
```