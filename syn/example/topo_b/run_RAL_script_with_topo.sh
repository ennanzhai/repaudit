# current directory is:
# ./project/example/topo_a

topo_file_name=$(ls | grep *.conf)

cp -b $topo_file_name ../topo.conf # copy & replace to [topo.conf] with back up previous [topo.conf] to [topo.conf~]
echo "running example with topo.conf = [$topo_file_name]"

cd ..
sh ./run_example.sh ./topo_b/RAL_script
cp ./topo.conf~ ./topo.conf # recover previous topo.conf
rm ./topo.conf~ # cleanup backup file

