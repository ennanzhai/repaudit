#include "../include/primitive.hpp"
/**
 * The first input is a IP address, i.e., x, whose type is string. 
 The second input is the DepDB, i.e., the IP path information file.
 The function searches the input IP address, x, in the DepDB, 
 and then returns a Node value. All the information such as name and failure
 probability could be obtained from this value.
 **/

unordered_map<string, string> node_var_mp;
string inputFileName;
string faultGraphName;
string outputFileName = "../example/output.src";
FaultGraph defaultFg("topo.conf");


Node server(string varName, string ip_address, string filename){
  return _switch(varName, ip_address, filename);
}

/**
 * Exactly the same as the above function. Just different function name :) . 
 The first input is a IP address, i.e., x, whose type is string. 
 The second input is the DepDB, i.e., the IP path information file.
 The function searches the input IP address, x, in the DepDB, 
 and then returns a Node value. All the information such as name and failure
 probability could be obtained from this value.
 **/
Node _switch(string varName, string ip_address, string filename){
  Node node;
  if(filename == "topo.conf"){
    node = defaultFg.findNodeByIP(ip_address);
  }else{
    FaultGraph fg(filename);
    node = fg.findNodeByIP(ip_address);
  }
  node_var_mp[node.name] = varName; 
  return node;
}

// edit by Xi
Edge edge(Node startNode, Node endNode){
  return Edge(startNode.name, endNode.name);
}

void inputFile(string varName){
  inputFileName = varName;
}

vector<Node> genNodes(Node node, int cnt){
  vector<Node> res;
  string name = node_var_mp[node.name];
  string ip = node.ip;
  string prefix;
  int pos, len = name.size();
  for(pos = 0; pos < len; ++pos){
    if(isdigit(name[pos])) break;
    prefix += name[pos];
  }
  // check
  for(int i = pos; i < len; ++i){
    if(!isdigit(name[i])){
      cout << "Invalid node name! " << endl;
      return res;
    }
  }
  res.push_back(node);
  int idx = stoi(name.substr(pos));
  stringstream ss(ip);
  char dot;
  int a,b,c,d;
  ss >> a >> dot >> b >> dot >> c >> dot >> d;
  string ipPrefix = to_string(a) + '.' + to_string(b) + '.' + to_string(c) + '.';
  for(int i = 1; i < cnt; ++i){
    string newVar = prefix + to_string(idx + i);
    string newIp = ipPrefix + to_string(d + i);
    Node newNode = _switch(newVar, newIp);
    res.push_back(newNode);
  }
  return res;
}
// posConstr: the result edges must include all of the nodes in posConstr
// negConstr: don't add edge to nodes in negConstr
// scheme: addPath / addNode; output: addEdge
void goal(const FaultGraph old_fg, double goal_prob, vector<Node>& posConstr, vector<Node>& negConstr, string scheme) {
  cout << ">>> the goal fail probability " << goal_prob << endl;
  cout << ">>> before any changes, current fail probability = " << failProb(old_fg) << endl;
  if (failProb(old_fg) < goal_prob) {
    cout << ">>> already satisfying goal. return" << endl;
    return;
  } else {
    cout << ">>> need some work to be done to get to that goal." << endl;
  }

  if(scheme == "AddPath"){
      goalAddPath(old_fg, goal_prob, posConstr, negConstr);
  }else if(scheme == "AddNode"){
      goalAddNode(old_fg, goal_prob, posConstr, negConstr);
  }else{
    cout << "Invalid scheme! Please input AddPath or AddNode!" << endl;
  }
}

