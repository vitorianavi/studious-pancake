#ifndef __LIB_GERAL__
#define __LIB_GERAL__

#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>
#include <pqxx/pqxx>
#include <unordered_map>

using namespace std;
using namespace pqxx;

class Category {
public:
    int id;
    string name;
    int parent_id;
};

class Review {
public:
    char date[11];
    string costumer;
    int rating;
    int votes;
    int helpful;
};

class Product {
public:
    int id;
    string asin;
    string title;
    string group;
    int salesrank;
    int downloaded;
    vector<Review> reviews;
    vector<int> categories;
};

class Similar {
public:
    string asin_product;
    string asin_similar;
};

#endif
