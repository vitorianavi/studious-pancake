#ifndef __LIB_DATABASE__
#define __LIB_DATABASE__

#include "geral.hpp"

void create_tables(connection& conn);
void insert_category(work& transaction, Category category);
void insert_product_category(work& transaction, string asin_product, int id_category);
void insert_similar(work& transaction, Similar similar);
void insert_reviews(work& transaction, Product product);
void insert_product(work& transaction, Product product);

#endif
