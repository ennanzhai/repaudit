#include "../include/FaultGraph.hpp"

// construct complete fault graph
FaultGraph::FaultGraph(string filename){
  string line;
  ifstream file(filename);
  if(file.is_open()){
    init();
    while(getline(file, line)/* && isError == OK*/){
      string command = trimComment(line);
      if(command.empty()){
        continue;
      }
      else if(command[0] == '<'){
        //** edit by Xi
        Path path = parsePath(command);
        addPath(path);
        string start = path.src;
        for(string node: path.route){
          adjlist[start].insert(node);
          start = node;
        }
        adjlist[start].insert(path.dst);
        //**
      }else if(command[0] == '{'){
        Node node = parseNode(command);
        if (name_to_id.find(node.name) == name_to_id.end()) {
          addNode(node);
          if(node.name.find("S") != string::npos){
            servers.push_back(node);
          }
          ip_to_node[node.ip] = node;
          name_to_ip[node.name] = node.ip;
          name_to_id[node.name] = (int)nodes.size();
        }
      }else{
        //error
        isError = LINE_ILLEGAL;
      }
    }
  }
  else{
    isError = FILE_CANNOT_OPEN;
  }
  file.close();
}

FaultGraph::~FaultGraph(){

}

void FaultGraph::init(){
  this->isError = OK;
  this->paths.clear();
  this->replica.clear();
}

void FaultGraph::setReplica(vector<string> ip_list){
  replica.clear();
  unordered_set<string> ip_set(ip_list.begin(), ip_list.end());
  for(auto node = nodes.begin(); node != nodes.end(); node++){
    if(ip_set.count(node->ip) > 0 ){
      // find corresponding replica node
      replica.push_back(*node);
    }
  }
}

//*** edit by Xi
void FaultGraph::addEdge(Edge edge){
  if(adjlist.find(edge.end) == adjlist.end()){
    cout << "No existing path in the network. Cannot add an edge from "
      << edge.start << " to " << edge.end << "." << endl;
    return;
  }
  adjlist[edge.start].insert(edge.end);
  Path res;
  res.src = edge.start;
  genPaths(res, edge.end);
}

void FaultGraph::genPaths(Path& path, string node){
    if(adjlist.find(node) == adjlist.end()){
      path.dst = node;
      paths.push_back(path);
      return;
    }
    path.route.push_back(node);
    for(string adj: adjlist[node]){
      genPaths(path, adj);
    }
    path.route.pop_back();
}

void FaultGraph::addReplica(Node node){
  replica.push_back(node);
}
//***

void FaultGraph::addNode(Node node){
  nodes.push_back(node);
}

void FaultGraph::addPath(Path path){
  paths.push_back(path);
}

void FaultGraph::delPath(Path target){
  for(auto path = paths.begin(); path != paths.end(); path++){
    if(target == *path){
      paths.erase(path);
      return ;
    }
  }
  cout<<"delPath(): Not a valid path."<<endl;
}
void FaultGraph::setReplicaByNode(vector<Node> nodes){
  replica = nodes;
}

int FaultGraph::getCNFId(int nodeid) {
  if (nodeid_to_cnfid.find(nodeid) != nodeid_to_cnfid.end()) {
    return nodeid_to_cnfid[nodeid];
  }
  int cnt = nodeid_to_cnfid.size();
  nodeid_to_cnfid[nodeid] = cnt + 1;
  cnfid_to_nodeid[cnt + 1] = nodeid;
  return cnt + 1;
}

