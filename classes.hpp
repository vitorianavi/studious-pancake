#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>
#include <unordered_map>

using namespace std;

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
