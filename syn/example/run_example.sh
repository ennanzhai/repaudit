if [ $# != 1 ] ; then
   echo usage: run_example.sh \<example_file_name\>
   exit 1
fi


# running script
rm ../build/* # clean up build

cd ../src
echo compiling parser...
make parser

echo generating intermidiate code...
../build/parser ../example/$1 ../build/intermediate.cpp

echo compiling solver...
make solver

echo copying files...
cp ./maxino-2015-k16-static ../build/
cp ../example/topo.conf ../build/

echo running solver...
cd ../build/
./solver
