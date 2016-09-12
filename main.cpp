//
//  main.cpp
//  SimpleGraph
//
//  Created by Abderrahmane on 18/08/2016.
//  Copyright © 2016 Ecole Mohammadia d'Ingenieurs. All rights reserved.
//

#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <cstdint>
#include <functional>
#include <queue>

#include "GraphAlgorithm.hpp"
#include "GraphLoader.hpp"

using  ns = std::chrono::nanoseconds;
using get_time = std::chrono::steady_clock ;



int main(int argc, const char * argv[]) {
   
    //uint64_t len;
    //std::cin >> len;
    std::shared_ptr<Graph<std::string>> graph(new Graph<std::string>());
    GraphLoader g_loader;
    
//    std::ofstream file("test.in");
//    if (file.is_open()) {
//        for (int i = 0; i < 50; ++i) {
//            for (int j = 0; j < 50; ++j) {
//                if (j!= i) {
//               file << i << " " << j << "\n";
//              }
//            }
//        }
//    }
//    file.close();
    

    g_loader.load_graph(graph, "test.in");
    
    GraphAlgorithm<std::string> algo;
    ThreadPool pool(std::thread::hardware_concurrency());

    
    std::shared_ptr<std::unordered_map<uint64_t, uint64_t>>  result = algo.GetShortestDistancesParallel(graph, 0, pool);
    
    for (auto i = result->begin(); i != result->end(); ++i)
        std::cout <<i->first << " is far by " << i->second << std::endl;
    
    
    //std::cout << algo.IsFullyConnectedParallel(graph, pool) << std::endl;
    
    //std::cout << algo.IsWeaklyConnectedParallel(graph, pool) << std::endl;

//
//    for (int i = 0; i < 1; ++i) {
//    auto start = get_time::now();
//    std:: cout << algo.IsFullyConnectedParallel(graph)<< std::endl;
//    auto end = get_time::now();
//    auto diff = end - start;
//    std::cout<<"Elapsed time in fully P is :  "<< std::chrono::duration_cast<ns>(diff).count()<<" ns "<<std::endl;
//    }
    
//    start = get_time::now();
//    std:: cout << algo.IsFullyConnectedParallel(graph)<< std::endl;
//    end = get_time::now();
//    diff = end - start;
//    std::cout<<"Elapsed time in fully P is :  "<< std::chrono::duration_cast<ns>(diff).count()<<" ns "<<std::endl;
    
    

    

    
    
    return 0;
}




//    for (Graph<std::string>::NodeIter it = graph->begin(); it != graph->end(); ++it) {
//        for (Graph<std::string>::EdgeIter edge_it = graph->begin_edge(it); edge_it != graph->end_edge(it); ++edge_it) {
//            std::cout << "edge " << *(it) << " " << (*edge_it).first << (*edge_it).second << std::endl;
//        }
//    }

//    auto start = get_time::now();
//    std:: cout << algo.is_fully_connected_parallel(graph)<< std::endl;
//    auto end = get_time::now();
//    auto diff = end - start;
//    std::cout<<"Elapsed time in fully is :  "<< std::chrono::duration_cast<ns>(diff).count()<<" ns "<<std::endl;

//    std::stack<NodeIter<std::string>> s;
//    s.push(it);
//
//    while (!is_empty(s)) {
//        NodeIter<std::string> current = s.top();
//        s.pop();
//        visited[*current] = true;
//
//
//    }

   // std::atomic<bool> plain_array[5] = {};
    //std::atomic<bool> p(false);
//    for (int i  = 0; i < 5; ++i) {
//        threads.push_back(std::thread(values_push_back, std::ref(plain_array), i));
//    }
//    for (auto& th : threads) th.join();
    
    //std::cout << value <<std::endl;
//    for (bool th : values)
//        std::cout << th <<std::endl;
   // std::cout << p.load() ;
    
//    uint64_t l;
//    std::cin >> l;
//    std::shared_ptr<Graph<std::string>> graph(new Graph<std::string>(l));
//    GraphLoader g_loader;
//    
//    g_loader.load_graph(graph, "test.in");
//    graph.add_edge(0L, 1L);
//    graph.add_edge(0L, 2L);
//    graph.add_edge(0L, 3L);
//    graph.add_edge(3L, 4L);
//    graph.add_edge(4L, 5L);
//    
//    //std::cout << graph.are_adjacents(0, 1) << std::endl;
//    //std::cout << graph.are_adjacents(0L, 0L) << std::endl;
//    graph.set_node_property(2L, "Me");
//    NodeIter<std::string> it(graph);
//    for (it; it.is_valid(); ++it) {
//        EdgeIter<std::string> e_it(graph, it);
//        for (e_it; e_it.is_valid(); ++e_it)
//            std::cout << "edge " << *(it) << " " << (*e_it).first << (*e_it).second << std::endl;
//        
//    }
//    GraphAlgorithm algo;
//    std::cout << algo.is_weakly_connected(graph) << std::endl;
//    std::cout << algo.is_fully_connected(graph) << std::endl;

    
//    std:: cout <<graph.export_node_property_to_string();
//    return 0;
//}
