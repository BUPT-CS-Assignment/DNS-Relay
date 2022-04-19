% auto make on windows %
mkdir build
cd build
cmake ..
make

% auto run with debug-level-2 %
cd ..
cd bin
dnsrelay -d2