void goalAddPath(const FaultGraph fg_input, double goal_prob, const vector<Node> &posConstr, const vector<Node> &negConstr) {
    FaultGraph fg = fg_input;

    chrono::steady_clock::time_point time_begin = chrono::steady_clock::now();

    string first_replica_name = fg.replica[0].name; // only need first
    // positive constraint handle
    for (auto node : posConstr) {
      fg.addEdge(Edge(first_replica_name, node.name));
    }
//    cout << "debug: after ensuring positive constraint, fail probability = " << failProb(fg) << endl;
    unordered_set<string> second_level_names; // type 2 of nodes second levels; that is the set that we are going to try


    unordered_set<string> second_level_names_for_replicas; // type 1 of nodes
    for (auto replica : fg.replica) {
      for (auto second_level_node_used_by_replica: fg.adjlist[replica.name]) {
        second_level_names_for_replicas.insert(second_level_node_used_by_replica);
      }
    }
    // find aggregator / TOR for [second_level_names]
    for (auto name_to_children_names : fg.adjlist) {
      string name = name_to_children_names.first;
      if (name.find("S") == string::npos) {
        continue; // cout << name << " is not a server. skip" << endl;
      }

  //      cout << name << " is good. add its children." << endl;
      for (auto next_level_name : name_to_children_names.second) { // names of children
        if (second_level_names_for_replicas.find(next_level_name) != second_level_names_for_replicas.end()) {
          continue; // cout << "debug: 2nd level node [" << next_level_name << "] is already connected by some replica, skip"<< endl;
        }
        second_level_names.insert(next_level_name);
      }
    }

    // handle negative constrain

    for (auto neg_constr_node : negConstr)
      second_level_names.erase(neg_constr_node.name); // can be put in when [second_level_names] is added

    chrono::steady_clock::time_point time_end = chrono::steady_clock::now();
    std::cout << ".....Finish handling paths.. Taking (sec) = "
            << (chrono::duration_cast<chrono::microseconds>(time_end - time_begin).count()) / 1000000.0
            << endl; 
    time_begin = chrono::steady_clock::now(); // reset begin timeer

    // try all element to see if it satisfy fail prob goal
    double best_prob = get_fail_prob_after_add_edge(fg, second_level_names, first_replica_name,
                                                    second_level_names.size());
    if (best_prob >= goal_prob) {
      cout << ">>> the best fail prob we can get is " << best_prob << ", still not enough for the goal fail probability: "
           << goal_prob << endl;
      return;
    }
    // binary search, search from adding 0 (no element) to adding all elements

    int num_edge_needed_without_postconstr = binary_search_in_num_of_edges(goal_prob, fg, first_replica_name, second_level_names);
    double achieved_fail_prob = get_fail_prob_after_add_edge(fg, second_level_names, first_replica_name, num_edge_needed_without_postconstr);
    int num_edge_needed_with_postconstr = num_edge_needed_without_postconstr + posConstr.size();
    cout << ">>> the best fail prob we can get is " << achieved_fail_prob
         << ", which meets goal fail probability " << goal_prob << endl;

    cout << "\t  the number of edge needed adding is: " << num_edge_needed_with_postconstr << endl;
    cout << "\t  the edges needed adding are: " << endl;
    auto it = second_level_names.begin(); // name of second level name


    time_end = chrono::steady_clock::now();
    std::cout << ".....Time after paths, finished binary search (sec) = "
              << (chrono::duration_cast<chrono::microseconds>(time_end - time_begin).count()) / 1000000.0
              << endl;
    // TODO maximum 8 edges per replica nodes; also check if # of edges needed > 8 * replica.size()
    // TODO printout "RAL code" instead of just edge with node names
    ofstream output_ral_program;
    output_ral_program.open(outputFileName, fstream::out | fstream::trunc);
    copyInputFile(output_ral_program);

    int edge_idx = 0;
    int switch_idx = 0;
    string first_replica_var = node_var_mp[first_replica_name];

    for (int i = 0; i < num_edge_needed_without_postconstr && it != second_level_names.end(); i++, it++) {
      string node_var_name;
      if(node_var_mp.find(*it) == node_var_mp.end()){
        output_ral_program << "let newSwitch" << ++switch_idx << " = switch(\"" << fg.name_to_ip[*it] << "\");" << endl;
        node_var_name = "newSwitch" + to_string(switch_idx);
      }else{
        node_var_name = node_var_mp[*it];
      }
      cout << "\t  edge: [" << first_replica_var << " -> " << node_var_name << "]" << endl;
      output_ral_program << "let e" << ++edge_idx << " = edge(" << first_replica_var << ", " << node_var_name << ");" << endl;
      output_ral_program << "AddEdge(" << faultGraphName << ", e" << edge_idx << ");" << endl;
    }

    if (posConstr.size() > 0) {
      for (auto node : posConstr) {
        cout << "\t  edge: [" << first_replica_var << " -> " << node_var_mp[node.name] << "] (positive constrain)" << endl;
        output_ral_program << "let e" << ++edge_idx << " = edge(" << first_replica_var << ", " << node_var_mp[node.name] << ");" << endl;
        output_ral_program << "AddEdge(" << faultGraphName << ", e" << edge_idx << ");" << endl;
      }
    }
    cout << ">>> Generating RAL program to \"" << outputFileName << "\"" << endl;
    output_ral_program.close();
}

