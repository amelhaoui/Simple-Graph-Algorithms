//
//  GraphAlgorithm_parallel.h
//  SimpleGraph
//
//  Created by Abderrahmane on 05/09/2016.
//  Copyright © 2016 Ecole Mohammadia d'Ingieurs. All rights reserved.
//

#ifndef GraphAlgorithm_parallel_h
#define GraphAlgorithm_parallel_h

#include <unordered_map>
#include "ThreadPool.hpp"

// check weither a node has been visited, if not visit it
template <typename T>
bool GraphAlgorithm<T>::is_node_visited(std::shared_ptr<std::unordered_map<uint64_t, bool>> visited,
                                        uint64_t index) {
    
    std::lock_guard<std::mutex> lk(visit_node_mutex);
    if(visited->find(index) == visited->end()) {
        visited->insert({index, true});
        return true;
    }
    return false;
}

// push to the top an element and notify one of the thread waiting this condition
template <typename T>
void GraphAlgorithm<T>::push_element(std::shared_ptr<std::stack<uint64_t>> pool, uint64_t index) {
    std::lock_guard<std::mutex> lk(stack_mutex);
    pool->push(index);
    condition.notify_one();
}

// loop waiting for elements to be added to the stack and consumed
// consumed refers to add adjacent nodes to the concurrent stack ..
// when all thread are waiting, one thread triggers the stop_program and allow all other threads to stop
template <typename T>
void GraphAlgorithm<T>::consume_nodes(std::shared_ptr<Graph<T>> graph,
                                               std::shared_ptr<std::stack<uint64_t>> pool,
                            std::shared_ptr<std::unordered_map<uint64_t, bool>> visited,
                                               std::shared_ptr<int> num_blocked_thread,
                                      std::shared_ptr<bool> stop_program) {
    
    bool first_try = true;
    
    while (true) {
        uint64_t current_node;
        
        {
            std::unique_lock<std::mutex> lk(stack_mutex);
            
            // increment and decrement at the end of the block to detect how many thread are within the block
            ++(*num_blocked_thread);
            
            
            // verify if all threads are waiting, if yes trigger the stop option
            // => no thread can access the stack and send notification
            // and notify all waiting thread to stop
            if (*num_blocked_thread == std::thread::hardware_concurrency()) {
                
                // used because of small graphs, to recheck
                if (first_try) {
                    std::this_thread::yield();
                    first_try = false;
                    --(*num_blocked_thread);
                    continue;
                }
                
                *stop_program = true;
                condition.notify_all();
                return;
            }
            
            while (pool->empty() && !(*stop_program) )
                condition.wait(lk);
            
            if (*stop_program) {
                return;
            }
            
            current_node = pool->top();
            pool->pop();
            
            --(*num_blocked_thread);
        }
        
        NodeIter it (graph->get_nodes(), current_node);
        EdgeIter edge_it = graph->begin_edge(it);
        
        // iterate over all edges starting from node *it
        // and push destination to stack if node not visited
        while (edge_it  != graph->end_edge(it)) {
            uint64_t destination = (*edge_it).second;
            
            if (is_node_visited(visited, destination)) {
                push_element(pool, destination);
            }
            
            ++edge_it;
        }
    }
    
}

// Rotate the edge matrix representation and return smart pointer to a new graph
// overall time complexity O(|E|)
// overall space complexity O(|E|)
template <typename T>
std::shared_ptr<Graph<T>> GraphAlgorithm<T>::transform_graph_to_undirected (std::shared_ptr<Graph<T>> graph) {
    // copy all edges O(|E|) space and O(|E|) time where graph(V, E)
    std::unordered_map<uint64_t, std::unordered_map<uint64_t, bool>> edge = graph->get_all_edges();
    // create a graph copy
    std::shared_ptr<Graph<T>> graph_copy(new Graph<T>());
    
    // start iterating from a node
    // O(|E|) time
    for (NodeIter it = graph->begin(); it != graph->end(); ++it) {
        for(EdgeIter edge_it = graph->begin_edge(it); edge_it != graph->end_edge(it); ++edge_it) {
            edge[(*edge_it).second][(*edge_it).first] = true;
        }
    }
    
    // 2 another copies O(|E|) time O(1) space
    // alternative would be to use shared pointers for the graph members class
    graph_copy->set_nodes(graph->get_nodes());
    graph_copy->set_all_edges(edge);
    
    return graph_copy;
}

