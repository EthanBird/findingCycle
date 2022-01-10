//
// Created by Administrator on 2021/10/15.
//
#include <fstream>
#include <stack>
#include <functional>
#define MIN_EDGE_AMOUNT 3
#define MAX_EDGE_AMOUNT 6
#include "Graph.h"

bool Graph::add_node(g_id id) {
    if(node_exists(id)){
        // cout << "ListNode " << id << " already exist!" << endl;
        return false;
    }

    node_ptr n(new Node(id));
    nodes.emplace(id, n);
    return true;
}
bool Graph::add_edge(g_id from, g_id to, Time timestamp, double amount){
    if(!node_exists(from) || !node_exists(to)){
        // cout << "Nonexistent node(-s). Cannot add edge!" << endl;
        return false;
    }

    edge_ptr e(new Edge(nodes.at(from), nodes.at(to), timestamp, amount));
//    edges.emplace(e->id,e);
    nodes.at(from)->edges_out->emplace_back(e);
    nodes.at(to)->edges_in->emplace_back(e);
    return true;
}
bool Graph::remove_node(g_id id){
    if(!node_exists(id)){
        // cout << "Cannot remove nonexistent node!" << endl;
        return false;
    }

    // Remove all edges to and from node
    for(auto& n : nodes){
        //cout << n.first << endl;
        if(edge_exists(n.first, id))
            remove_edge(n.first, id);
        if(edge_exists(id, n.first))
            remove_edge(id, n.first);
    }

    // Remove node
    nodes.erase(id);
    return true;
}
bool Graph::remove_edge(g_id from, g_id to){
    if(!edge_exists(from, to)){
        cout << "Cannot remove nonexistent edge!" << endl;
        return false;
    }

    // TODO: remove this code duplication
    auto edge_it = std::find_if(edges.begin(), edges.end(), [from,to](std::pair<g_id, edge_ptr> e)
    { return (e.second->from->id == from && e.second->to->id == to); });
    if(edge_it != edges.end()) edges.erase(edge_it->first);
//    nodes.at(from)->remove_edge_to(to);
//    nodes.at(to)->remove_edge_from(from);
    return true;
}
bool Graph::node_exists(g_id node) const{
    return !(nodes.find(node) == nodes.end());
}
bool Graph::edge_exists(g_id from, g_id to) const{
    return std::find_if(edges.begin(), edges.end(), [from,to](std::pair<g_id, edge_ptr> e)
    { return (e.second->from->id == from && e.second->to->id == to); }) != edges.end();
}

