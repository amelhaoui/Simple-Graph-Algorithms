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
   
    std::shared_ptr<Graph<std::string>> graph(new Graph<std::string>());
    GraphLoader g_loader;
    
    // used to write in file for testing
    /**
    std::ofstream file("test.in");
    if (file.is_open()) {
        for (int i = 0; i < 50; ++i) {
            for (int j = 0; j < 50; ++j) {
                if (j == i) {
               file << i << " " << j << "\n";
              }
            }
        }
    }
    file.close();
    **/
    

    g_loader.load_graph(graph, "test.in");
    
    std::cout << graph->export_node_property_to_string();
    
    GraphAlgorithm<std::string> algo;
    ThreadPool pool(std::thread::hardware_concurrency());

    std::cout << " Is Weakly Connected Parallel " << algo.IsWeaklyConnectedParallel(graph, pool) << std::endl;

    
    std::shared_ptr<std::unordered_map<uint64_t, uint64_t>>  result = algo.GetShortestDistancesParallel(graph, 0, pool);
    
    if (result != NULL) {
        for (auto i = result->begin(); i != result->end(); ++i)
            std::cout <<i->first << " is far by " << i->second << std::endl;
    }
    
    
    std::cout << "Is Fully Connected Parallel " << algo.IsFullyConnectedParallel(graph, pool) << std::endl;
    
    std::cout << "Is Fully Connected " << algo.IsFullyConnected(graph) << std::endl;
    
    std::cout << " Is Weakly Connected " << algo.IsWeaklyConnected(graph) << std::endl;



    

    
    
    return 0;
}