#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>
#include <unordered_map>
#include <pqxx/pqxx>

using namespace std;
using namespace pqxx;

typedef struct {
    string name;
    int parent_id;
} Category;

typedef struct {
    char date[11];
    string costumer;
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
    int total_reviews;
    int downloaded;
    float avg_rating;
    vector<Review> reviews;
} Product;

typedef struct {
    char asin_product[15];
    char asin_similar[15];
} Similar;

typedef struct {
    char asin_product[15];
    int id_category;
} Product_category;


void create_tables(connection& conn) {
    string sql_create_category;
    string sql_create_product;
    string sql_create_product_category;
    string sql_create_product_similar;
    string sql_create_review;
    work transaction(conn);

    sql_create_category = "CREATE TABLE CATEGORY(" \
    "ID INT PRIMARY KEY NOT NULL, " \
    "NAME VARCHAR(50), "\
    "ID_PARENT INT, "\
    "FOREIGN KEY (ID_PARENT) REFERENCES CATEGORY(ID));";

    sql_create_product = "CREATE TABLE PRODUCT(" \
    "ASIN VARCHAR(15) PRIMARY KEY NOT NULL, "\
    "TITLE TEXT NOT NULL, "\
    "PRODUCT_GROUP VARCHAR(30), "\
    "SALESRANK INT, "\
    "AVG_RATING FLOAT, "\
    "TOTAL_REVIEWS INT, "\
    "DOWNLOADED INT);";

    sql_create_product_category = "CREATE TABLE PRODUCT_CATEGORY(" \
    "ASIN_PRODUCT VARCHAR(15) NOT NULL, " \
    "ID_CATEGORY INT NOT NULL, "\
    "PRIMARY KEY (ASIN_PRODUCT, ID_CATEGORY), " \
    "FOREIGN KEY (ASIN_PRODUCT) REFERENCES PRODUCT(ASIN), "\
    "FOREIGN KEY (ID_CATEGORY) REFERENCES CATEGORY(ID));";

    sql_create_product_similar = "CREATE TABLE PRODUCT_SIMILAR(" \
    "ASIN_PRODUCT VARCHAR(15) NOT NULL, " \
    "ASIN_SIMILAR VARCHAR(15) NOT NULL, "\
    "PRIMARY KEY (ASIN_PRODUCT, ASIN_SIMILAR), " \
    "FOREIGN KEY (ASIN_PRODUCT) REFERENCES PRODUCT(ASIN), "\
    "FOREIGN KEY (ASIN_SIMILAR) REFERENCES PRODUCT(ASIN));";

    sql_create_review = "CREATE TABLE REVIEW(" \
    "ID SERIAL PRIMARY KEY NOT NULL, " \
    "PUBLIC_DATE DATE NOT NULL, "\
    "COSTUMER VARCHAR(15) NOT NULL, "\
    "RATING INT NOT NULL, "\
    "VOTES INT DEFAULT 0, "\
    "HELPFUL INT, "\
    "ASIN_PRODUCT VARCHAR(15) NOT NULL, "\
    "FOREIGN KEY (ASIN_PRODUCT) REFERENCES PRODUCT(ASIN));";

    transaction.exec(sql_create_category);
    cout << "Table \"category\" created successfully\n";
    transaction.exec(sql_create_product);
    cout << "Table \"product\" created successfully\n";
    transaction.exec(sql_create_product_category);
    cout << "Table \"product_category\" created successfully\n";
    transaction.exec(sql_create_product_similar);
    cout << "Table \"product_similar\" created successfully\n";
    transaction.exec(sql_create_review);
    cout << "Table \"review\" created successfully\n";
    transaction.commit();
}

void insert_category(work& transaction, int id, Category category) {
    string sql_insert;


    sql_insert = " INSERT INTO CATEGORY(ID, NAME, ID_PARENT) SELECT ";
    sql_insert += to_string(id) + ", '" + category.name + "', ";
    if(category.parent_id != 0) {
        sql_insert += to_string(category.parent_id);
    } else {
        sql_insert += "NULL";
    }
    sql_insert += " ";
    sql_insert += "WHERE NOT EXISTS (SELECT ID FROM CATEGORY WHERE id=" + to_string(id) +" );";

    transaction.exec(sql_insert);

}

