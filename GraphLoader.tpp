//
//  GraphLoader.cpp
//  SimpleGraph
//
//  Created by Abderrahmane on 30/08/2016.
//  Copyright © 2016 Ecole Mohammadia d'Ingieurs. All rights reserved.
//

#include <fstream>

template <typename T>
void GraphLoader<T>::load_graph (std::shared_ptr<Graph<T>> g, std::string file_path) {
    std::ifstream file(file_path);
    
    if (file.is_open()) {
        uint64_t source, destination;
        while (file >> source && file >> destination) {
            g->add_edge(source, destination);
        }
    }
    file.close();
}