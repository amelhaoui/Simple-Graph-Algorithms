//
//  GraphAlgorithm.hpp
//  SimpleGraph
//
//  Created by Abderrahmane on 31/08/2016.
//  Copyright © 2016 Ecole Mohammadia d'Ingénieurs. All rights reserved.
//

#ifndef GraphAlgorithm_hpp
#define GraphAlgorithm_hpp



#include "Graph.hpp"
#include "ThreadPool.hpp"



template<typename T>
class GraphAlgorithm {
 public:
  typedef typename Graph<T>::NodeIter NodeIter;
  typedef typename Graph<T>::EdgeIter EdgeIter;

  // stack_mutex: control the stack pushing and poping actions
  // visit_node_mutex: control if a node has been visited and to check state to visited
  // We can use one mutex  and condition variable
  // in multiple context if we suppose algorithms won't be concurrent
  std::mutex visit_node_mutex, stack_mutex, mutex, queue_mutex;
  // condition: wait until an index node has been added to the stack of jobs or to exist if notified to
  // when an element is available one of the waiting thread proceed
  std::condition_variable condition;

 public:
  bool IsWeaklyConnected(std::shared_ptr<Graph<T>> g);

  bool IsFullyConnected(std::shared_ptr<Graph<T>> g);

  // Algorithm to retrieve minimal distance to all vertices from a start node idx
  // Distance in terms of edges separting start_idx and all vertices
  // unreachable nodes have value of UINT64_MAX
  std::unordered_map<uint64_t, uint64_t> GetShortestDistances(std::shared_ptr<Graph<T>> graph,
                                                              uint64_t start_idx);

  // Depth First Search algorithm to explore the graph
  // start node is determined by the Node iterator
  bool DepthFirstSearch(std::shared_ptr<Graph<T>> graph);

/* ----- Parallel ------ */

  bool IsWeaklyConnectedParallel(std::shared_ptr<Graph<T>> graph, ThreadPool &thread_pool);

  bool IsFullyConnectedParallel(std::shared_ptr<Graph<T>> graph, ThreadPool &thread_pool);

  std::shared_ptr<std::unordered_map<uint64_t, uint64_t>> GetShortestDistancesParallel(std::shared_ptr<Graph<T>> graph,
                                                                                       uint64_t start_idx,
                                                                                       ThreadPool &thread_pool);

  bool DepthFirstSearchParallel(std::shared_ptr<Graph<T>> graph, ThreadPool &thread_pool);

  // Function waiting for available nodes to consume them
  // consume means check if the node has out-edges from it and push them to the stack
  void consume_nodes(std::shared_ptr<Graph<T>> graph,
                     std::shared_ptr<std::stack<uint64_t>> pool,
                     std::shared_ptr<std::unordered_map<uint64_t, bool>> visited,
                     std::shared_ptr<int> num_running_thread,
                     std::shared_ptr<bool> stop_program);

  // Consume nodes on the queue and updating distances
  void consume_nodes_distance(std::shared_ptr<Graph<T>> graph,
                              std::shared_ptr<std::queue<uint64_t>> pool,
                              std::shared_ptr<std::unordered_map<uint64_t, bool>> visited,
                              std::shared_ptr<std::unordered_map<uint64_t, uint64_t>> distances,
                              std::shared_ptr<int> num_blocked_thread,
                              std::shared_ptr<bool> stop_program);

  // Push the element index to the stack and ensuring it's atomic by a lock_guard
  void push_element(std::shared_ptr<std::stack<uint64_t>> pool, uint64_t index);

  // Check if a node has been visited using std::lock_guard
  bool is_node_visited(std::shared_ptr<std::unordered_map<uint64_t, bool>> visited, uint64_t index);

  // (start, end) represent the window of bucket this helper will verify and return
  // the result to the main thread
  void is_fully_connected_helper(uint64_t start, uint64_t end,
                                 std::unordered_map<uint64_t, std::unordered_map<uint64_t, bool>> &edges,
                                 uint64_t num_distinct_edges,
                                 std::shared_ptr<bool> is_fully_connected);

//----- Common ------

  // rotate the edge matrix representation and return smart pointer to a new graph
  std::shared_ptr<Graph<T>> transform_graph_to_undirected(std::shared_ptr<Graph<T>> graph);

};

