
#ifndef GRAPH_H
#define GRAPH_H
#define SYC_TASK
#include <iostream>
#include <thread>
#include <vector>
#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <iomanip>
#include "SkipList.h"

#ifdef SYC_TASK
#define OnePath std::vector<edge_ptr>
#define SKIP_LIST_MAX_LEVEL 16
#endif

using std::string;
using std::cout;
using std::endl;

//  graph (bigraph)
class Graph{
public:
    int i = 0;
    int* counter = &i;
    typedef unsigned long long g_id;
    std::mutex nodes_mutex;
    std::mutex edges_mutex;
    std::mutex next_edge_id_mutex;
    std::mutex ans_mutex;
    // Internal counter for edge ID
    g_id  next_edge_id = 0;
    g_id get_next_edge_id() {
        return next_edge_id++;
    }
    // Define node and edge structs

    struct Node;
    struct Edge;

    typedef unsigned long long Time;


    typedef std::shared_ptr<Node> node_ptr;
    typedef std::shared_ptr<Edge> edge_ptr;
    long edge_size = 0;
    struct Node{
        g_id id;
        SkipList<double, edge_ptr>* edges_in = new SkipList<double, edge_ptr>(SKIP_LIST_MAX_LEVEL);
        SkipList<double, edge_ptr>* edges_out= new SkipList<double, edge_ptr>(SKIP_LIST_MAX_LEVEL);
        //        std::vector<edge_ptr> *edges_in = new std::vector<edge_ptr>();
        //        std::vector<edge_ptr> *edges_out = new std::vector<edge_ptr>();
        Node(g_id id) : id(id) {}

    };
    struct Edge{
        node_ptr from, to;
        Time timestamp;
        double amount;

		Edge( node_ptr from, node_ptr to, Time timestamp, double amount)
		: from(from), to(to), timestamp(timestamp), amount(amount) {}
    };


    void dfs_test(node_ptr start, node_ptr cur, edge_ptr last_edge, int depth, std::unordered_set<g_id>* set,
                  std::vector<OnePath>* ans, OnePath* tmpPath);
    void dfs_1(node_ptr start, node_ptr cur, edge_ptr last_edge, int depth, std::unordered_set<g_id>* set,
                  std::vector<OnePath>* ans, OnePath* tmpPath);

    Graph() {
        next_edge_id = 1;
        *counter = 0;
        cout.setf(std::ios::fixed);
    }
    // Map of nodes and edges (using id as key)
    std::unordered_map<g_id, node_ptr> nodes;
    std::unordered_map<g_id, edge_ptr> edges;
    std::vector<OnePath> *output_ans = new std::vector<OnePath>;


    // TODO: consider encapsulating the nodes and edges
    void doMyTask();
    void dfs_1_iter( Graph*, node_ptr , std::vector<OnePath>* ans = nullptr);
    void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c);
    bool add_node(g_id id);
    bool add_edge(g_id from, g_id to, Time timestamp, double amount);
    void  batch_add_node_by_string(Graph *graph, std::vector<string> *str_vec, int real_size = -1 ,bool to_be_delete = false);
    void  batch_add_edge_by_string(Graph* graph, std::vector<string> *str_vec, int real_size = -1 ,bool to_be_delete = false);

    bool remove_node(g_id id);
    bool remove_edge(g_id from, g_id to);

    bool node_exists(g_id node) const;
    bool edge_exists(g_id from, g_id to) const;

    long get_edge_size(){
        return edge_size;
    }
    bool empty_node()   { return nodes.empty(); }
    bool empty_edge()   { return edge_size == 0; }
    bool empty()    { return empty_node() || empty_edge(); }
    void make_mini_dataset(int size = 100000);
    void search();

    void test_LRU();

};




#endif // GRAPH_H