void FaultGraph::genCNF(string filename){
  unordered_map<string, string> ip_to_name;
  vector<vector<int>> cnf;
  unordered_set<string> replica_name;
  unordered_set<int> cnf_nodes;
  
  nodeid_to_cnfid.clear();
  cnfid_to_nodeid.clear();
  int cnfid;

  for(auto it = replica.begin(); it != replica.end(); it++){
    replica_name.insert(it->name);
  }
  for(auto it = paths.begin(); it != paths.end(); it++){
    // current path leads to Internet
    // && src of current path is selected by replica
    if(it->dst == "Internet" && replica_name.count(it->src) > 0){
      // store current path *it to clause
      vector<int> clause;
      // push first element to clause
      cnfid = getCNFId(name_to_id[it->src]);
      clause.push_back(cnfid);
      cnf_nodes.insert(cnfid);

      // push rest elements to clause
      for(auto route = it->route.begin(); route != it->route.end(); route++){
        cnfid = getCNFId(name_to_id[*route]);
        clause.push_back(cnfid);
        cnf_nodes.insert(cnfid);
      }
      cnf.push_back(clause);
    }
  }

  // convert vector to cnf text file
  // create file
  ofstream file;
  file.open(filename);
  // p cnf (num of variables) (num of clauses)
  file << "p cnf " << cnf_nodes.size() << " " << cnf.size() << endl;
  for(auto clause = cnf.begin(); clause != cnf.end(); clause++){
    for(auto item = clause->begin(); item != clause->end(); item++){
      file << *item << " ";
    }
    // write terminate 0 marker
    file << "0" <<endl;
  }
  file.close();
}

//generate WCNF file from the faultgraph
void FaultGraph::genWCNF(string filename, string metric) {
  unordered_map<string, string> ip_to_name;
  vector<vector<int>> cnf;
  unordered_set<string> replica_name;
  unordered_set<int> cnf_nodes;
  
  nodeid_to_cnfid.clear();
  cnfid_to_nodeid.clear();
  int cnfid;

  for(auto it = replica.begin(); it != replica.end(); it++){
    replica_name.insert(it->name);
  }
  for(auto it = paths.begin(); it != paths.end(); it++){
    // current path leads to Internet
    // && src of current path is selected by replica
    if(it->dst == "Internet" && replica_name.count(it->src) > 0){
      // store current path *it to clause
      vector<int> clause;
      // push first element to clause
      cnfid = getCNFId(name_to_id[it->src]);
      clause.push_back(cnfid);
      cnf_nodes.insert(cnfid);

      // push rest elements to clause
      for(auto route = it->route.begin(); route != it->route.end(); route++){
        cnfid = getCNFId(name_to_id[*route]);
        clause.push_back(cnfid);
        cnf_nodes.insert(cnfid);
      }
      cnf.push_back(clause);
    }
  }

  // convert vector to cnf text file
  // create file
  ofstream file;
  file.open(filename);
  // p wcnf (num of variables) (num of clauses) (max marker)
  int cnf_node_cnt = cnf_nodes.size();
  int sum_weight = 0;
  if (metric == "SIZE") {
    sum_weight = cnf_node_cnt;
  }else {
    for (int node : cnf_nodes) {
      sum_weight += getMappedWeight(nodes[node - 1].faultProbability);
    }
  }
  file << "p wcnf " << cnf_node_cnt << " " << cnf.size() + cnf_node_cnt << " " << sum_weight + 10 << endl;
  for(auto clause = cnf.begin(); clause != cnf.end(); clause++){
    file << sum_weight + 10 << " ";
    for(auto item = clause->begin(); item != clause->end(); item++){
      file << *item << " ";
    }
    // write terminate 0 marker
    file << "0" <<endl;
  }
  //output all variables
  for (int node : cnf_nodes) {
    if (metric == "SIZE") {
      file << "1 " << -1 * node << " 0" << endl;
    }else if (metric == "PROB") {
      file << getMappedWeight(nodes[node - 1].faultProbability) << " " 
        << -1 * node << " 0" << endl;
    }
  }
  file.close();
}

//get the mapped integer weight based on fault probability
int FaultGraph::getMappedWeight(double prob) {
  return (int)((1 - prob) * MAP_FACTOR);
}

// find node by ip address
Node FaultGraph::findNodeByIP(string ip_address){
  /*
  for (auto it = nodes.begin(); it != nodes.end(); ++it)
  {
    if (it->ip == ip_address)
    {
      return *it;
    }
  }
  */
  if(ip_to_node.find(ip_address) != ip_to_node.end()) 
    return ip_to_node[ip_address];
  string errorMsg;
  errorMsg = "No node found by ip: ";
  errorMsg.append(ip_address);
  cout<<errorMsg<<endl;
  return Node();
}

