#include "database.cpp"
#include <pqxx/pqxx>

using namespace pqxx;

void split(vector<string>& tokens, string str, const char delimiter[], bool first) {
    char *dup = strdup(str.c_str());
    char *token = strtok(dup, delimiter);

    tokens.clear();

    if(first) {
        tokens.push_back(string(token));

        token = strtok(NULL, "\n");
        tokens.push_back(string(token));

        return;
    }

    while(token != NULL) {
        tokens.push_back(string(token));

        token = strtok(NULL, delimiter);
    }

    free(dup);
}

void normalize(string& str) {
    size_t found;

    found = str.find("'");
    while(found != string::npos) {
        str.insert(found, "'");
        found = str.find("'", found+2);
    }
}

bool is_number(const char str[]) {
    int len = strlen(str);
    int is_num = true;

    for (int i = 0; i < len; i++) {
        if(!isdigit(str[i])) {
            is_num = false;
            break;
        }
    }

    return is_num;
}

int read_id(ifstream& file) {
    string buffer;
    vector<string> tokens;

    getline(file, buffer);
    split(tokens, buffer, ": ", false);
    return stoi(tokens[1]);
}

string read_asin(ifstream& file) {
    string buffer;
    vector<string> tokens;

    getline(file, buffer);
    split(tokens, buffer, ": ", false);
    return tokens[1].substr(0, tokens[1].size()-1);
}

string read_title(ifstream& file) {
    string buffer;
    vector<string> tokens;

    getline(file, buffer);
    split(tokens, buffer, " ", false);
    if(tokens[0].compare("discontinued") == 0) {
        return string();
    }
    split(tokens, buffer, " ", true);
    normalize(tokens[1]);

    return tokens[1].substr(0, tokens[1].size()-1);;
}

string read_group(ifstream& file) {
    string buffer;
    vector<string> tokens;

    getline(file, buffer);
    split(tokens, buffer, " \r", true);
    return tokens[1].substr(0, tokens[1].size()-1);;
}

int read_salesrank(ifstream& file) {
    string buffer;
    vector<string> tokens;

    getline(file, buffer);
    split(tokens, buffer, " ", false);
    return stoi(tokens[1]);
}

void read_similars(vector<Similar>& similars, ifstream& file, string prod_asin) {
    string buffer;
    vector<string> tokens;
    Similar similar;
    int total_similars;

    getline(file, buffer);
    split(tokens, buffer, " ", false);
    total_similars = stoi(tokens[1]);

    for (int i = 2; i < total_similars+2; i++) {
        similar.asin_product = prod_asin;
        similar.asin_similar = tokens[i];
        similars.push_back(similar);
    }
}

void read_categories(vector<int>& categories, ifstream& file, unordered_map<int, bool>& hash_categories, work& transaction) {
    string buffer;
    vector<string> tokens;
    int total_categories;

    getline(file, buffer);
    split(tokens, buffer, " ", false);

    total_categories = stoi(tokens[1]);

    for (int i = 0; i < total_categories; i++) {
        getline(file, buffer);
        split(tokens, buffer, "|[]", false);

        int inicio = 3;

        Category category;

        category.parent_id = 0;
        if(is_number(tokens[1].c_str())) {
            category.id = stoi(tokens[1]);
            category.name = "";
            inicio = 2;
        } else {
            category.id = stoi(tokens[2]);
            category.name = tokens[1];
        }

        if(hash_categories.count(category.id) == 0) {
            hash_categories[category.id] = true;
            insert_category(transaction, category);
        }

        for (int j = inicio; j < tokens.size()-1; j+=2) {
            category.parent_id = category.id;
            category.name = tokens[j];
            normalize(category.name);

            if(is_number(tokens[j+1].c_str())) {
                category.id = stoi(tokens[j+1]);
            } else {
                category.name += "[" + tokens[j+1] + "]";
                category.id = stoi(tokens[j+2]);
                j += 1;
            }

            if(hash_categories.count(category.id) == 0) {
                hash_categories[category.id] = true;
                insert_category(transaction, category);
            }
        }
        categories.push_back(category.id);
    }
}

int read_reviews(vector<Review>& reviews, ifstream& file) {
    string buffer;
    vector<string> tokens;
    int total_reviews;
    Review review_record;

    getline(file, buffer);
    split(tokens, buffer, " ", false);

    total_reviews = stoi(tokens[4]);

    for (int i = 0; i < total_reviews; i++){
        getline(file, buffer);
        split(tokens, buffer, " ", false);
        strcpy(review_record.date, tokens[0].c_str());
        review_record.costumer = tokens[2];
        review_record.rating = stoi(tokens[4]);
        review_record.votes = stoi(tokens[6]);
        review_record.helpful = stoi(tokens[8]);
        reviews.push_back(review_record);
    }

    return total_reviews;
}

void read_data(const char filename[], vector<Similar>& similars, unordered_map<int, bool>& hash_categories, connection& conn) {
    ifstream file(filename);
    string buffer;

    int total_records;
    int count_records = 0;

    vector<string> tokens;

    // ignoring first comment
    getline(file, buffer);

    // getting number of records
    getline(file, buffer);
    split(tokens, buffer, " ", false);
    total_records = stoi(tokens[2]);

    printf("total records: %d\n", total_records);

    while (count_records < total_records) {
        Product record;
        bool discontinued;
        int total_reviews = 0;
        work transaction(conn);

        count_records += 1;

        getline(file, buffer);

        record.id = read_id(file);
        record.asin = read_asin(file);
        record.title = read_title(file);

        discontinued = record.title.empty();
        //cout << record.id << "\n" << record.asin << "\n" << record.title << "\n";
        if(!discontinued) {
            record.group = read_group(file);
            record.salesrank = read_salesrank(file);
            read_similars(similars, file, record.asin);
            read_categories(record.categories, file, hash_categories, transaction);
            total_reviews = read_reviews(record.reviews, file);
        }

        insert_product(transaction, record);

        for (auto category:record.categories) {
            insert_product_category(transaction, record.asin, category);
        }

        if(total_reviews > 0) {
            insert_reviews(transaction, record);
        }

        transaction.commit();

        cout << " Records: " << count_records << '\n';
      }

      work transaction(conn);
      insert_similars(transaction, similars);
      transaction.commit();
}

int main() {
    vector<Similar> similars(5000);
    unordered_map<int, bool> categories;

    connection conn("dbname=trab1 user=anavi password=admin hostaddr=127.0.0.1 port=5432");
    if (conn.is_open()) {
        cout << "Opened database successfully: " << conn.dbname() << "\n";

    } else {
        cout << "Can't open database\n";
    }

     create_tables(conn);
    read_data("amazon-meta.txt", similars, categories, conn);
}
