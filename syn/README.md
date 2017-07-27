# cs692

cs692 conducted by Ennan Zhai

##compile the project

Requirements:

1)gcc 4.9 or above installed

2)Because the sat solver we used is compiled under linux, so we require using linux system
  to run this project(Redhat, Ubuntu, etc).

To compile, you can use "run.sh" to compile both the parser and the solver at one time. Or you
can compile the parser and the solver separately. For example:

Go to the src folder,

1)Compile the parser

  make parser

2)Compile the solver

  make solver

  Note: Before compiling the solver, you should generate the intermediate.cpp file under the build
  folder. The solver will compile intermediate.cpp as well.

The generated executables are put under build folder.

##run the parser

Go to the build folder, run the parser:

./parser input_language_file intermediate_cpp_file

(The generated intermidiate_cpp_file must be named intermediate.cpp and be placed under build folder)

The parser will translate the script with the language we defined to cpp file.

##run the solver

make sure two files are copied to build folder:

1)maxino-2015-k16-static(The sat solver we used)

2)topo.conf(network topology file)

then run:

./solver

The solver will execute the commands in original language script and call the backend primitives.

##file structure

1)doc: include the api document that covers the primary primitive apis we implemented.

2)example: include some example language script. We provide a smaller one with less replicas and a
            larger one with more replicas for each function test.

3)include: the header files

4)run.sh: the compile-and-run script, the script will make the parser and solver,and execute the commands
          specified in input.src.

5)src: include the cpp files, Makefile, and sat solver binary.

6)test: include some test input script and the topology file.


##how to write a new example

To write a new example, you should follow these syntax:

1)create replica server instance

    syntax: let s = server(IP_ADDRESS);
   
    e.g. let s1 = server("127.23.0.2");

         let s2 = server("127.23.0.3");

         let ...

2)create a server list object

    syntax: let rep = [s1, s2, ...., sk];

    e.g. let rep = [s1, s2, s3];

3)create a fault graph

    syntax: let ft = FailureGraph(REPLICA_LIST);

    e.g. let ft = FailureGraph(rep)

4)The primitive function

  a)rankRCG:

    syntax: let list = RankRCG(FAULT_GRAPH, k, METRIC); //here metric can be either SIZE of PROB

    e.g. let list = RankRCG(ft, 3, "PROB");
  
  b)rankNode:

    syntax: let list = RankNode(FAULT_GRAPH,k);

    e.g. let list = RankNode(ft, 5);
  
  c)failProb:

    syntax: let n = FailProb(FAULT_GRAPH);

    e.g. let n = FailProb(ft);

  d)recRep:

    syntax: let list = RecRep(REPLICA_LIST, k);

    e.g. let list = RecRep(rep, 5);

save the script file and name it as input.src, then run the parser and solver to execute.

Or you can also use the script under example folder to run them separately.

run_example.sh <example_file_name>

