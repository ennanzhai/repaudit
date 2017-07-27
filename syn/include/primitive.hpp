// Primitive functions of FaultGraph
// Interfaces provided for upper layer - Language parser
#ifndef _PRIMITIVE
#define _PRIMITIVE

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <sstream>
#include <cstdlib>
#include <iostream>
#include <cstdarg>
#include <initializer_list>
#include <fstream>
#include <ctime>
#include <algorithm>
#include <chrono>
#include "FaultGraph.hpp"

using namespace std;

/**
 * The first input is a IP address, i.e., x, whose type is string. 
The second input is the DepDB, i.e., the IP path information fstream.
The function searches the input IP address, x, in the DepDB, 
and then returns a Node value. All the information such as name and failure
probability could be obtained from this value.
 **/
Node server(string varName, string ip_address, string filename = "topo.conf");


/**
 * Exactly the same as the above function. Just different function name :) . 
The first input is a IP address, i.e., x, whose type is string. 
The second input is the DepDB, i.e., the IP path information fstream.
The function searches the input IP address, x, in the DepDB, 
and then returns a Node value. All the information such as name and failure
probability could be obtained from this value.
 **/
Node _switch(string varName, string ip_address, string filename = "topo.conf");

Edge edge(Node start, Node end);


void goal(const FaultGraph fg, double prob, vector<Node>& posConstr, vector<Node>& negConstr, string scheme);

void goalAddNode(const FaultGraph fg, double goal_prob, const vector<Node> &posConstr, const vector<Node> &negConstr);

void goalAddPath(const FaultGraph fg, double goal_prob, const vector<Node> &posConstr, const vector<Node> &negConstr);

double binary_search_in_num_of_edges(double goal_prob, const FaultGraph &fg, const string &first_replica_name,
                   const unordered_set<string> &second_level_names);

double get_fail_prob_after_add_edge(const FaultGraph &fg, const unordered_set<string> &second_level_names,
                                    const string &first_replica_name, int k);

double binary_search_in_num_of_replicas(double goal_prob, const FaultGraph &fg, const unordered_set<Node>& candidates);

double get_fail_prob_after_add_replica(const FaultGraph &fg, const unordered_set<Node>& candidates, int k);
/**
 * This function generate a fault graph for a given replication deployment.

The first input parameter is a vector containing multiple Node variables. 
The vector in principle means a given replication deployment. Each element 
in this vector should be a replica server. 

The second input parameter is the path for DepDB, i.e., the IP path
information fstream.

After running this function, it should generate a CNF fstream. In fact, we
do not need to consider any weight so far. Thus, this CNF is a standard 
CNF.

In order to enable the following function, e.g., miniCostSAT, we may
need a shell command to transfer a CNF to WCNF, i.e., weighted CNF.
But this is quite straightforward.

In principle, we actually use CNF to represent a fault graph.

 **/
FaultGraph faultGraph(string varName, const vector<Node> replica, string filename = "topo.conf");

/**
 * vector<vector<Node>> RankRCG(fstream cnf, int k, string metric)
This function generates a list ranking the top k RCG in the given cnf
according to metric.

In this function, the input is a cnf, which should be transfered to a wcnf.

How to implement RankRCG() is still not easy. Let's see the following 
pseudo-code:

vector<vector<Node>> RankRCG(fstream cnf, int k, string metric):
  vector<vector<Node>> n;
  
  if metric = SIZE then: // rank RCGs based on sizes
    generate wcnf and weights are 1;
  else if metric = FailProb: // rank RCGs based on failure probabilities
    generate wcnf and weights are generated based on failure probability
  
  for i = 1 to k:
    vector<Node> r <- miniCostSAT(wcnf);
    n.append(r);
    r <- negation(r);
    wcnf.append(r)
    
  return n;  

 **/
vector<vector<Node>> rankRCG(const FaultGraph fg, int k, string metric);

/**
 double FailProb(fstream cnf)

This function computes failure probability for the given cnf.

The easiest way I want to implement this function is to use RankRCG().
For instance, we run RankRCG(cnf, 5, SIZE), thus getting a list of risk
groups, per group corresponding to several nodes.

We could use the following way to compute the failure probability:
Pr(S1) + ... + Pr(S10) - Pr(S1) * Pr(S2) - ... - Pr(S9) * Pr(S10) +
  Pr(S1) * Pr(S2) * Pr(S3) + ... + Pr(S8) * Pr(S9) * Pr(S10) - ...

However, it seems the above way is not fast. Thus, we still need to drop
some accuracy. I am planning to use Pr(S1) + ... + Pr(S10) to fast 
obtain failure probability.

Alternatively, I proposed another way using SAT model counter, but it seems
that solution is also not fast. Let me think whether we really need the
SAT model counter-based approach.

 **/
double failProb(const FaultGraph fg, int k = 10);


/**
 * vector<Node> RankNode(fstream cnf, string metric)

This function generates a relative importance value for each node.

- First, we use RankRCG() to get top-k RCGs.
- Then, we
 **/
vector<Node> rankNode(const FaultGraph fg, int k = 10);

// edit by Xi
void addEdge(FaultGraph& fg, Edge edge);
void addNdoe(FaultGraph& fg, Node node);

FaultGraph addPath(const FaultGraph fg, string path);

FaultGraph delPath(const FaultGraph fg, string path);

vector<string> overlapPath(string path1, string path2);

vector<vector<Node>> recRep(vector<Node> sList,
  int num_replica, string filename = "topo.conf");
vector<vector<Node>> recRep(vector<Node> sList,
                                string filename, 
                                int num_replica, int size = 1);

// we have to declare and implement here since its a template function
template<class... ArgsT>
vector<Node> genVector(ArgsT... ts){
  vector<Node> re= {(Node)ts...};
  return re;
}

// edit by Xi
void inputFile(string varName);

void copyInputFile(ofstream& fout);

void print(double n);
void print(char * s);
void print(vector<Path> t);
void print(vector<vector<Node>> t);
void print(vector<Node> t);
void print(Node t);
void print(Path t);
void print(const FaultGraph fg);

#endif