// Rotate the edge matrix representation and return smart pointer to a new graph
// overall time complexity O(|E|)
// overall space complexity O(|E|)
template<typename T>
std::shared_ptr<Graph<T>> GraphAlgorithm<T>::transform_graph_to_undirected(std::shared_ptr<Graph<T>> graph) {
  // copy all edges O(|E|) space and O(|E|) time where graph(V, E)
  std::unordered_map<uint64_t, std::unordered_map<uint64_t, bool>> edge = graph->get_all_edges();
  // create a graph copy
  std::shared_ptr<Graph<T>> graph_copy(new Graph<T>());

  // start iterating from a node
  // O(|E|) time
  for (NodeIter it = graph->begin(); it != graph->end(); ++it) {
    for (EdgeIter edge_it = graph->begin_edge(it); edge_it != graph->end_edge(it); ++edge_it) {
      edge[(*edge_it).second][(*edge_it).first] = true;
    }
  }

  // 2 another copies O(|E|) time O(1) space
  // alternative would be to use shared pointers for the graph members class
  graph_copy->set_nodes(graph->get_nodes());
  graph_copy->set_all_edges(edge);

  return graph_copy;
}

// Depth first search implementation
// overall time complexity O(|E| +|V|) where graph(V, E)
// overall space complexity O(|V|)
template<typename T>
bool GraphAlgorithm<T>::DepthFirstSearch(std::shared_ptr<Graph<T>> graph) {
  std::unordered_map<uint64_t, bool> visited;

  NodeIter it = graph->begin();
  std::stack<NodeIter> s;
  s.push(it);

  while (!s.empty()) {
    NodeIter current = s.top();
    s.pop();
    visited[*current] = true;

    EdgeIter edge_it = graph->begin_edge(current);
    while (edge_it != graph->end_edge(current)) {
      if (visited.find((*edge_it).second) == visited.end())
        s.push(NodeIter(graph->get_nodes(), (*edge_it).second));
      ++edge_it;
    }
  }
  return visited.size() == graph->get_nodes().size() ? true : false;
}

// We transform the graph to an undirected graph then we try to visit all nodes using DFS
// from a starting node determined by the NodeIterator
// Overall Time complexity O(|V| + |E|)
// Overall Space Complexity O(|V| + |E|)
template<typename T>
bool GraphAlgorithm<T>::IsWeaklyConnected(std::shared_ptr<Graph<T>> graph) {
  graph = transform_graph_to_undirected(graph); // O(|E|) time and space
  return DepthFirstSearch(graph); // O(|V|) space and O(|V| + |E|) time
}

// Verify if each (w, u) from V where graph(V, E) there is a directed edge from w to u and from u to w
// We do it by comparing the size of edges starting from each node
// The total number of distinct edges needs to be |V|*|V-1| if there are no self edges from every vertex
// Overall Time Complexity O(|V|)
// Overall Space Complexity O(1)
template<typename T>
bool GraphAlgorithm<T>::IsFullyConnected(std::shared_ptr<Graph<T>> graph) {

  std::unordered_map<uint64_t, std::unordered_map<uint64_t, bool>> &edges = graph->get_all_edges();
  uint64_t num_distinct_edges = graph->get_nodes().size() - 1; // return |V|-1  where graph(V,E)

  // we verify first that there is at least one edge starting from each node
  if (edges.size() != graph->get_nodes().size())
    return false;

  // iterate over all vertices and compare the size of edges starting from this vertices
  for (auto it = graph->begin(); it != graph->end(); ++it) {
    // if there is a self-edge for the node *it, we decrement the size before we compare
    if (edges[*it].find(*it) != edges[*it].end() &&
        edges[*it].size() - 1 != num_distinct_edges)
      return false;
    else if (edges[*it].find(*it) == edges[*it].end()
        && edges[*it].size() != num_distinct_edges)
      return false;
  }

  return true;
}

