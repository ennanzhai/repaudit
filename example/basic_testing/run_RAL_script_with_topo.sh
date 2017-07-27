# current directory is:
# ./project/example/topo_a

# ranking RCGs

echo "Basic testing starts"

topo_file_name=$(ls | grep *.conf)

cp -b $topo_file_name ../topo.conf # copy & replace to [topo.conf] with back up previous [topo.conf] to [topo.conf~]
echo "running example with topo.conf = [$topo_file_name]"

cd ..
echo "=================Ranking RCGs================"
sh ./run_example.sh ./basic_testing/RAL_script_1
echo "-----------------Test pass"

echo "=================Failure Prob================"
sh ./run_example.sh ./basic_testing/RAL_script_2
echo "-----------------Test pass"


cp ./topo.conf~ ./topo.conf # recover previous topo.conf
rm ./topo.conf~ # cleanup backup file