template <typename T>
bool GraphAlgorithm<T>::DepthFirstSearch_Parallel (std::shared_ptr<Graph<T>> graph, ThreadPool& thread_pool) {
    
    // push both to the heap of the process
    // better than sharing thread's stack memory adresses
    
    std::shared_ptr<std::unordered_map<uint64_t, bool>> visited(new std::unordered_map<uint64_t, bool>);
    std::shared_ptr<int> num_blocked_thread(new int());
    std::shared_ptr<std::stack<uint64_t>> stack_pool(new std::stack<uint64_t>);
    std::shared_ptr<bool> stop_program(new bool());
    
    //std::vector<std::thread>  thread_pool;
    
    uint64_t start_node = *(graph->begin());
    
    // init the stack with a start element
    stack_pool->push(start_node);
    visited->insert({start_node, true});
    
    // set concurent jobs
    for (int i = 0; i < std::thread::hardware_concurrency(); ++i) {
        thread_pool.doJob("DFS", std::bind(&GraphAlgorithm<T>::consume_nodes, this,
                                          graph, stack_pool, visited, num_blocked_thread, stop_program));
    }
    
    // wait for all threads to finish
    while (!thread_pool.isFinished("DFS"))
        continue;
    
    // at this point only the main thread is running
    // it's safe to get the size of the hash table
    return visited->size() == graph->get_nodes().size() ? true : false;
}


template <typename T>
void GraphAlgorithm<T>::consume_nodes_distance(std::shared_ptr<Graph<T>> graph,
                                      std::shared_ptr<std::queue<uint64_t>> pool,
                                      std::shared_ptr<std::unordered_map<uint64_t, bool>> visited,
                                    std::shared_ptr<std::unordered_map<uint64_t, uint64_t>> distances,
                                      std::shared_ptr<int> num_blocked_thread,
                                               std::shared_ptr<bool> stop_program) {
    
    bool first_try = true;
    
    while (true) {
        uint64_t current_node;
        
        {
            std::unique_lock<std::mutex> lk(queue_mutex);
            
            // increment and decrement at the end of the block to detect how many thread are within the block
            ++(*num_blocked_thread);
            
            
            // verify if all threads are waiting, if yes trigger the stop option
            // => no thread can access the stack and send notification
            // and notify all waiting thread to stop
            if (*num_blocked_thread == std::thread::hardware_concurrency()) {
                
                // used because of small graphs, to recheck
                if (first_try) {
                    std::this_thread::yield();
                    first_try = false;
                    --(*num_blocked_thread);
                    continue;
                }
                
                *stop_program = true;
                condition.notify_all();
                return;
            }
            
            while (pool->empty() && !*stop_program )
                condition.wait(lk);
            
            if (*stop_program) {
                return;
            }
            
            current_node = pool->front();
            pool->pop();
            
            --(*num_blocked_thread);
        }
        
        NodeIter it (graph->get_nodes(), current_node);
        EdgeIter edge_it = graph->begin_edge(it);
        
        // iterate over all edges starting from node *it
        // and push destination to stack if node not visited
        while (edge_it  != graph->end_edge(it)) {
            uint64_t destination = (*edge_it).second;
            
            {
                std::unique_lock<std::mutex> lk(visit_node_mutex);
                
                if(visited->find(destination) == visited->end()) {
                    visited->insert({destination, true});
                    pool->push(destination);
                }
                
                distances->at(destination) = std::min (distances->at(destination), distances->at(current_node) + 1);

            }
            
            ++edge_it;
        }
    }
    
    
}



#endif /* GraphAlgorithm_parallel_h */