void goalAddNode(const FaultGraph fg_input, double goal_prob, const vector<Node> &posConstr, const vector<Node> &negConstr) {
    FaultGraph fg = fg_input;
    chrono::steady_clock::time_point time_begin = chrono::steady_clock::now();
    unordered_set<Node> candidates(fg.servers.begin(), fg.servers.end());
    for(auto server: posConstr){
      fg.addReplica(server);
    }
    //cout << "debug: after ensuring positive constraint, fail probability = " << failProb(fg) << endl;

    for(auto server: fg.replica){
      candidates.erase(server);
    }

    for (auto server: negConstr){
      candidates.erase(server);
    }
    chrono::steady_clock::time_point time_end = chrono::steady_clock::now();
    std::cout << ".....Finish handling nodes.. Taking (sec) = "
            << (chrono::duration_cast<chrono::microseconds>(time_end - time_begin).count()) / 1000000.0
            << endl; 
    time_begin = chrono::steady_clock::now(); // reset begin timeer

    double best_prob = get_fail_prob_after_add_replica(fg, candidates, candidates.size());
    if (best_prob >= goal_prob) {
      cout << ">>> the best fail prob we can get is " << best_prob << ", still not enough for the goal fail probability: "
           << goal_prob << endl;
      return;
    }


    int num_replica_needed_without_postconstr = binary_search_in_num_of_replicas(goal_prob, fg, candidates);
    double achieved_fail_prob = get_fail_prob_after_add_replica(fg, candidates, num_replica_needed_without_postconstr);
    int num_replica_needed_with_postconstr = num_replica_needed_without_postconstr + posConstr.size();
    cout << ">>> the best fail prob we can get is " << achieved_fail_prob
         << ", which meets goal fail probability " << goal_prob << endl;

    cout << "\t  the number of replica needed adding is: " << num_replica_needed_with_postconstr << endl;
    cout << "\t  the nodes needed adding are: " << endl;


    time_end = chrono::steady_clock::now();
    std::cout << ".....Time after nodes, finished binary search (sec) = "
              << (chrono::duration_cast<chrono::microseconds>(time_end - time_begin).count()) / 1000000.0
              << endl;
    ofstream output_ral_program;
    output_ral_program.open(outputFileName, fstream::out | fstream::trunc);
    copyInputFile(output_ral_program);

    auto it = candidates.begin();
    int switch_idx = 0;

    for (int i = 0; i < num_replica_needed_without_postconstr && it != candidates.end(); i++, it++) {
      
      string node_var_name;
      string node_name = (*it).name;
      if(node_var_mp.find(node_name) == node_var_mp.end()){
        output_ral_program << "let newSwitch" << ++switch_idx << " = switch(\"" << fg.name_to_ip[node_name] << "\");" << endl;
        node_var_name = "newSwitch" + to_string(switch_idx);
      }else{
        node_var_name = node_var_mp[node_name];
      }
      cout << "\t  replica: [" << node_var_name << "]" << endl;
      output_ral_program << "AddNode(" << faultGraphName << ", " << node_var_name << ");" << endl;
    }

    if (posConstr.size() > 0) {
      for (auto node : posConstr) {
        cout << "\t  replica: [" << node_var_mp[node.name] << "] (positive constrain)" << endl;
        output_ral_program << "AddNode(" << faultGraphName << ", " << node_var_mp[node.name] << ");" << endl;
      }
    }
    cout << ">>> Generating RAL program to \"" << outputFileName << "\"" << endl;
    output_ral_program.close();
}

