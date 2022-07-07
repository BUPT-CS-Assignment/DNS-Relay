% auto make on windows %
mkdir build
cd build
cmake ..
make

% auto run with debug-level-1 %
cd ..
cd bin
dnsrelay -d -c16