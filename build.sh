debug_mode=0
for arg in "$@"; do
    if [ "$arg" == "--debug" ] || [ "$arg" == "-d" ]; then
        debug_mode=1
        break
    fi
done

cd build
if [ $debug_mode -eq 1 ]; then
    cmake -DCMAKE_BUILD_TYPE=Debug ..
else
    cmake -DCMAKE_BUILD_TYPE=Release ..
fi
make
cd ..
cp build/mosfet mosfet