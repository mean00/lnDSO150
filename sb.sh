export PPP=/arm/prebuilt1502/bin
export CCC=$PPP/clang-15
export CPP=$PPP/clang++-15
export ANA=$CCC

rm -Rf ../build/*
/riscv/tools_llvm/bin/scan-build -v --use-analyzer=$ANA --use-cc=$CCC --use-c++=$CPP  --analyzer-target=arm-none-eabi cmake  -DUSE_SCAN_BUILD=True  ..
/riscv/tools_llvm/bin/scan-build -v --use-analyzer=$ANA --use-cc=$CCC --use-c++=$CPP  --analyzer-target=arm-none-eabi make VERBOSE=1  #-unknown-elf make

