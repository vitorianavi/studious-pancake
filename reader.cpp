#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>
#include <unordered_map>

using namespace std;

typedef struct {
    string name;
    int parent_id;
} Category;

typedef struct {
    char date[11];
    string customer;
    int rating;
    int votes;
    int helpful;
} Review;

typedef struct {
    int id;
    string asin;
    string title;
    string group;
    int salesrank;
    int downloaded;
    float avg_rating;
    vector<string> similars;
    vector<Category> categories;
    vector<Review> reviews;
} Product;

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

void read_data(const char filename[]) {
    ifstream file(filename);
    string buffer;
    Product record;

    int total_records;
    int count_records = 0;
    int total_similars;
    int total_categories;
    int total_reviews;

    vector<string> tokens;
    unordered_map<int, Category> categories;

    // ignoring first comment
    getline(file, buffer);

    // getting number of records
    getline(file, buffer);
    split(tokens, buffer, " ", false);
    total_records = stoi(tokens[2]);

    printf("total records: %d\n", total_records);

    while (count_records < total_records) {
        getline(file, buffer);
            //    cout << buffer << "\n";

        // reading id
        getline(file, buffer);
        split(tokens, buffer, ":", false);
        printf("id: %s ", tokens[1].c_str());
        record.id = stoi(tokens[1]);

        // reading asin
        getline(file, buffer);
        split(tokens, buffer, ":", false);
        // printf("asin: %s", tokens[1].c_str());
        // std::cout << std::endl;
        record.asin = tokens[1];

        // reading title
        getline(file, buffer);
        split(tokens, buffer, " ", false);
        if(tokens[0].compare("discontinued") == 0) {
          count_records ++;
            continue;
        }
        split(tokens, buffer, " ", true);
        // printf("title: %s", tokens[1].c_str());
        // std::cout << std::endl;
        record.title = tokens[1];

        // reading group
        getline(file, buffer);
        split(tokens, buffer, " ", true);
        // printf("group: %s", tokens[1].c_str());
        // std::cout << std::endl;
        record.group = tokens[1];

        // reading salesrank
        getline(file, buffer);
        split(tokens, buffer, " ", false);
        // printf("salesrank: %s", tokens[1].c_str());
        // std::cout << std::endl;
        record.salesrank = stoi(tokens[1]);

        // reading similars
        getline(file, buffer);
        split(tokens, buffer, " ", false);
    //    cout << buffer << "\n";
        total_similars = stoi(tokens[1]);
        // printf("similars: %d ", total_similars);

        if(total_similars != 0) {
            for (int i = 2; i < total_similars+2; i++){
                //printf("%s ", tokens[i].c_str());
                record.similars.push_back(tokens[i]);
            }
        }

        cout << "\n";

        //reading categories
        getline(file, buffer);
        split(tokens, buffer, " ", false);
    //    cout << buffer << "\n";
        total_categories = stoi(tokens[1]);

        // printf("categories: %d\n", total_categories);

        if(total_categories != 0) {

            for (int i = 0; i < total_categories; i++) {
                getline(file, buffer);
                split(tokens, buffer, "|[]", false);

            /*    // indica que a primeira categoria não tem nome
                int odd = false;
                int inicio = 3;
                if(tokens.size()%2 != 0) odd = true;*/
                int inicio = 3;

                int id;
                Category category;

                category.parent_id = 0;
                if(is_number(tokens[1].c_str())) {
                    // cout << "odd aqui\n" << tokens[0] << " " << tokens[1] << "\n";
                    id = stoi(tokens[1]);
                    category.name = "";
                    inicio = 2;
                } else {
                    id = stoi(tokens[2]);;
                    category.name = tokens[1];
                }
                categories[id] = category;

                // cout << "id " << id << " name " << category.name << " parent " << category.parent_id << "\n";

                for (int j = inicio; j < tokens.size()-1; j+=2) {
                    category.parent_id = id;
                    category.name = tokens[j];
                    if(is_number(tokens[j+1].c_str())) {
                        // printf("entrou 1 if\n");
                        id = stoi(tokens[j+1]);
                    } else {
                        // printf("entrou 2 if\n");
                        category.name += "[" + tokens[j+1] + "]";
                        cout << tokens[j+2] << "\n";
                        id = stoi(tokens[j+2]);
                        j += 1;
                    }
                    // cout << "id " << id << " name " << category.name << " parent " << category.parent_id << "\n";
                    categories[id] = category;
                }
            }
        }

        // reading reviews
        Review review_record;
        getline(file, buffer);
        split(tokens, buffer, " ", false);

        // cout << "total reviews: " << tokens[2] << " downloaded: " << tokens[4] << " avg_rating: " << tokens[7] << "\n";
        total_reviews = stoi(tokens[4]);

        if(total_reviews != 0) {

            record.downloaded = stoi(tokens[4]);
            record.avg_rating = stoi(tokens[7]);
            for (int i = 0; i < total_reviews; i++){
                getline(file, buffer);
                split(tokens, buffer, " ", false);
                strcpy(review_record.date, tokens[0].c_str());
                review_record.customer = tokens[2];
                review_record.rating = stoi(tokens[4]);
                review_record.votes = stoi(tokens[6]);
                review_record.helpful = stoi(tokens[8]);
                // cout << review_record.date << " customer: " << review_record.customer << " rating: " << review_record.rating << "  votes: " << review_record.votes << " helpful: "<< review_record.helpful << "\n";
                // cout << review_record.helpful << "\n";
                record.reviews.push_back(review_record);
            }
        }
        //printf("%s\n", buffer);
        buffer.clear();
        count_records++;
        printf("count_records: %d\n", count_records);
      }
}


int main() {
    read_data("amazon-meta.txt");
}