Path FaultGraph::parsePath(string line){
  Path res;
  line = line.substr(1, line.size() - 3);
  istringstream iss(line);
  string str;
  while (iss >> str) {
    size_t pos = str.find('=');
    string key = str.substr(0, pos);
    string value = str.substr(pos + 1);
    //remove " on two ends
    value = value.substr(1, value.size() - 2);
   // if (name_to_id.find(value) == name_to_id.end()) {
    //  cout << value << endl;
    //}
    if (key == "src") {
      res.src = value;
    }else if (key == "dst") {
      res.dst = value;
    }else if (key == "route") {
      istringstream is(value);
      string tmp;
      while (getline(is, tmp, ',')) {
        res.route.push_back(tmp);
      }
    }
  }
  return res;
}


Node FaultGraph::parseNode(string line){
  vector<string> tmp;
  line = line.substr(1, line.size() - 2);
  istringstream iss(line);
  string str;
  while (getline(iss, str, ',')) {
    size_t pos = str.find('"');
    if (pos != string::npos) {
      str = str.substr(pos + 1, str.size() - pos - 2);
    }
    tmp.push_back(str);
  }
  Node res;
  res.name = tmp[0];
  res.description = tmp[1];
  res.ip = tmp[2];
  res.faultProbability = atof(tmp[3].c_str());
  return res;
}


vector<Path> FaultGraph::subPath(){
	vector<Path> res;
	unordered_set<string> replica_name;
	for(auto it = replica.begin(); it != replica.end(); it++){
		replica_name.insert(it->name);
	}
  
	for(auto it = this->paths.begin(); it != this->paths.end(); it++){
		if(replica_name.count(it->src) > 0 && it->dst == "Internet"){
			res.push_back(*it);
		}	
	}

	return res;
}

vector<int> FaultGraph::getStartNode(){
	unordered_set<int> res_set;
	vector<int> res;
	vector<Path> paths = this->subPath();
	for(auto path = paths.begin(); path != paths.end(); path++){
		res_set.insert(name_to_id[path->route.back()]);
	}
	for(auto it = res_set.begin(); it != res_set.end(); it++){
		res.push_back(*it);
	}
	return res;
}

/// return value:
// key - node id
// value - set of succeeding node ids.
unordered_map<int, unordered_set<int>> FaultGraph::subGraph(){
	unordered_map<int, unordered_set<int>> subgraph;
	vector<Path> subpaths = this->subPath();
	
    // DEBUG
 // for(auto it = subpaths.begin(); it!= subpaths.end(); it++){
 //   cout<<*it;
 // }

	for(auto path = subpaths.begin(); path != subpaths.end(); path++){
		vector<string> route = path->route; // copy route 
		reverse(route.begin(), route.end());
		
		// push back the src node
		route.push_back(path->src);

		string first_node_name = *route.begin();
		int prev_node_id = this->name_to_id[first_node_name];
		for(auto it = route.begin() + 1; it != route.end(); it++){
			int curr_node_id = this->name_to_id[*it];
			// prev node id do not exist in subgraph
			if(subgraph.find(prev_node_id) == subgraph.end()){
				subgraph[prev_node_id] = unordered_set<int>();
			}
			// set will erase duplicate
			subgraph[prev_node_id].insert(curr_node_id);
			prev_node_id = curr_node_id;
		}
		// store id of last node
		if(subgraph.find(prev_node_id) == subgraph.end()){
			subgraph[prev_node_id] = unordered_set<int>();
		}
	}
	return subgraph;
}


void FaultGraph::sortNodeByIP(vector<int>& node_ids){
	auto& nodes = this->nodes;
	sort(node_ids.begin(), node_ids.end(),[&nodes](int const& lhs, int const& rhs){
		return nodes[lhs] < nodes[rhs];
	});
}