// We find shortest distance by using the Breadth first search on the graph
// and using comparaison of accumulated distance
// Overall time complexity is same for BFS
template<typename T>
std::unordered_map<uint64_t, uint64_t> GraphAlgorithm<T>::GetShortestDistances(std::shared_ptr<Graph<T>> graph,
                                                                               uint64_t start_idx) {

  std::unordered_map<uint64_t, uint64_t> distance;
  // init distances to UINT64 MAX
  for (auto it = graph->get_nodes().begin(); it != graph->get_nodes().end(); ++it)
    distance[it->first] = std::numeric_limits<uint64_t>::max();

  distance[start_idx] = 0; // init distance for start node

  // nodes that has been visited
  std::unordered_map<uint64_t, bool> visited;
  visited[start_idx] = true;

  std::queue<uint64_t> queue;
  queue.push(start_idx);

  while (!queue.empty()) {
    uint64_t current = queue.front();
    queue.pop();
    visited[current] = true;

    NodeIter it(graph->get_nodes(), current);

    for (EdgeIter edge_it = graph->begin_edge(it); edge_it != graph->end_edge(it); ++edge_it) {
      uint64_t destination = (*edge_it).second;

      if (!visited[destination])
        queue.push(destination);
      // chose the min value from two values
      distance[destination] = std::min(distance[destination], distance[current] + 1);

    }

  }
  return distance;

}


//----------------------------------------------
//---------Parallel version of algorithms ------
//----------------------------------------------

template<typename T>
void GraphAlgorithm<T>::is_fully_connected_helper(uint64_t start,
                                                  uint64_t end,
                                                  std::unordered_map<uint64_t,
                                                                     std::unordered_map<
                                                                         uint64_t,
                                                                         bool>> &edges,
                                                  uint64_t num_distinct_edges,
                                                  std::shared_ptr<bool> is_fully_connected) {

  for (uint64_t i = start; i < end; ++i) {
    {
      // we verify if another thread have found it's not fully connected
      std::lock_guard<std::mutex> lk(mutex);
      if (!(*is_fully_connected)) return;
    }

    for (auto local_it = edges.begin(i); local_it != edges.end(i); ++local_it) {
      uint64_t source = local_it->first;
      std::unordered_map<uint64_t, bool> &out_edges = local_it->second;

      if ((out_edges.find(source) != out_edges.end() && out_edges.size() - 1 != num_distinct_edges) ||
          (out_edges.find(source) == out_edges.end() && out_edges.size() != num_distinct_edges)) {
        {
          std::lock_guard<std::mutex> lk(mutex);
          *is_fully_connected = false;
        }
      }

    }
  }
}

// check weither a node has been visited, if not visit it
template <typename T>
bool GraphAlgorithm<T>::is_node_visited(std::shared_ptr<std::unordered_map<uint64_t, bool>> visited, uint64_t index) {

  std::lock_guard<std::mutex> lk(visit_node_mutex);
  if (visited->find(index) == visited->end()) {
    visited->insert({index, true});
    return true;
  }
  return false;
}

// push to the top an element and notify one of the thread waiting this condition
template<typename T>
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

      while (pool->empty() && !(*stop_program))
        condition.wait(lk);

      if (*stop_program) {
        return;
      }

      current_node = pool->top();
      pool->pop();

      --(*num_blocked_thread);
    }

    NodeIter it(graph->get_nodes(), current_node);
    EdgeIter edge_it = graph->begin_edge(it);

    // iterate over all edges starting from node *it
    // and push destination to stack if node not visited
    while (edge_it != graph->end_edge(it)) {
      uint64_t destination = (*edge_it).second;

      if (is_node_visited(visited, destination)) {
        push_element(pool, destination);
      }

      ++edge_it;
    }
  }

}


template <typename T>
bool GraphAlgorithm<T>::DepthFirstSearchParallel(std::shared_ptr<Graph<T>> graph, ThreadPool &thread_pool) {

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

template<typename T>
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

      while (pool->empty() && !*stop_program)
        condition.wait(lk);

      if (*stop_program) {
        return;
      }

      current_node = pool->front();
      pool->pop();

      --(*num_blocked_thread);
    }

    NodeIter it(graph->get_nodes(), current_node);
    EdgeIter edge_it = graph->begin_edge(it);

    // iterate over all edges starting from node *it
    // and push destination to stack if node not visited
    while (edge_it != graph->end_edge(it)) {
      uint64_t destination = (*edge_it).second;

      {
        std::unique_lock<std::mutex> lk(visit_node_mutex);

        if (visited->find(destination) == visited->end()) {
          visited->insert({destination, true});
          pool->push(destination);
        }

        distances->at(destination) = std::min(distances->at(destination), distances->at(current_node) + 1);

      }

      ++edge_it;
    }
  }


}

