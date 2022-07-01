# auto make on linux #
mkdir build
cd build
cmake ..
make

# auto run with debug-level-2 #
cd ..
cd bin
sudo setcap cap_net_bind_service=+ep dnsrelay
./dnsrelay -d2