for i in {0..127}; do
echo "Start $i run"
    PERIOD=$((250000 + $i * 5000))
    echo $PERIOD
    perf record -e cycles/period=$PERIOD/ -j call ~/gcc_patched/gcc-install-final/bin/g++ -w tramp3d-v4.cpp -o /dev/null
    perf script -F event,brstacksym --no-demangle >> tmp
done
