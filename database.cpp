#include <pqxx/pqxx>
#include "classes.hpp"

using namespace pqxx;

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

    sql_insert = "INSERT INTO CATEGORY (ID, NAME, ID_PARENT) VALUES (";
    sql_insert += to_string(id) + ", '" + category.name + "', ";
    if(category.parent_id != 0) {
        sql_insert += to_string(category.parent_id);
    } else {
        sql_insert += "NULL";
    }
    sql_insert += ");";

    transaction.exec(sql_insert);
}

void insert_product_category(work& transaction, string asin_product, string asin_similar) {
    string sql_insert;

    cout << "Inserting Product_Category...\n";
    sql_insert = "INSERT INTO PRODUCT_CATEGORY (ASIN_PRODUCT, ID_CATEGORY) VALUES ('";
    sql_insert += asin_product + "', " + asin_similar + ");";


    transaction.exec(sql_insert);
}

void insert_similar(work& transaction, vector<Similar> similars) {
    int i, size, str_length;
    string sql_insert;

    size = similars.size();

    cout << "Inserting similars...\n";
    sql_insert = "INSERT INTO PRODUCT_SIMILAR (ASIN_PRODUCT, ASIN_SIMILAR) VALUES ";
    for (i = 0; i < size; i++) {
        sql_insert += "('" + similars[i].asin_product + "','" + similars[i].asin_similar + "'),";
    }

    str_length = sql_insert.length();
    sql_insert.at(str_length-1) = ';';

    transaction.exec(sql_insert);
}

void insert_reviews(work& transaction, Product product) {
    int i, review_size, str_length;
    string sql_insert;

    cout << "Inserting reviews...\n";
    review_size = product.reviews.size();
    sql_insert = "INSERT INTO REVIEW (PUBLIC_DATE, COSTUMER, RATING, VOTES, HELPFUL, ASIN_PRODUCT) VALUES ";
    for (i = 0; i < review_size; i++) {
        sql_insert += "('" + to_string(product.reviews[i].date) + "', '" + product.reviews[i].costumer + "', ";
        sql_insert += to_string(product.reviews[i].rating) + ", " + to_string(product.reviews[i].votes) + ", ";
        sql_insert += to_string(product.reviews[i].helpful) + ", '" + product.asin + "'),";
    }

    str_length = sql_insert.length();
    sql_insert.at(str_length-1) = ';';

    transaction.exec(sql_insert);
    //transaction.commit();
}

void insert_product(work& transaction, Product product) {
    string sql_insert;
    //work transaction(conn);
    cout << "Inserindo product:" << product.asin << '\n';

    sql_insert = "INSERT INTO PRODUCT (ASIN, TITLE, PRODUCT_GROUP, SALESRANK, DOWNLOADED)";
    sql_insert += " VALUES ('" + product.asin + "', '" + product.title + "', '" + product.group + "', ";
    sql_insert += to_string(product.salesrank) + ", " + to_string(product.downloaded) + ");";
    transaction.exec(sql_insert);
    //transaction.commit();
}
