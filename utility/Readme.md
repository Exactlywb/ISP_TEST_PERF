# How to run

```bash
mkdir build && cd build && cmake .. && make
readelf -s --wide ~/gcc_patched/gcc-install-final/libexec/gcc/x86_64-pc-linux-gnu/9.4.0/cc1plus | grep ' FUNC ' > nm.file
./c3_utility --readelf=nm.file --output=order.sorted --number=128 --delta=127 --command="~/gcc_patched/gcc-install-final/bin/g++ -w tramp3d-v4.cpp -o /dev/null"
```