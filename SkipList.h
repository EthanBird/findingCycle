
#ifndef TEST_SKIPLIST_H
#define TEST_SKIPLIST_H


/* ************************************************************************
> File Name:     skiplist.h
> Created Time:  Sun Dec  2 19:04:26 2018
> Description:
 ************************************************************************/

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <mutex>
#include <fstream>
#include <vector>
#include <set>

#define STORE_FILE "store/dumpFile"



//Class template to implement node
template<typename K, typename V>
class ListNode {

public:

    ListNode() {}

    ListNode(K k, V v, int);

    ~ListNode();

    K get_key() const;

    V get_value() const;

    void set_value(V);


    // Linear array to hold pointers to next node of different level
    ListNode<K, V> **forward;

    int node_level;

    struct node_cmp{ // special for edge_ptr
        bool operator() (V v1, V v2) const {

            if(v1->timestamp == v2->timestamp){
//                if(v1->from->id == v2->from->id){
                    return v1->to->id < v2->to->id;
//                }
//                return v1->from->id < v2->from->id;
            }
            return v1->timestamp < v2->timestamp;
        }
    };
    std::set<V, node_cmp> values;

private:

    K key;

//    V value;
};

template<typename K, typename V>
ListNode<K, V>::ListNode(const K k, const V v, int level) {

    this->key = k;
//    this->value = v;
    if(v != NULL)
        this->values.insert(v);
    this->node_level = level;

    // level + 1, because array index is from 0 - level
    this->forward = new ListNode<K, V>*[level + 1];

    // Fill forward array with 0(NULL)
    memset(this->forward, 0, sizeof(ListNode<K, V>*) * (level + 1));
};

template<typename K, typename V>
ListNode<K, V>::~ListNode() {
    delete []forward;
};

template<typename K, typename V>
K ListNode<K, V>::get_key() const {
    return key;
};

//template<typename K, typename V>
//V ListNode<K, V>::get_value() const {
//    return value;
//};
template<typename K, typename V>
void ListNode<K, V>::set_value(V value) {
    this->value=value;
};

// Class template for Skip list
template <typename K, typename V>
class SkipList {

public:
    SkipList(int);
    ~SkipList();
    int get_random_level();
    ListNode<K, V>* create_node(K, V, int);
    int insert_element(K, V);

    int emplace_back(V v){
        mtx.lock();
        int  code = insert_element(v->amount, v);
        mtx.unlock();
        return code;
    }
    void display_list();
    bool search_element(K);
    void delete_element(K);
    void dump_file();
    void load_file();

    ListNode<K, V>* startOf(K);
    ListNode<K, V>*  endOf(K);
    ListNode<K, V>*  get_header(){
        return _header;
    };
    std::vector<V>* getRange(K, K);
    int size();

private:
    void get_key_value_from_string(const std::string& str, std::string* key, std::string* value);
    bool is_valid_string(const std::string& str);

private:
    std::mutex mtx;     // mutex for critical section
    std::string delimiter = ":";
    // Maximum level of the skip list
    int _max_level;

    // current level of skip list
    int _skip_list_level;

    // pointer to header node
    ListNode<K, V> *_header;

    // file operator
    std::ofstream _file_writer;
    std::ifstream _file_reader;

    // skiplist current element count
    int _element_count;
};

// create new node
template<typename K, typename V>
ListNode<K, V>* SkipList<K, V>::create_node(const K k, const V v, int level) {
    ListNode<K, V> *n = new ListNode<K, V>(k, v, level);
    return n;
}