// reference: https://discuss.leetcode.com/topic/23597/short-c-answer-and-minimize-api-calls
double binary_search_in_num_of_edges(double goal_prob, const FaultGraph &fg, const string &first_replica_name,
                     const unordered_set<string> &second_level_names) {
  int lower_bound = 1;
  int upper_bound = second_level_names.size();
  //cout << "DEBUG: trying between " << lower_bound << " ~ " << upper_bound << " edges" << endl;
  while (lower_bound < upper_bound) {
    int mid = lower_bound + (upper_bound - lower_bound) / 2;
    double fail_prob_in_test = get_fail_prob_after_add_edge(fg, second_level_names, first_replica_name, mid);
    bool satisfy_goal = (fail_prob_in_test < goal_prob);
    if (!satisfy_goal) // not satisfying goal
      lower_bound = mid + 1;
    else
      upper_bound = mid;
  }
  return lower_bound;
}

double get_fail_prob_after_add_edge(const FaultGraph &fg, const unordered_set<string> &second_level_names,
                                    const string &first_replica_name, int k) { // adding k edges
  FaultGraph test_fg = fg;
  auto it = second_level_names.begin(); // name of second level name
//  cout << "==============\ndebug: trying k = " << k << endl;
  for (int i = 0; i < k && it != second_level_names.end(); i++, it++) {
    Edge newEdge(first_replica_name, *it);
//    cout << "debug: adding edge " << newEdge;
//    cout << "\tdebug: before, fg_edge fail prob is " << failProb(test_fg) << '\n';
    test_fg.addEdge(newEdge);
//    cout << "\tdebug: after, fg_edge fail prob is " << failProb(test_fg) << '\n';
  }
  return failProb(test_fg);
}

double binary_search_in_num_of_replicas(double goal_prob, const FaultGraph &fg, const unordered_set<Node>& candidates) {
  int lower_bound = 1;
  int upper_bound = candidates.size();
  //cout << "DEBUG: trying between " << lower_bound << " ~ " << upper_bound << " replicas" << endl;
  while (lower_bound < upper_bound) {
    int mid = lower_bound + (upper_bound - lower_bound) / 2;
    double fail_prob_in_test = get_fail_prob_after_add_replica(fg, candidates, mid);
    bool satisfy_goal = (fail_prob_in_test < goal_prob);
    if (!satisfy_goal) // not satisfying goal
      lower_bound = mid + 1;
    else
      upper_bound = mid;
  }
  return lower_bound;
}

double get_fail_prob_after_add_replica(const FaultGraph &fg, const unordered_set<Node>& candidates, int k) { // adding k edges
  FaultGraph test_fg = fg;
  //cout << "==============\ndebug: trying k = " << k << endl;
  auto it = candidates.begin();
  for (int i = 0; i < k && it != candidates.end(); i++, it++) {
    //cout << "debug: adding replica " << *it;
    //cout << "\tdebug: before, fg_edge fail prob is " << failProb(test_fg) << '\n';
    test_fg.addReplica(*it);
    //cout << "\tdebug: after, fg_edge fail prob is " << failProb(test_fg) << '\n';
  }
  return failProb(test_fg);
}
/**
 * This function generate a fault graph for a given replication deployment.

 The first input parameter is a vector containing multiple Node variables. 
 The vector in principle means a given replication deployment. Each element 
 in this vector should be a replica server. 

 The second input parameter is the path for DepDB, i.e., the IP path
 information file.

 After running this function, it should generate a CNF file. In fact, we
 do not need to consider any weight so far. Thus, this CNF is a standard 
 CNF.

 In order to enable the following function, e.g., miniCostSAT, we may
 need a shell command to transfer a CNF to WCNF, i.e., weighted CNF.
 But this is quite straightforward.

 In principle, we actually use CNF to represent a fault graph.

 **/

FaultGraph faultGraph(string varName, vector<Node> replica, string filename){
  faultGraphName = varName;
  FaultGraph fg = FaultGraph(filename);
  fg.setReplicaByNode(replica);
  return fg;
}


/*read the last line of maxino solver result
 */
vector<int> readMaxinoResult(string filename) {
  ifstream fin(filename);
  string line, last;
  while (getline(fin, line)) {
    last = line;
  }
  fin.close();
  if (last == "UNSATISFIABLE") {
    //return {}; // CHANGED THIS LINE
	return vector<int>();
  }
  int i = 0;
  string str;
  istringstream iss(last);
  vector<int> res;
  while (iss >> str) {
    if (i > 0) {
      res.push_back(atoi(str.c_str()));
    }
    i++;
  } 
  return res;
}

/*generate a new wcnf file adding a new clause
 */
