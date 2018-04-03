//
//  GraphLoader.hpp
//  SimpleGraph
//
//  Created by Abderrahmane on 21/08/2016.
//  Copyright © 2016 Ecole Mohammadia d'Ingieurs. All rights reserved.
//

#ifndef GraphLoader_h
#define GraphLoader_h

#include "Graph.hpp"

#include <fstream>


// we  load a graph from file using the example representation
// and deduct how many vertex we have
class GraphLoader {
public:
    template <typename T>
    void load_graph(std::shared_ptr<Graph<T>> g, std::string file_path);
};


// Overall time complexity is O(|E|) where E is a set of edges
template <typename T>
void GraphLoader::load_graph (std::shared_ptr<Graph<T>> g, std::string file_path) {
    std::ifstream file(file_path);
    
    // we use a constant so that we limit control over the nodes
    // to the graph class and reference because we only do checking, we don't need a copy
    // O(1) time complexity
    const std::unordered_map<uint64_t, T>& nodes = g->get_nodes();
    
    // we read from file, if we are able to
    if (file.is_open()) {
        uint64_t source, destination;
        while (file >> source && file >> destination) {
            if (nodes.find(source) == nodes.end())
                g->insert_node(source);
            if (nodes.find(destination) == nodes.end())
                g->insert_node(destination);
            
            g->add_edge(source, destination);
        }
    }
    file.close();
}

#endif /* GraphLoader_h */
