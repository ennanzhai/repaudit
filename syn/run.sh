# running script
cd src
echo compiling parser...
make parser

echo generating intermidiate code...
../build/parser ../example/input.src ../build/intermediate.cpp

echo compiling solver...
make solver

echo copying files...
cp ./maxino-2015-k16-static ../build/
cp ../example/topo.conf ../build/

echo running solver...
cd ../build/
./solver