void insert_product_category(work& transaction, Product_category product_category) {
    int i, size, str_length;
    string sql_insert;
    char record[50];

    size = product_category.size();

    cout << "Inserting Product_Category...\n";
    sql_insert = "INSERT INTO PRODUCT_CATEGORY (ASIN_PRODUCT, ID_CATEGORY) VALUES ";
    sprintf(record, "(%s, %d);", product_category.asin_product, product_category.id_category);
    sql_insert += to_string(record);

    transaction.exec(sql_insert);
}

void insert_similars(work& transaction, vector<Similar> similars) {
    int i, size, str_length;
    string sql_insert;
    char record[50];

    size = similars.size();

    cout << "Inserting similars...\n";
    sql_insert = "INSERT INTO PRODUCT_SIMILAR (ASIN_PRODUCT, ASIN_SIMILAR) VALUES ";
    for (i = 0; i < size; i++) {
        sprintf(record, "(%s, %s),", similars[i].asin_product, similars[i].asin_similar);
        sql_insert += to_string(record);
        }

    str_length = sql_insert.length();
    sql_insert.at(str_length-1) = ';';

   transaction.exec(sql_insert);
}

void insert_reviews(work& transaction, Product product) {
    int i, j, prod_size, review_size, str_length;
    string sql_insert;
    char record[600];

    cout << "Inserting reviews...\n";
    review_size = product.reviews.size();
    sql_insert = "INSERT INTO REVIEW (PUBLIC_DATE, COSTUMER, RATING, VOTES, HELPFUL, ASIN_PRODUCT) VALUES ";
        for (i = 0; i < review_size; i++) {
            sql_insert += "('" + to_string(product.reviews[i].date) + "', '" + product.reviews[i].costumer + "', ";
            sql_insert += to_string(product.reviews[i].rating) + ", " + to_string(product.reviews[i].votes) + ", ";
            sql_insert += to_string(product.reviews[i].helpful) + ", '" +product.asin+ "'),";
        }

        str_length = sql_insert.length();
        sql_insert.at(str_length-1) = ';';

        transaction.exec(sql_insert);
        //transaction.commit();



}

void insert_products(work& transaction, Product product) {
    int i, size;
    string sql_insert;
    //work transaction(conn);
    //cout << "Inserindo product:" << product.asin << '\n';

    sql_insert = "INSERT INTO PRODUCT (ASIN, TITLE, PRODUCT_GROUP, SALESRANK, AVG_RATING, TOTAL_REVIEWS, DOWNLOADED)";
    sql_insert += " VALUES ('" + product.asin + "', '" + product.title + "', '" + product.group + "', ";
    sql_insert += to_string(product.salesrank) + ", " + to_string(product.avg_rating) + ", ";
    sql_insert += to_string(product.total_reviews) + ", " + to_string(product.downloaded) + ");";
    transaction.exec(sql_insert);
    //transaction.commit();
    insert_reviews(transaction, product);
}

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

