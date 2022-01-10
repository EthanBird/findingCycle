#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <vector>
#include <thread>
#include <functional>
#include <set>
#include "Graph.h"
#include "threadpool.h"

#define BATCH_SIZE 32
using namespace  std;

clock_t start;

void record_time(clock_t start, int display_type = 0);

#if 1
void call_edge(Graph* graph, std::vector<string> *str_vec, int real_size = -1 , bool to_be_delete = false){
    graph->batch_add_edge_by_string(graph, str_vec, real_size, to_be_delete);
}
void call_node(Graph* graph, std::vector<string> *str_vec, int real_size = -1 ,bool to_be_delete = false){
    graph->batch_add_node_by_string(graph, str_vec, real_size, to_be_delete);
}

template<typename T>
void call_loop(Graph *graph, T np) {
    graph->dfs_1_iter(graph, np);
}

void guard_node(Graph* graph, int total_node){
    while(graph->nodes.size() != total_node){
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}
void guard_edge(Graph* graph, int total_edge){
    while(graph->get_edge_size() != total_edge){
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}
void guard_loop(std::threadpool* exec){
    while(exec->_tasks.size() != 0){
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}
int main(int argc, char* argv[]) {
    start=clock();
    std::threadpool executor{ 32 };
    //string output_file_name  = "H:\\data\\out_task_1_2021_11_22.txt";
    string account_file_name = argv[1];
    string transfer_file_name= argv[2];
	string output_file_name  = argv[3];
//    account_file_name = "H:\\data\\account_tiny.csv";  // tiny 2
//    transfer_file_name= "H:\\data\\transfer_tiny.csv"; // tiny 2

//    account_file_name = "H:\\data\\account_mini.csv";  // mini 5037
//   transfer_file_name= "H:\\data\\transfer_mini.csv"; // mini 5037

//    account_file_name ="H:\\data\\account.csv";// task 1
//    transfer_file_name="H:\\data\\transfer.csv";// task 1

//    account_file_name =R"(H:\data\scale10\account.csv)"; // task 2
//    transfer_file_name=R"(H:\data\scale10\transfer.csv)";// task 2

    ifstream account(account_file_name);
    ifstream transfer(transfer_file_name);
    ofstream outfile(output_file_name);

    string temp;

    Graph* my_graph = new Graph();

    if(!account.is_open() || !transfer.is_open()){
        cout<<"Wrong!"<<endl;
        transfer.close();
        account.close();
        // outfile.close();
        exit(-1);
    }
    vector<string> *acc_v = new vector<string>(BATCH_SIZE);
    vector<string> *tran_v = new vector<string>(BATCH_SIZE);
    int line = 0;
    int total_node = 0;

    cout<<"add Node"<<endl;
    while(getline(account,temp)){
        (*acc_v)[line++] = temp;
        total_node++;
        if(line == BATCH_SIZE){
            executor.commit(std::bind(call_node,my_graph,acc_v,line,true));
            line = 0;
            acc_v = new vector<string>(BATCH_SIZE);
        }
    }
    if(line != 0){
        executor.commit(std::bind(call_node,my_graph,acc_v,line,true));
        line = 0;
    }

    cout<<"waiting thread..."<<endl;
    thread guard_node_thread(guard_node, my_graph, total_node);
    guard_node_thread.join();

    record_time(start);

    int total_edge = 0;
    cout<<"add edge"<<endl;
    while(getline(transfer,temp)){
        if(temp.empty()) continue;
        (*tran_v)[line++] = temp;
        total_edge++;
        if(line == BATCH_SIZE){
            executor.commit(std::bind(call_edge,my_graph,tran_v,line,true));
            line = 0;
            tran_v = new vector<string>(BATCH_SIZE);
        }

    }
    if(line != 0){
        executor.commit(std::bind(call_edge,my_graph,tran_v,line,true));
    }
    cout<<"input_total_edge:"<<total_edge<<endl;
    // 2563373
    cout<<"waiting thread..."<<endl;
    thread guard_edge_thread(guard_edge, my_graph, total_edge);
    guard_edge_thread.join();
    record_time(start);


    for(const auto& node_item : my_graph->nodes){
        auto np = node_item.second;
        if(np->edges_in->size() && np->edges_out->size()){
            executor.commit(std::bind(call_loop<shared_ptr<Graph::Node>>, my_graph, np));
        }
    }


    cout<<"waiting executor thread..."<<endl;
    thread guard_loop_thread(guard_loop, &executor);
    guard_loop_thread.join();

    cout<<"output_ans->size() : "<<my_graph->output_ans->size()<<endl;
    for(auto tmpPath : *(my_graph->output_ans)){
        outfile<<"("<<tmpPath[0]->from->id<<")";
        for(int i = 0; i < tmpPath.size(); i++){
            outfile<<"-["<<tmpPath[i]->timestamp<<","<<std::fixed<<std::setprecision(2)<<tmpPath[i]->amount<<"]->("<<tmpPath[i]->to->id<<")";
        }
        outfile<<endl;
    }
    record_time(start);
    account.close();
    transfer.close();
    outfile.close();

    delete my_graph;
    return 0;
}


# endif

void record_time(clock_t start, int display_type) {
    clock_t ending = clock();
    double endtime=(double)(ending-start)/CLOCKS_PER_SEC;
    switch (display_type) {
        case 0: cout<<"Total time:"<<endtime<<endl;		//s为单位
        default:cout<<"Total time:"<<endtime*1000<<"ms"<<endl;	//ms为单位
    }
}