void genWCNFWithNewClause(string filename, vector<int>& clause) {
  ifstream fin(filename);
  vector<string> output;
  //read the first line
  string line;
  getline(fin, line);
  vector<string> tmp;
  istringstream iss(line);
  string str;
  while (iss >> str) {
    tmp.push_back(str);
  }
  str.clear();
  for (int i = 0; i < (int)tmp.size(); ++i) {
    if (i == 3) {
      str.append(to_string(atoi(tmp[i].c_str()) + 1));
    }else {
      str.append(tmp[i]);
    }
    str.push_back(' ');
  }
  str.pop_back();
  output.push_back(str);
  while (getline(fin, line)) {
    output.push_back(line);
  }
  str.clear();
  str.append(tmp.back());
  str.push_back(' ');
  for (int i = 0; i < (int)clause.size(); ++i) {
    str.append(to_string(clause[i] * -1));
    str.push_back(' ');
  }
  str.push_back('0');
  output.push_back(str);
  fin.close();
  ofstream fout(filename);
  for (string line : output) {
    fout << line << endl;
  }
  fout.close();
}

/**
 * vector<vector<Node>> RankRCG(FILE cnf, int k, string metric)
 This function generates a list ranking the top k RCG in the given cnf
 according to metric.

 In this function, the input is a cnf, which should be transfered to a wcnf.

 How to implement RankRCG() is still not easy. Let's see the following 
 pseudo-code:

 vector<vector<Node>> RankRCG(FILE cnf, int k, string metric):
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
vector<vector<Node>> rankRCG(FaultGraph fg, int k, string metric){
  vector<vector<Node>> rcglist;
  //generate wcnf file
  fg.genWCNF("wcnf_output", metric);
  int errno;
  for (int i = 0; i < k; ++i) {
    //call the Maxino solver
    errno = system("./maxino-2015-k16-static wcnf_output > wcnf_maxino_result");
    //read the solver result
    vector<int> sol = readMaxinoResult("wcnf_maxino_result");
    if (sol.empty()) {
      return rcglist;
    }
    vector<Node> v;
    for (int i = 0; i < (int)sol.size(); ++i) {
      if (sol[i] > 0) {
        int nodeid = fg.cnfid_to_nodeid[sol[i]];
        v.push_back(fg.nodes[nodeid - 1]);
      }
    }
    rcglist.push_back(v);
    genWCNFWithNewClause("wcnf_output", sol);
  }
  return rcglist;
}

/**
  double FailProb(FILE cnf)

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

Algorithm: Use Pr(S1) + ... + Pr(S10) + Pr(Sk)
 **/
double failProb(const FaultGraph fg, int k){
  vector<vector<Node>> nodeSets = rankRCG(fg, k, "SIZE");
  double failProb = 0.0;
  for(auto nodes = nodeSets.begin(); nodes != nodeSets.end(); nodes++){
    double setFailProb = 1.0;
    for(auto node = nodes->begin(); node != nodes->end(); node++){
      setFailProb *= node->faultProbability;
    }
    if(!nodes->empty()){
      failProb += setFailProb;	
    }
  }
  return failProb;
}


/**
 * vector<Node> RankNode(FaultGraph fg, int k)

 This function generates a relative importance value for each node.

 - First, we use RankRCG() to get top-k RCGs. (rank by size)
 - Then, we
 **/
 // TODO: To be tested
 // WARNING: This function assumes there is no duplicate nodes in each clause
 // Say, {A B C A} is not legal cause A is duplicated.
vector<Node> rankNode(const FaultGraph fg, int k){
  // we rank k * 2 clauses to ensure there is enough unique nodes in them
  vector<vector<Node>> nodeClauses = rankRCG(fg, k * 2, "SIZE");
  vector<Node> res;
  // store all unique nodes' failProb
  unordered_map<Node, double> node_failProb;
  
  // for all clauses
  for (auto clause = nodeClauses.begin(); clause != nodeClauses.end(); clause++)
  {
    // init failProb to 1.0
    double failProb_clause = 1.0;

    // for all nodes
    for(auto node = clause->begin(); node != clause->end(); node++){
      failProb_clause *= node->faultProbability;
    }

    // add failProb_clause to each node appeared in current clause
    for(auto node = clause->begin(); node != clause->end(); node++){
      if(node_failProb.find(*node) == node_failProb.end()){
        node_failProb[*node] = .0;
      }
      node_failProb[*node] += failProb_clause;
    }
  }

  // rank the nodes in node_failProb by their failProbs
  // here we assume k is small so just use bubble sort
  // Bubble sort
  Node highestNode;
  while(!node_failProb.empty() && (int)res.size() < k){
    highestNode = (node_failProb.begin())->first;
    for(auto it = node_failProb.begin(); it != node_failProb.end(); it++){
      if(it->second > node_failProb[highestNode]){
        highestNode = it->first;
      }
    }
    // add highestNode to res
    // remove it from hashmap
    res.push_back(highestNode);
    node_failProb.erase(highestNode);
  }
  return res;

}

