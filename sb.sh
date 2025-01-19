export PPP=/arm/tools_llvm/bin
export CCC=$PPP/clang-18
export CPP=$PPP/clang++-18
export ANA=$CCC
#export SB=/arm/tools_llvm/bin/scan-build
export SB=~/.local/bin/scan-build
set -x
rm -Rf ../build/*
${SB} -v --override-compiler --use-analyzer=$ANA --use-cc=$CCC --use-c++=$CPP  --analyzer-target=arm-none-eabi cmake  -DUSE_SCAN_BUILD=True  ..
${SB} -v --override-compiler --use-analyzer=$ANA --use-cc=$CCC --use-c++=$CPP  --analyzer-target=arm-none-eabi make VERBOSE=1  #-unknown-elf make