void Graph::dfs_1_iter(Graph* my_graph, node_ptr start, std::vector<OnePath>* a_ans){

    std::stack<edge_ptr> edge_stack;
    OnePath tmp_path;
    std::vector<OnePath>* ans = a_ans == nullptr ? my_graph->output_ans : a_ans;
    std::unordered_set<g_id> visit;
    visit.insert(start->id);
    int depth = 0;
    auto head = start->edges_out->get_header()->forward[0];
    while(head != nullptr){
        for(auto it = head->values.begin(); it !=head->values.end() && *it != nullptr; it++){
            edge_stack.push((*it));
        }
        head = head->forward[0];
    }

    while(!edge_stack.empty()){
        while( !tmp_path.empty() && tmp_path.back()->to->id != edge_stack.top()->from->id){
            visit.erase(tmp_path.back()->to->id);
            tmp_path.pop_back();
        }

        tmp_path.push_back(edge_stack.top());
        edge_stack.pop();
        depth = tmp_path.size();

        edge_ptr last_edge = tmp_path.back();
        node_ptr cur = last_edge->to;
        visit.insert(cur->id);
        if(depth >= MAX_EDGE_AMOUNT){
            continue;
        }

        double left_amount = last_edge->amount * 0.9;
        double right_amount = last_edge->amount * 1.1;
        auto leftNode = cur->edges_out->startOf(left_amount);
        auto rightNode = cur->edges_out->endOf(right_amount);
        if(rightNode != nullptr && rightNode->get_key() <= right_amount){
            rightNode = rightNode->forward[0];
        }
        while(leftNode != nullptr && leftNode != rightNode  ){
            for(auto it = leftNode->values.upper_bound(last_edge); it != leftNode->values.end() ; it++){
                edge_ptr cur_edge = *it;
                if(cur_edge->to->id == start->id && depth >= MIN_EDGE_AMOUNT - 1){
                    tmp_path.push_back(cur_edge);

                    my_graph->ans_mutex.lock();
                    ans->push_back(tmp_path);
                    *(my_graph->counter) += 1;
                    my_graph->ans_mutex.unlock();

                    if(*(my_graph->counter) % 100 == 0){
                        cout<<"found:"<<*(my_graph->counter)<<" | ept:"<<0<<endl;
                    }
                    tmp_path.pop_back();
                }

                if(visit.count(cur_edge->to->id) == 0 && depth < MAX_EDGE_AMOUNT - 1){
                    edge_stack.push(cur_edge);
                }

            }
            leftNode = leftNode->forward[0];
        }
    }
}
void  Graph::batch_add_node_by_string(Graph *graph, std::vector<string> *str_vec, int real_size ,bool to_be_delete ){
    if(real_size == -1)
        real_size = str_vec->size();
    std::vector<node_ptr> *node_vec = new std::vector<node_ptr>(real_size);
    for(auto str_node: *str_vec){
        if(str_node.empty())
            continue;
        g_id id = std::stoull(str_node);
        node_ptr node(new Node(id));
        node_vec->emplace_back(node);
    }

    graph->nodes_mutex.lock();
    for(auto node : (*node_vec)){
        if(node != nullptr){
            graph->nodes.emplace(node->id, node);
        }
    }
    graph->nodes_mutex.unlock();
    if(to_be_delete)
        delete str_vec;
    delete node_vec;
}
void  Graph::batch_add_edge_by_string(Graph* graph, std::vector<string> *str_vec, int real_size ,bool to_be_delete){
    if(real_size == -1)
        real_size = str_vec->size();
    int count = 0;

    for(auto str_edge: *str_vec){
        if(str_edge.size() <= 1)
            continue;
        std::vector<string> tokens;
        graph->SplitString(str_edge,tokens,",");
        g_id from               = std::stoull(tokens[0]);
        g_id to                 = std::stoull(tokens[1]);
        if(!graph->node_exists(from) || !graph->node_exists(to)){
            continue;
        }
        Time timestamp          = std::stoull(tokens[2]);
        double amount           = std::stod(tokens[3]);
        count ++;
        edge_ptr edge(new Edge(graph->nodes.at(from), graph->nodes.at(to), timestamp, amount));
        graph->nodes.at(from)->edges_out->emplace_back(edge);
        graph->nodes.at(from)->edges_in->emplace_back(edge);

    }
    graph->edges_mutex.lock();
    edge_size += count;
    graph->edges_mutex.unlock();
    if(to_be_delete)
        delete str_vec;
}

void Graph::make_mini_dataset(int size){
    if(this->empty()){
        return;
    }
    std::unordered_set<g_id> tmpNode;
    std::unordered_set<g_id> edgeMark;
    std::vector<g_id> tmpNode_v;

    int count = size;
    for(auto ptr : this->nodes){
        if(tmpNode.size() < size){
            tmpNode.emplace(ptr.first);
            tmpNode_v.push_back(ptr.first);
        } else {
            break;
        }
    }
    size = size > tmpNode_v.size() ? tmpNode_v.size() : size;
    std::ofstream  a("C:\\data\\account_mini.csv");
    std::ofstream  b("C:\\data\\transfer_mini.csv");
    for(int i = 0; i < size; i++){
        g_id id =  tmpNode_v[i];
        a<<id<<endl;
    }
    a.close();
    b.close();
}
void Graph::SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c){
    std::string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while(std::string::npos != pos2)
    {
        v.emplace_back(s.substr(pos1, pos2-pos1));

        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }
    if(pos1 != s.length())
        v.emplace_back(s.substr(pos1));
}
