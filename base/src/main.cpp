/*************************************************************************
	> Author: Wayne Ho
	> Purpose: TODO
	> Created Time: Fri Apr 17 16:32:22 2015
	> Mail: hewr2010@gmail.com 
 ************************************************************************/
#include "argparse/macro-argparse-jquery.hh"
#include "solver/solver.h"
#include "eigen_triangle.h"
#include "triangle_brute_force.h"
#include "triangle_stream.h"
#include "storage/graph_builder.h"
#include "storage/mgraph.h"
#include "report/table_generator.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <ctime>
#include <map>
#include <cstdlib>
using namespace std;
using namespace sae::io;

DEF_ARGUMENT_CLASS(
	Argument,
	string,		content,	"test",		OPTIONAL,	OPT_SLH(-c, --content, "what to say"),
	//int,		number,		1,			REQUIRED,	OPT_LH(-n, "what number is it? "),
	bool,		decorate,	true,		OPTIONAL,	OPT_SLWH(-d, --decorate, true, "decoreate the output")
);


void makeFakeData(int numVertex=10, double p = 0.1) {
	//int numEdge = rand() % (numVertex * numVertex / 3) + numVertex;
    int numEdge = (numVertex * (numVertex - 1)) / 2 * p;
	GraphBuilder<int> graph;
	for (int i = 0; i < numVertex; ++i) graph.AddVertex(i, 0);
	map<pair<int, int>, bool> edges;
	for (int i = 0; i < numEdge; ++i) {
		pair<int, int> edge;
		do {
			int x = rand() % (numVertex - 1);
			int y = rand() % (numVertex - x - 1) + x + 1;
			edge = make_pair(x, y);
		} while (edges.find(edge) != edges.end());
		edges[edge] = true;
		graph.AddEdge(edge.first, edge.second, 0);
		//cout << edge.first << " " << edge.second << endl;
	}
	system("mkdir -p fake");
	graph.Save("./fake/graph");
}


void testTriangle(char* prefix="./fake/graph") {
    // prepare graph
	MappedGraph *graph;
	graph = MappedGraph::Open(prefix);
    // set up table
	TableGenerator table;
	string title[] = {"vertexes", "edges", "Brute Force", "Stream", "Eigen"};
	table.setTitle(vector<string>(title, title + sizeof(title) / sizeof(title[0])));
    vector<string> score;
    score.push_back(toString<int>(graph->VertexCount()));
    score.push_back(toString<int>(graph->EdgeCount()));
	// brute force
    Triangle_Brute_Force bf(graph);
    int bf_cnt = bf.solve();
    cout << "[brute force]\t" << bf_cnt << endl;
    score.push_back(toString<int>(bf_cnt));
	// streaming
	Triangle_Stream stm(50, 1000);
	int stream_cnt(0);
	for (auto itr = graph->Edges(); itr->Alive(); itr->Next()) {
		vid_t x = itr->Source()->GlobalId(), y = itr->Target()->GlobalId();
		auto res = stm.solve(x, y);
		stream_cnt = res.second;
		cout << "\r" << "[streaming]\t" << res.first << " " << res.second << flush;
	}
	cout << endl;
	cout << "[streaming]\t" << stream_cnt << endl;
    score.push_back(toString<int>(stream_cnt));
    // eigen
    EigenTriangle et(graph);
    double et_cnt = et.solve(graph -> VertexCount() * 0.1);
    cout << "[eigen triangle]\t" << et_cnt << endl;
    score.push_back(toString<int>(int(et_cnt)));
	// evaluation
    cout << "\terror " << (float(bf_cnt - stream_cnt) / bf_cnt * 100) << "%" << endl;
    cout << "\terror " << (float(bf_cnt - et_cnt) / bf_cnt * 100) << "%" << endl;
    graph->Close();
    // report table
    table.addRow(score);
    string tableContent = table.report();
    cout << tableContent << endl;
    ofstream fout("report.md");
    fout << tableContent << endl;
    fout.close();
}


void testTable() {
	TableGenerator table;
	string title[] = {"z", "y", "x"};
	table.setTitle(vector<string>(title, title + sizeof(title) / sizeof(title[0])));
	for (int l = 0; l < rand() % 10 + 1; ++l) {
		vector<string> content;
		content.push_back(toString(l));
		for (int i = 0; i < rand() % 10 + 1; ++i) content.push_back(toString(rand() % 1000));
		table.addRow(content);
	}
	cout << table.report() << endl;
}


int main(int argc, char **argv) {
    int vertexNum = 100;
    double edgeProb = 0.7;
	// parse arguments
	Argument args;
	if (!args.parse_args(argc, argv)) return 1;
	//cout << args.content() << endl;
	
	// main process
	srand(time(NULL));
	//testTable();
	makeFakeData(vertexNum, edgeProb);
	testTriangle();
    return 0;
}

