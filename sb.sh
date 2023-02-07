export PPP=/arm/tools_llvm/bin
export CCC=$PPP/clang-17
export CPP=$PPP/clang++-17
export ANA=$CCC
set -x
rm -Rf ../build/*
/arm/tools_llvm/bin/scan-build -v --use-analyzer=$ANA --use-cc=$CCC --use-c++=$CPP  --analyzer-target=arm-none-eabi cmake  -DUSE_SCAN_BUILD=True  ..
/arm/tools_llvm/bin/scan-build -v --use-analyzer=$ANA --use-cc=$CCC --use-c++=$CPP  --analyzer-target=arm-none-eabi make VERBOSE=1  #-unknown-elf make

