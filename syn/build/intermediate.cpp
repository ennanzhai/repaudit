// Auto generated file
// Generated by Left-right parser
#include "../include/primitive.hpp"
using namespace std;

int main(int argc, char const *argv[])
{
	inputFile("../example/./topo_a/RAL_script");
	auto s1 = server("s1", "172.28.228.1");
	auto s2 = server("s2", "172.28.228.2");
	auto s3 = server("s3", "172.28.228.3");
	auto s4 = server("s4", "172.28.228.4");
	auto rep = genVector(s1, s2, s3, s4);
	auto ft = faultGraph("ft", rep);
	auto posConstr = genVector();
	auto negConstr = genVector();
	goal(ft, 0.009, posConstr, negConstr, "AddPath");
	return 0;
}
