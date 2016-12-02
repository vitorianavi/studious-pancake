#ifndef __LIB_PARSER__
#define __LIB_PARSER__

#include "geral.hpp"
#include "database.hpp"

using namespace pqxx;

void split(vector<string>& tokens, string str, const char delimiter[], bool first);
void normalize(string& str);
bool is_number(const char str[]);
int read_id(ifstream& file);
string read_asin(ifstream& file);
string read_title(ifstream& file);
string read_group(ifstream& file);
int read_salesrank(ifstream& file);
void read_similars(vector<Similar>& similars, ifstream& file, string prod_asin);
void read_categories(vector<int>& categories, ifstream& file, unordered_map<int, bool>& hash_categories, work& transaction);
int read_reviews(vector<Review>& reviews, ifstream& file);
void read_data(const char filename[], vector<Similar>& similars, unordered_map<int, bool>& hash_categories, connection& conn);

#endif
