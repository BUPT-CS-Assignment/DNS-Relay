% auto make on windows %
mkdir build
cd build
cmake ..
make

@REM % auto run with debug-level-1 %
@REM cd ..
@REM cd bin
@REM dnsrelay -d -c64 =10.3.9.44