// edit by Xi
void addEdge(FaultGraph& fg, Edge edge){
  fg.addEdge(edge);
}

void addNode(FaultGraph& fg, Node node){
  fg.addReplica(node);
}

FaultGraph addPath(const FaultGraph fg, string line){
  FaultGraph newFg = fg;
  line = trimComment(line);
  Path path = FaultGraph::parsePath(line);
  newFg.addPath(path);
  return newFg;
}

FaultGraph delPath(const FaultGraph fg, string line){
  FaultGraph newFg = fg;
  line = trimComment(line);
  Path path = FaultGraph::parsePath(line);
  newFg.delPath(path);
  return newFg;
}

vector<string> overlapPath(string path1_str, string path2_str){
  path1_str = trimComment(path1_str);
  path2_str = trimComment(path2_str);
  Path path1 = FaultGraph::parsePath(path1_str);
  Path path2 = FaultGraph::parsePath(path2_str);

  // convert route vector to route set
  unordered_set<string> path1_routes(path1.route.begin(), path1.route.end());
  vector<string> overlapPathStr;
  for(auto route = path2.route.begin(); route != path2.route.end(); route++){
    if(path1_routes.count(*route) > 0){
      overlapPathStr.push_back(*route);
    }
  }
  return overlapPathStr;
}

//Recursive function to generate the recommended replicas
int recRepRecurse(FaultGraph& fg, vector<int>& serverNodes, vector<int>& replicas,
  unordered_map<int, unordered_set<int>>& graph, vector<int> startNodes,
  int num_replica) {
 // cout<<"start nodes: ";
//  for(auto it = startNodes.begin(); it != startNodes.end(); it++){
//    cout<<fg.nodes[*it - 1].name<<",";
//  }
//  cout<<endl;
//  cout <<"num_replica: " << num_replica<<endl;
  if (startNodes.empty() || num_replica == 0) {
//    cout<<"start node is empty, num_replica = "<< num_replica<<endl;
    return num_replica;
  }

  int node = startNodes[0];
  
  if (graph[node].size() == 0) {
    //reach to top of graph
    //randomly select one replica server
    int id = startNodes[rand() % startNodes.size()];
    int n = 0;
    
    while (n < num_replica) {
      size_t i;
      for(i = 0; i < startNodes.size(); i++){
        int startNode_id = startNodes[i];
        if(find(replicas.begin(), replicas.end(), startNode_id) == replicas.end()){
          break;
        }
      }
      if(i == startNodes.size()){
         return num_replica - n;
      }
      while (find(replicas.begin(), replicas.end(), id) != replicas.end()) {
        //the server is already selected before
        id = startNodes[rand() % startNodes.size()];
      }
      replicas.push_back(id);
      n++;
    }
    return 0;
  }
  if (num_replica & 1) {
    //we first randomly select one replica server
    int id = serverNodes[rand() % serverNodes.size()];
    while (find(replicas.begin(), replicas.end(), id) != replicas.end()) {
      //the server is already selected before
      id = serverNodes[rand() % serverNodes.size()];
    }
    replicas.push_back(id);
    num_replica--;
    if(num_replica == 0) {
      return 0;
    }
  }
  //recursively get the replicas in the two halves
  unordered_set<int> next;
  for (int i = 0; i < (int)startNodes.size(); ++i) {
    int curid = startNodes[i];
    for (int nextid : graph[curid]) {
      next.insert(nextid);
    }
  }
  vector<int> nextid;
  for (int id : next) {
    nextid.push_back(id);
  }
  fg.sortNodeByIP(nextid);
  int mid = nextid.size() / 2;
  vector<int> left(nextid.begin(), nextid.begin() + mid);
  vector<int> right(nextid.begin() + mid, nextid.end());
  int rest_replica = recRepRecurse(fg, serverNodes, replicas, graph, left, num_replica / 2);

  rest_replica = recRepRecurse(fg, serverNodes, replicas, graph, right, num_replica / 2 + rest_replica);

  if(rest_replica > 0){ 
    rest_replica = recRepRecurse(fg, serverNodes, replicas, graph, left, rest_replica);
  }
  return rest_replica; //error , not enough space to allocate replica!
  
}

