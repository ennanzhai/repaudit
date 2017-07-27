// FaultGraph implementation functions
#ifndef _FAULTGRAPH
#define _FAULTGRAPH
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <exception>
#include <iostream>
#include <algorithm>

#include "utilities.hpp"

#define MAP_FACTOR 100

using namespace std;

// server node class
struct Node{
  string name;
  string description;
  string ip;
  double faultProbability;
  // overload operator for hashing, due to the approximate accuracy of double variable, 
  // we do not compare faultProbability here
  bool operator==(const Node& n) const{
    return this->name == n.name && 
        this->description == n.description && 
        this->ip == n.ip;
  }

  bool operator<(const Node& rls) const{
	// order by ip address
	 return this->get_ip_int() < rls.get_ip_int();
  }
  friend ostream &operator<<( ostream &output, const Node &node){ 
     output << "name: " << node.name << ", ip: " << node.ip << ", fault probability: "<< node.faultProbability;
     return output;
  }
  int get_ip_int() const{
	istringstream iss(ip);
	string str;
	getline(iss, str, '.');
	unsigned int a = stoi(str);
	getline(iss, str, '.');
	unsigned int b = stoi(str);
	getline(iss, str, '.');
	unsigned int c = stoi(str);
	getline(iss, str, '.');
	unsigned int d = stoi(str);
	unsigned int re = (a << 24) + (b << 16) + (c << 8) + d;
	return re;
  }
};
// std::hash can be injected in namespace std
namespace std
{
    template<> struct hash<Node>
    {
        typedef Node argument_type;
        typedef std::size_t result_type;
        result_type operator()(argument_type const& s) const
        {
            result_type const h1 ( std::hash<std::string>()(s.name) );
            result_type const h2 ( std::hash<std::string>()(s.description) );
            result_type const h3 ( std::hash<std::string>()(s.ip) );
            return (h1 ^ (h2 << 1)) ^ (h3 << 1); // or use boost::hash_combine
        }
    };
}


// path class
// TODO:
// - vector<Node> nodes;
struct Path{
  string src, dst;
  vector<string> route;
  inline bool operator==(const Path& rhs)
  { return this->src == rhs.dst && this->dst == rhs.dst
    && this->route == rhs.route;/* do actual comparison */ }
};

inline ostream& operator<<(ostream& out, const Path& path){
    out << path.src << " ";
    for(auto it = path.route.begin(); it != path.route.end(); it++){
      out<< *it<< " ";
    }
    out<< path.dst << endl;
    return out;
}

// edit by Xi
struct Edge{
  string start, end;
  Edge(){}
  Edge(string s1, string s2): start(s1), end(s2){}
  inline bool operator==(const Edge& edge)
  { return this->start == edge.start && this->end == edge.end;}
};
inline ostream& operator<<(ostream& out, const Edge& edge){
    out << edge.start << "->" << edge.end << endl;
    return out;
}

class FaultGraph
{
public:
  vector<Path> paths;
  vector<Node> nodes;
  vector<Node> replica; // replica ip address
  vector<Node> servers;
  //edit by Xi
  unordered_map<string, unordered_set<string>> adjlist; // strings are names like "S1","Agg1", not ip
  unordered_map<string, Node> ip_to_node;
  unordered_map<string, string> name_to_ip;
  unordered_map<string, int> name_to_id;
  unordered_map<int, int> nodeid_to_cnfid;
  unordered_map<int, int> cnfid_to_nodeid;
  ErrorCode isError;

  int getCNFId(int nodeid);

  //construct complete fault graph
  FaultGraph(string filename);
  ~FaultGraph();

  // init member variables
  void init();

  void setReplica(vector<string> ip_list);
  void setReplicaByNode(vector<Node> nodes);
  
  unordered_map<int, unordered_set<int>> subGraph();
  vector<Path> subPath();
  vector<int> getStartNode();
  
  void genCNF(string filename);
  void genWCNF(string filename, string metric);
  int getMappedWeight(double prob);

  void sortNodeByIP(vector<int>& node_ids);

  Node findNodeByIP(string ip_address);

  //edit by Xi
  void addEdge(Edge edge);
  void addReplica(Node node);
  void addPath(Path path);
  void addNode(Node node);
  void delPath(Path path);

  //edit by Xi
  void genPaths(Path& path, string node);

  static Node parseNode(string line);
  static Path parsePath(string line);

};
#endif