// Insert given key and value in skip list
// return 1 means element exists
// return 0 means insert successfully
/*
                           +------------+
                           |  insert 50 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |                      insert +----+
level 3         1+-------->10+---------------> | 50 |          70       100
                                               |    |
                                               |    |
level 2         1          10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 1         1    4     10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 0         1    4   9 10         30   40  | 50 |  60      70       100
                                               +----+

*/
template<typename K, typename V>
int SkipList<K, V>::insert_element(const K key, const V value) {


    ListNode<K, V> *current = this->_header;

    // create update array and initialize it
    // update is array which put node that the node->forward[i] should be operated later
    ListNode<K, V> *update[_max_level + 1];
    memset(update, 0, sizeof(ListNode<K, V>*) * (_max_level + 1));

    // start form highest level of skip list
    for(int i = _skip_list_level; i >= 0; i--) {
        while(current->forward[i] != NULL && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
         update[i] = current;
    }

    // reached level 0 and forward pointer to right node, which is desired to insert key.
    current = current->forward[0];

    // if current node have key equal to searched key, we get it
    if (current != NULL && current->get_key() == key) {
        // std::cout << "key: " << key << ", exists" << std::endl;
        current->values.insert(value);
//        ListNode<K, V>* inserted_node = create_node(key, value, 0);
//        inserted_node->forward[0] = current->forward[0];
//        current->forward[0] = inserted_node;
        _element_count ++;

        return 1;
    }

    // if current is NULL that means we have reached to end of the level
    // if current's key is not equal to key that means we have to insert node between update[0] and current node
    if (current == NULL || current->get_key() != key ) {
        // Generate a random level for node
        int random_level = get_random_level();;


        // If random level is greater thar skip list's current level, initialize update value with pointer to header
        if (random_level > _skip_list_level) {
            for (int i = _skip_list_level+1; i < random_level+1; i++) {
                update[i] = _header;
            }
            _skip_list_level = random_level;
        }

        // create new node with random level generated
        ListNode<K, V>* inserted_node = create_node(key, value, random_level);

        // insert node
        for (int i = 0; i <= random_level; i++) {
            inserted_node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = inserted_node;
        }
        // std::cout << "Successfully inserted key:" << key << ", value:" << value << std::endl;
        _element_count ++;
    }

    return 0;
}

// Display skip list
template<typename K, typename V>
void SkipList<K, V>::display_list() {

    std::cout << "\n*****Skip List*****"<<"\n";
    for (int i = 0; i <= _skip_list_level; i++) {
        ListNode<K, V> *node = this->_header->forward[i];
        std::cout << "Level " << i << ": ";
        while (node != NULL) {
            std::cout << node->get_key() << ":" ;
            // Test
            for(auto it = node->values.begin(); it != node->values.end(); it++){
                std::cout<< (*it)->timestamp << ",";
            }
            std::cout <<";";
            node = node->forward[i];
        }
        std::cout << std::endl;
    }
}

// Dump data in memory to file
template<typename K, typename V>
void SkipList<K, V>::dump_file() {

    std::cout << "dump_file-----------------" << std::endl;
//    _file_writer.open(STORE_FILE);
    _file_writer.open("H:/data/skiplist/"+std::to_string(time(0)));
    ListNode<K, V> *node = this->_header->forward[0];

    while (node != NULL) {
        _file_writer << node->get_key() << ":" << node->values.size() << "\n";
        // std::cout << node->get_key() << ":" << node->values.size() << ";\n";
        node = node->forward[0];
    }

    _file_writer.flush();
    _file_writer.close();
    return ;
}

// Load data from disk
template<typename K, typename V>
void SkipList<K, V>::load_file() {

    _file_reader.open(STORE_FILE);
    std::cout << "load_file-----------------" << std::endl;
    std::string line;
    std::string* key = new std::string();
    std::string* value = new std::string();
    while (getline(_file_reader, line)) {
        get_key_value_from_string(line, key, value);
        if (key->empty() || value->empty()) {
            continue;
        }
        insert_element(*key, *value);
        std::cout << "key:" << *key << "value:" << *value << std::endl;
    }
    _file_reader.close();
}

// Get current SkipList size
template<typename K, typename V>
int SkipList<K, V>::size() {
    return _element_count;
}

template<typename K, typename V>
void SkipList<K, V>::get_key_value_from_string(const std::string& str, std::string* key, std::string* value) {

    if(!is_valid_string(str)) {
        return;
    }
    *key = str.substr(0, str.find(delimiter));
    *value = str.substr(str.find(delimiter)+1, str.length());
}

template<typename K, typename V>
bool SkipList<K, V>::is_valid_string(const std::string& str) {

    if (str.empty()) {
        return false;
    }
    if (str.find(delimiter) == std::string::npos) {
        return false;
    }
    return true;
}

// Delete element from skip list
template<typename K, typename V>
void SkipList<K, V>::delete_element(K key) {

    mtx.lock();
    ListNode<K, V> *current = this->_header;
    ListNode<K, V> *update[_max_level + 1];
    memset(update, 0, sizeof(ListNode<K, V>*) * (_max_level + 1));

    // start from highest level of skip list
    for (int i = _skip_list_level; i >= 0; i--) {
        while (current->forward[i] !=NULL && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
        update[i] = current;
    }

    current = current->forward[0];
    if (current != NULL && current->get_key() == key) {

        // start for lowest level and delete the current node of each level
        for (int i = 0; i <= _skip_list_level; i++) {

            // if at level i, next node is not target node, break the loop.
            if (update[i]->forward[i] != current)
                break;

            update[i]->forward[i] = current->forward[i];
        }

        // Remove levels which have no elements
        while (_skip_list_level > 0 && _header->forward[_skip_list_level] == 0) {
            _skip_list_level --;
        }

        std::cout << "Successfully deleted key "<< key << std::endl;
        _element_count --;
    }
    mtx.unlock();
    return;
}

// Search for element in skip list
/*
                           +------------+
                           |  select 60 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |
level 3         1+-------->10+------------------>50+           70       100
                                                   |
                                                   |
level 2         1          10         30         50|           70       100
                                                   |
                                                   |
level 1         1    4     10         30         50|           70       100
                                                   |
                                                   |
level 0         1    4   9 10         30   40    50+-->60      70       100
*/
template<typename K, typename V>
bool SkipList<K, V>::search_element(K key) {

    std::cout << "search_element-----------------" << std::endl;
    ListNode<K, V> *current = _header;

    // start from highest level of skip list
    for (int i = _skip_list_level; i >= 0; i--) {
        while (current->forward[i] && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
    }

    //reached level 0 and advance pointer to right node, which we search
    current = current->forward[0];

    // if current node have key equal to searched key, we get it
    if (current and current->get_key() == key) {
        std::cout << "Found key: " << key << ", value: " << current->get_value() << std::endl;
        return true;
    }

    std::cout << "Not Found Key:" << key << std::endl;
    return false;
}

// construct skip list
template<typename K, typename V>
SkipList<K, V>::SkipList(int max_level) {

    this->_max_level = max_level;
    this->_skip_list_level = 0;
    this->_element_count = 0;

    // create header node and initialize key and value to null
    K k ;
    V v ;
    this->_header = new ListNode<K, V>(k, v, _max_level);
};

template<typename K, typename V>
SkipList<K, V>::~SkipList() {

    if (_file_writer.is_open()) {
        _file_writer.close();
    }
    if (_file_reader.is_open()) {
        _file_reader.close();
    }
    delete _header;
}

template<typename K, typename V>
int SkipList<K, V>::get_random_level(){

    int k = 1;
    while (rand() % 2) {
        k++;
    }
    k = (k < _max_level) ? k : _max_level;
    return k;
}

template<typename K, typename V>
ListNode<K, V>* SkipList<K, V>::startOf(K key) {
    ListNode<K, V> *current = _header;

    // start from highest level of skip list
    for (int i = _skip_list_level; i >= 0; i--) {
        while (current->forward[i] != NULL && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
    }

    //reached level 0 and advance pointer to right node, which we search
    current = current->forward[0];
    return current;
}

template<typename K, typename V>
ListNode<K, V>* SkipList<K, V>::endOf(K key) {
    ListNode<K, V>* node = startOf(key);
    if(node == NULL) return node;
    while(node->forward[0] != NULL && node->forward[0]->get_key() == key){
        node = node->forward[0];
    }
    return node;
}


template<typename K, typename V>
std::vector<V>* SkipList<K, V>::getRange(K left, K right) {
    ListNode<K, V>* leftNode = startOf(left);
    ListNode<K, V>* rightNode = endOf(right);

    std::vector<V> *ret = new std::vector<V>;
    if(left > right || leftNode == NULL){
        return ret;
    }
    if(rightNode == NULL){
        while(leftNode != NULL ){
            ret->push_back(leftNode->get_value());
            leftNode = leftNode->forward[0];
        }
        return ret;
    }
    if(rightNode->get_key() <= right){
        rightNode = rightNode->forward[0];
    }
    while( leftNode != rightNode){
        ret->push_back(leftNode->get_value());
        leftNode = leftNode->forward[0];

    }
    return ret;
}


// vim: et tw=100 ts=4 sw=4 cc=120
#endif //TEST_SKIPLIST_H
