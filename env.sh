
# rm -rf ./dump/*
export IGC_ShaderDumpEnable=1
export IGC_DumpToCustomDir="/home/zhuwei/workspace/tvisa/build/dump"
export ZE_AFFINITY_MASK=0
export IGC_disableCompaction=1

# write a bash code to get /opt/intel/oneapi/compiler/2024.1/ from icpx=/opt/intel/oneapi/compiler/2024.1/bin/icpx
icpx=$(which icpx)
icpx_base_path=$(dirname $(dirname "$icpx"))
echo "compiler path: $icpx_base_path"

export PATH=$PATH:${icpx_base_path}/bin/compiler/
# export IGC_VATemp=2
# igc_home=/home/zhuwei/igc
# export PATH=$igc_home/usr/bin:$PATH
# export LD_LIBRARY_PATH=$igc_home/usr/lib/x86_64-linux-gnu/:$LD_LIBRARY_PATH
# export OCL_ICD_VENDORS=$igc_home/etc/OpenCL/vendors/intel.icd
# export IGC_VISAOptions=" -TotalGRFNum 256 -enableBCR -nolocalra -printregusage -DPASTokenReduction -enableHalfLSC"
echo "ocloc path: $(which ocloc)"

# icpx -fsycl $1 -I./ -I./include \
#   -fsycl-device-code-split=per_kernel -Wtautological-constant-compare sycl_misc.cpp -lblas -o a.out
# ./a.out