void read_data(const char filename[], vector<Similar>& similars, vector<Product_category>& product_category, connection& conn) {
    ifstream file(filename);
    string buffer;
    Product record;
    Product_category p_cat;

    int total_records;
    int count_records = 0;
    int total_similars;
    int total_categories;
    int total_reviews;

    vector<string> tokens;
    //vector<Product> products(100);



    // ignoring first comment
    getline(file, buffer);

    // getting number of records
    getline(file, buffer);
    split(tokens, buffer, " ", false);
    total_records = stoi(tokens[2]);

    printf("total records: %d\n", total_records);

    while (count_records < total_records) {
        work transaction(conn);
        getline(file, buffer);

        // reading id
        getline(file, buffer);
        split(tokens, buffer, ": ", false);
    //    printf("id: %s\n", tokens[1].c_str());
        record.id = stoi(tokens[1]);

        // reading asin
        getline(file, buffer);
        split(tokens, buffer, ": ", false);
        record.asin = tokens[1];
        strcpy(p_cat.asin_product, tokens[1].c_str());

        // reading title
        getline(file, buffer);
        split(tokens, buffer, " ", false);
        if(tokens[0].compare("discontinued") == 0) {
            count_records ++;
            continue;
        }
        split(tokens, buffer, " ", true);
        normalize(tokens[1]);
        record.title = tokens[1];

        // reading group
        getline(file, buffer);
        split(tokens, buffer, " ", true);
        record.group = tokens[1];

        // reading salesrank
        getline(file, buffer);
        split(tokens, buffer, " ", false);
        record.salesrank = stoi(tokens[1]);

        // reading similars
        getline(file, buffer);
        split(tokens, buffer, " ", false);
        total_similars = stoi(tokens[1]);

        Similar similar;
        if(total_similars != 0) {
            for (int i = 2; i < total_similars+2; i++){
                strcpy(similar.asin_product, record.asin.c_str());
                strcpy(similar.asin_similar, tokens[i].c_str());
                similars.push_back(similar);
            }
        }

        //reading categories
        getline(file, buffer);
        split(tokens, buffer, " ", false);

        total_categories = stoi(tokens[1]);

        if(total_categories != 0) {

            for (int i = 0; i < total_categories; i++) {
                getline(file, buffer);
                split(tokens, buffer, "|[]", false);

                int inicio = 3;

                int id;
                Category category;

                category.parent_id = 0;
                if(is_number(tokens[1].c_str())) {
                    id = stoi(tokens[1]);
                    category.name = "";
                    inicio = 2;
                } else {
                    id = stoi(tokens[2]);;
                    category.name = tokens[1];
                    p_cat.id_category = id;
                }

                insert_category(transaction, id, category);
                insert_product_category(transaction, p_);
                for (int j = inicio; j < tokens.size()-1; j+=2) {
                    category.parent_id = id;
                    category.name = tokens[j];
                    normalize(category.name);
                    if(is_number(tokens[j+1].c_str())) {
                        id = stoi(tokens[j+1]);
                        p_cat.id_category = id;
                    } else {
                        category.name += "[" + tokens[j+1] + "]";
                        id = stoi(tokens[j+2]);
                        p_cat.id_category = id;
                        j += 1;
                    }

                    insert_category(transaction, id, category);
                    product_category.push_back(p_cat);
                }
            }
        }

        // reading reviews
        Review review_record;
        getline(file, buffer);
        split(tokens, buffer, " ", false);

        total_reviews = stoi(tokens[4]);
        record.total_reviews = total_reviews;

        if(total_reviews != 0) {

            record.downloaded = stoi(tokens[4]);
            record.avg_rating = stoi(tokens[7]);
            for (int i = 0; i < total_reviews; i++){
                getline(file, buffer);
                split(tokens, buffer, " ", false);
                strcpy(review_record.date, tokens[0].c_str());
                review_record.costumer = tokens[2];
                review_record.rating = stoi(tokens[4]);
                review_record.votes = stoi(tokens[6]);
                review_record.helpful = stoi(tokens[8]);
                record.reviews.push_back(review_record);
            }
        }
        buffer.clear();

    //    insert_product(conn, record);
    //    products.push_back(record);
        if(count_records % 100 == 0 && count_records > 0) {
             //printf("count_records: %d\n", count_records);
            //insert_products(conn, products);

            //products.clear();
        }
        insert_products(transaction,record);
        transaction.commit();

        if(count_records == 10000) return;
        count_records++;
        cout << " Records: " << count_records << '\n';
      }
      work transaction(conn);
      insert_similars(transaction, similars);
    //  insert_product_category(transaction, product_category);
      transaction.commit();
}


int main() {
    vector<Similar> similars(5000);
    //unordered_map<int, product_category> product_category;

    connection conn("dbname=trab1 user=clara password=admin hostaddr=127.0.0.1 port=5432");
    if (conn.is_open()) {
        cout << "Opened database successfully: " << conn.dbname() << "\n";

    } else {
        cout << "Can't open database\n";
    }

    //create_tables(conn);
    read_data("amazon-meta.txt", similars,conn);
}
