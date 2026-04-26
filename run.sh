#!/bin/bash
set -e

# [ENV] V-Sync ve Sürücü Kilidini Kırma (Linux & GPU Özel)
export vblank_mode=0
export __GL_SYNC_TO_VBLANK=0
export mesa_glthread=true
export RADV_PERFTEST=aco
export MESA_DEBUG=0
export MESA_NO_ERROR=1
export MESA_GL_VERSION_OVERRIDE=4.5

# İşlemciyi Performans Moduna Alma (Ondemand dalgalanmasını önler)
echo "İşlemci maksimum performans moduna kilitleniyor..."
echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor > /dev/null

mkdir -p build
cd build
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)

export LD_LIBRARY_PATH=$PWD:$LD_LIBRARY_PATH
./hqxball_client