// Algorithm:
//    1. 
// Return: 
//    - server nodes   
vector<vector<Node>> recRep(vector<Node> sList,
  int num_replica, string filename){
  return recRep(sList, filename, num_replica);
}
vector<vector<Node>> recRep(vector<Node> sList,
  string filename, 
  int num_replica, int size){
  FaultGraph fg = FaultGraph(filename);
  fg.setReplicaByNode(sList);  
  // extract all path whose src is contained by sList and dst == 'Internet'
  unordered_map<int, unordered_set<int>> subGraph = fg.subGraph();
  vector<pair<vector<Node>, double>> replicas2prob;
  vector<int> startNodes = fg.getStartNode();
  vector<int> serverNodes;
  for (int i = 0; i < (int)sList.size(); ++i) {
    serverNodes.push_back(fg.name_to_id[sList[i].name]);
  } 
  vector<vector<int>> v;
  srand(time(NULL));
  for (int i = 0; i < size * 10; ++i) {
    vector<int> replicas_id;
    int error_no = recRepRecurse(fg, serverNodes, replicas_id, subGraph, startNodes, num_replica);
    if(error_no != 0){
        cout<<"No enough space to allocate replica."<<endl;
        continue;
    }
    sort(replicas_id.begin(), replicas_id.end());
    bool f = false;
    for (int j = 0; j < (int)v.size(); ++j){
      if (v[j] == replicas_id) {
  //      for (int k = 0; k < v[j].size(); ++k) {
  //         cout << v[j][k] << ":" << replicas_id[k] << ",";
  //      }
  //      cout << endl;
        f = true;
        break;
      }
    }
    if (f) continue;
    v.push_back(replicas_id);
    vector<Node> nodes;
    for (int j = 0; j < (int)replicas_id.size(); ++j) {
      nodes.push_back(fg.nodes[replicas_id[j] - 1]);
    }
    fg.setReplicaByNode(nodes);
    replicas2prob.push_back(make_pair(nodes, failProb(fg)));
  }
  sort(replicas2prob.begin(), replicas2prob.end(),[](pair<vector<Node>, double> const & lhs,
      pair<vector<Node>, double> const& rhs) {return lhs.second < rhs.second;});
  vector<vector<Node>> replicas;
  int cnt = 0;
  for (auto& p : replicas2prob) {
    if (++cnt > size) {
      break;
    }
    replicas.push_back(p.first);
  }
  return replicas;
}

void copyInputFile(ofstream& fout){
  if (fout.is_open())
  {
    ifstream fin(inputFileName);
    if(fin.is_open()){
      string line;
      while(getline(fin, line)){
        if(line.find("Goal") == string::npos || line.find("//") != string::npos){
            fout << line << endl;
        }else{
          fout << "//Implement this statement to output RAL program: " << line << endl;
        }
      }
      fin.close();
    }else{
      cout << "Unable to open the input file" << endl;
    }  
  }
  else cout << "Unable to open the output file" << endl;
}
void print(double n){
  cout<< n <<endl;
}

void print(char * s){
  cout << s << endl;
}
void print(Node t){
	cout << t << endl;
}
void print(Path t){
  cout << t << endl;
}
void print(vector<Path> t){
  for(auto it = t.begin(); it != t.end(); it++){
    print(*it);
  }
}

void print(vector<Node> t){
	for(auto it = t.begin(); it != t.end(); it++){
		print(*it);
	}
}

void print(vector<vector<Node>> t){
  for(auto elem : t){
    print(elem);
    cout<<endl;
  }
}

void print(const FaultGraph fg){
  cout << "FaultGraph: " << endl;
  cout << "******Replicas******" << endl;
  print(fg.replica);
  cout << "******Paths******" << endl;
  print(fg.paths);
}