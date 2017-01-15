//
//  Graph.hpp
//  SimpleGraph
//
//  Created by Abderrahmane on 18/08/2016.
//  Copyright © 2016 Ecole Mohammadia d'Ingieurs. All rights reserved.
//

#ifndef Graph_h
#define Graph_h

#include <unordered_map>
#include <vector>
#include <utility>
#include <iterator>



template <typename T>
class Graph {
    typedef std::unordered_map<uint64_t, T> list_nodes;
    typedef std::unordered_map<uint64_t, std::unordered_map<uint64_t, bool>> matrix_edge;
    
private:
    list_nodes nodes_;
    matrix_edge edge_;
public:
    
    Graph() {};
    
    std::string export_node_property_to_string();

    T get_node_property(uint64_t n);
    void set_node_property(uint64_t n, T val);
    
    void add_edge(uint64_t source, uint64_t destination);
    void remove_edge(uint64_t source, uint64_t destination);

  list_nodes &get_nodes(); // return reference to the list of vertices
    void insert_node(uint64_t id);
    void set_nodes(list_nodes& g);
    
    void set_all_edges(matrix_edge& m);
    matrix_edge& get_all_edges(); // return a reference to the edge matrix representation
    
    // iterators
    class NodeIter {
        
        list_nodes& ref_nodes_;
        uint64_t index_;
        
        public :
        typedef NodeIter self_type;
        typedef uint64_t value_type;
        typedef NodeIter& reference;
        typedef NodeIter* pointer;
        
        // constructor
        NodeIter(list_nodes& list, uint64_t n) : ref_nodes_(list), index_(n) {}
        
        // copy constructor
        NodeIter(const self_type& node) : ref_nodes_(node.ref_nodes_), index_(node.index_) {}
        
        //~NodeIter();
        
        // assign operator
        self_type& operator= (const self_type& node) {
            index_ = node.index_;
            return *this;
        }
        
        // pre increment operator
        self_type& operator++() {
            auto it = ref_nodes_.find(index_);
            if (it != ref_nodes_.end() && ++it != ref_nodes_.end())
                index_ = it->first;
            else
                index_ = -1;
            
            return *this;
        }
        
        // ppost increment operator
        self_type operator++(int) {
            index_++;
            return index_;
        }
        
        // dereference operator
        value_type& operator*() {
            return index_;
        }
        
        pointer operator->() {
            return this;
        }
        
        bool operator== (const self_type& node) const { return index_ == node.index_; }
        bool operator!= (const self_type& node) const { return index_ != node.index_; }
        
    };

    class EdgeIter {
        
        matrix_edge& edges_;
        std::pair<uint64_t, uint64_t> pair_;
        
        public :
        
        typedef EdgeIter self_type;
        typedef std::pair<uint64_t, uint64_t> value_type;
        typedef EdgeIter& reference;
        typedef EdgeIter* pointer;
        
        
        EdgeIter(matrix_edge& edges, NodeIter it, bool end = false) : edges_(edges) {
            pair_.first = (*it); // return index to the current vertice (source of the edge)
            
            if (end || edges_[*it].size() == 0) {
                pair_.second = -1; // *it doesn't have any adjacent vertices or already visited them
            } else {
                pair_.second = edges_[*it].begin()->first; // delegate and use hash table iterator
            }
        }
        
        EdgeIter(const self_type& edge_it) : edges_(edge_it.edges_), pair_(edge_it.pair_) {}
        
        // ~EdgeIter();
        
        self_type& operator= (const self_type& edge_it) {
            // use built in assign operator for both containers std::hash table and std::pair
            edges_ = edge_it.edges_;
            pair_ = edge_it.pair_;
            return *this;
        }
        
        self_type& operator++() {
            // iterator of adjacent node which has pair_.first as a source
            auto it = edges_[pair_.first].find(pair_.second);
            ++it;
            // increment iterator of adjacent nodes and return if valid
            if (it != edges_[pair_.first].end())
                pair_.second = it->first;
            else
                pair_.second = -1;
            
            return *this;
        }
        
        value_type& operator*() {
            return pair_;
        }
        
        //friend void swap(self_type& lhs, self_type& rhs); //C++11
        pointer operator->() {
            return this;
        }
        
        bool operator== (const self_type& edge_it) const { return pair_ == edge_it.pair_ && edges_ == edge_it.edges_; }
        bool operator!= (const self_type& edge_it) const { return pair_ != edge_it.pair_ || edges_ != edge_it.edges_; }
        
    };
    

    NodeIter begin() { return nodes_.size() != 0 ?
        NodeIter(nodes_, nodes_.begin()->first) : NodeIter(nodes_, -1); }
    NodeIter end() { return NodeIter(nodes_, -1); }
    
    
    EdgeIter begin_edge(NodeIter node_it) { return EdgeIter(edge_, node_it); }
    EdgeIter end_edge(NodeIter node_it) { return EdgeIter(edge_, node_it, true); }
};



template< typename T>
std::string  Graph<T>::export_node_property_to_string() {
    std::string result;
    
    for (auto i = begin() ; i != end() ; ++i) {
        result += "node_id_" + std::to_string(*i) + " : " + get_node_property(*i) + "\n";
    }
    
    return result;
}


template< typename T>
std::unordered_map<uint64_t, T>& Graph<T>::get_nodes() {
    return nodes_; // return ref to the vertices list 
}

template <typename T>
void Graph<T>::insert_node(uint64_t id) {
    // an element is inserted if it doesn't exist with no property value
    nodes_[id];
}

template< typename T>
void Graph<T>::set_nodes(list_nodes& g) {
    nodes_ = g; // copy assignement operator linear time
}


template< typename T>
void Graph<T>::set_all_edges(matrix_edge& m) {
    edge_ = m; // copy assignement operator linear time
}

template< typename T>
 std::unordered_map<uint64_t, std::unordered_map<uint64_t, bool>>& Graph<T>::get_all_edges() {
    return edge_;
}

template< typename T>
T Graph<T>::get_node_property(uint64_t n) {
    auto it = get_nodes().find(n);
    if (it != get_nodes().end())
        return nodes_[n];
    else
        return NULL;
}

template< typename T>
void Graph<T>::set_node_property(uint64_t n, T val) {
    nodes_[n] = val;
}

template< typename T>
void Graph<T>::add_edge(uint64_t source, uint64_t destination) {
    edge_[source][destination] = true;
}

template< typename T>
void Graph<T>::remove_edge(uint64_t source, uint64_t destination) {
    // there is the posibility to set it to false and avoid rehashing
    // needs to be investigated deeply
    edge_[source].erase(destination);
}


#endif /* Graph_h */