template<typename T>
bool GraphAlgorithm<T>::IsWeaklyConnectedParallel(std::shared_ptr<Graph<T>> graph, ThreadPool &thread_pool) {

  graph = transform_graph_to_undirected(graph);

  return DepthFirstSearchParallel(graph, thread_pool);
}

template<typename T>
bool GraphAlgorithm<T>::IsFullyConnectedParallel(std::shared_ptr<Graph<T>> graph, ThreadPool &thread_pool) {

  std::unordered_map<uint64_t, std::unordered_map<uint64_t, bool>> &edges = graph->get_all_edges();
  uint64_t num_distinct_edges = graph->get_nodes().size() - 1; // return |V|-1  where graph(V,E)
  std::shared_ptr<bool> is_fully_connected(new bool(true));

  // we verify first that there is at least one edge starting from each node
  if (edges.size() != graph->get_nodes().size())
    return false;

  uint64_t bucket_count = edges.bucket_count();
  // divide the number of bucket by available cores
  // if the number of buckets < num core
  // adopt mono thread
  uint64_t window = bucket_count / std::thread::hardware_concurrency();
  uint64_t num_threads;
  if (window != 0)
    num_threads = std::thread::hardware_concurrency();
  else
    num_threads = 1;
  // the number of buckets isn't necessary a multiple of available concurrent cores
  uint64_t add = bucket_count % std::thread::hardware_concurrency();

  uint64_t start = 0, end = window + add;

  for (int i = 0; i < num_threads; ++i) {

    thread_pool.doJob("FullyConnected", std::bind(&GraphAlgorithm<T>::is_fully_connected_helper, this,
                                                  start, end, std::ref(edges), num_distinct_edges,
                                                  is_fully_connected
    ));
    start = end;
    end = start + window - 1;
  }

  // wait that the job launched finishes
  while (!thread_pool.isFinished("FullyConnected"))
    continue;

  return *is_fully_connected;
}

template<typename T>
std::shared_ptr<std::unordered_map<uint64_t, uint64_t>> GraphAlgorithm<T>::GetShortestDistancesParallel(std::shared_ptr<Graph<T>> graph,
                                                                                                        uint64_t start_idx,
                                                                                                        ThreadPool &thread_pool) {
  // verify that the node exist otherwise return null
  if (graph->get_nodes().find(start_idx) == graph->get_nodes().end())
    return NULL;
  // push both to the heap of the process
  // better than sharing thread's stack memory adresses

  std::shared_ptr<std::unordered_map<uint64_t, bool>> visited(new std::unordered_map<uint64_t, bool>);

  std::shared_ptr<std::unordered_map<uint64_t, uint64_t>> distances(new std::unordered_map<uint64_t, uint64_t>);
  // init distances to MAX value as they are not reacheable
  // it's possible to do it in parallel using bucket_count since unordered map iterator
  // are Forward type and not RAI
  for (auto it = graph->get_nodes().begin(); it != graph->get_nodes().end(); ++it)
    distances->insert({it->first, UINT64_MAX});

  std::shared_ptr<int> num_blocked_thread(new int());
  std::shared_ptr<std::queue<uint64_t>> queue_pool(new std::queue<uint64_t>());
  std::shared_ptr<bool> stop_program(new bool());


  // init start node
  uint64_t start_node = start_idx;
  distances->at(start_node) = 0;

  // init the stack with a start element
  queue_pool->push(start_node);
  visited->insert({start_node, true});

  // launch conccurent jobs
  for (int i = 0; i < std::thread::hardware_concurrency(); ++i) {
    thread_pool.doJob("distances", std::bind(&GraphAlgorithm<T>::consume_nodes_distance, this,
                                             graph, queue_pool, visited, distances, num_blocked_thread,
                                             stop_program));

  }

  // wait for jobs to finish
  while (!thread_pool.isFinished("distances"))
    continue;

  // at this point only the main thread is running
  // it's safe to get the size of the hash table
  return distances;

}

#endif /* GraphAlgorithm_hpp */
