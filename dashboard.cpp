#include <iomanip>
#include <unistd.h>
#include <sys/wait.h>
#include "geral.hpp"
#include "TextTable.h"


#define DATADIR dashboard_data/

template<typename T> void print_element(ostream& outfile, T t, const int& width) {
    outfile << "| " << left << setw(width) << setfill(' ') << t << " ";
}

void exec_cmd(char **args) {
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) {
    // chama o processo filho que irá executar o comando
        if (execvp(args[0], args) == -1) {
            perror(args[0]);
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // erro ao chamar processo filho
        perror(args[0]);
    } else {
        // Faz o processo pai esperar pela execução do processo filho
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
}

void remove_file(char *filename) {
    char *args[3];
    char rm[] = "rm";

    args[0] = rm;
    args[1] = filename;
    args[2] = NULL;

    exec_cmd(args);
}

void open_less(char *filename) {
    char *args[3];
    char less[] = "less";

    args[0] = less;
    args[1] = filename;
    args[2] = NULL;

    exec_cmd(args);
    //remove_file(filename);
}

void open_gnuplot(){
    char *args[3];
    char gnu[]= "gnuplot";
    char file[]= "plot_graph";

    args[0] = gnu;
    args[1] = file;
    args[2] = NULL;

    exec_cmd(args);
    //remove_file(filename);

}

void open_graphic(){
    char *args[3];
    char cmd[] = "eog";
    char file[]= "data_review.png";

    args[0] = cmd;
    args[1] = file;
    args[2] = NULL;

    exec_cmd(args);
}


void gen_graphic(result results){
    char filename[] = "graph.dat";
    int i = 0, size = results.size();
    string x[size], y[size];
    ofstream outfile(filename);

    outfile <<"# " << "X" << " " << "Y" << '\n';
    for (auto row:results) {
        for (auto field:row) {
            outfile << field.as<string>() << " ";
            i++;
            if(i%2 == 0){
                outfile << '\n';
                i = 0;
            }
        }
    }
    outfile.close();
    open_gnuplot();
    open_graphic();

}

void open_relatorio(result results){
    char filename[] = "Relatorio.txt";
    open_less(filename);
    gen_graphic(results);

}

void gen_review_table(ostream& outfile, result results, pair<int, int> range) {
    TextTable table( '-', '|', '+' );
    int size = results.size();

    /* List down all the records */
    table.add("ID");
    table.add("Cliente");
    table.add("Avaliação");
    table.add("Votos");
    table.add("Útil");
    table.endOfRow();

    for (result::const_iterator c = results.begin()+range.first; c != results.end()-(size-range.second); ++c) {
        for (auto field:c) {
            table.add(field.as<string>());
        }
        table.endOfRow();
    }

    outfile << table;
}

void gen_product_table(ostream& outfile, result results) {
    TextTable table( '-', '|', '+' );
    /* List down all the records */
    table.add("ASIN");
    table.add("Título");
    table.add("Grupo");
    table.add("Salesrank");
    table.add("Downloaded");
    table.endOfRow();

    for (auto row:results) {
        for (auto field:row) {
            table.add(field.as<string>());
        }
        table.endOfRow();
    }

    outfile << table;
}

void gen_date_rating_table(ostream& outfile, result results) {
    TextTable table( '-', '|', '+' );
    /* List down all the records */
    table.add("Data");
    table.add("Média");
    table.endOfRow();

    for (auto row:results) {
        for (auto field:row) {
            table.add(field.as<string>());
        }
        table.endOfRow();
    }

    outfile << table;
}

void gen_consulta_e_table(ostream& outfile, result results) {
    TextTable table( '-', '|', '+' );
    /* List down all the records */
    table.add("asin_product");
    table.add("avg_rat");
    table.endOfRow();

    for (auto row:results) {
        for (auto field:row) {
            table.add(field.as<string>());
        }
        table.endOfRow();
    }

    outfile << table;
}

void gen_id_category_rat_table(ostream& outfile, result results) {
    TextTable table( '-', '|', '+' );
    /* List down all the records */
    table.add("ID_category");
    table.add("Name");
    table.add("Rat");
    table.endOfRow();

    for (auto row:results) {
        for (auto field:row) {
            table.add(field.as<string>());
        }
        table.endOfRow();
    }

    outfile << table;
}

void gen_consultaG_table(ostream& outfile, result results) {
    TextTable table( '-', '|', '+' );
    /* List down all the records */
    table.add("costumer");
    table.add("qtd_review");
    table.add("product_group");
    table.add("ranking");
    table.endOfRow();

    for (auto row:results) {
        for (auto field:row) {
            table.add(field.as<string>());
        }
        table.endOfRow();
    }

    outfile << table;
}

void consultaA(connection& conn, string asin) {
     /* Create a non-transactional object. */
    nontransaction transac(conn);
    /* Create SQL statement */
    string sql = "SELECT * FROM (SELECT * FROM review WHERE asin_product = '";
    sql += asin + "' ORDER BY helpful desc, rating DESC LIMIT 5) AS A ";
    sql += "UNION ALL (SELECT * FROM review WHERE asin_product = '";
    sql += asin + "' ORDER BY helpful DESC, rating LIMIT 5);";

    /* Execute SQL query */
    result results(transac.exec(sql));

    cout << "\n5 comentários mais úteis e com maior avaliação do produto " + asin + ".\n\n";
    gen_review_table(cout, results, make_pair(0, 5));

    cout << "\n5 comentários mais úteis e com menor avaliação do produto " + asin + ".\n\n";
    gen_review_table(cout, results, make_pair(5, results.size()));
}

void consultaB(connection& conn, string asin) {
    /* Create a non-transactional object. */
    nontransaction transac(conn);
    /* Create SQL statement */
    string sql = "SELECT pss.asin, pss.title, pss.product_group, pss.salesrank, pss.downloaded, p.salesrank, p.title FROM product AS p "\
    "JOIN (SELECT * FROM product_similar AS ps, product AS s WHERE ps.asin_product = '";
    sql += asin + "' AND ps.asin_similar = s.asin) AS pss ON p.asin = pss.asin_product "\
    "WHERE p.salesrank > pss.salesrank;";

    /* Execute SQL query */
    result results(transac.exec(sql));

    result::const_iterator c = results.begin();
    cout << "\nASIN: " << asin << "\nTítulo: " << c[6].as<string>() << "\nSalesrank: " << c[5].as<int>() << "\n\n";
    cout << "Produtos similares com maior número de vendas:\n\n";

    gen_product_table(cout, results);
}

void consultaC(connection& conn, string asin) {
    /* Create a non-transactional object. */
    nontransaction transac(conn);
    /* Create SQL statement */
    string sql = "SELECT public_date, ROUND(AVG(rating),2) FROM review WHERE asin_product = '";
    sql += asin + "' GROUP BY public_date ORDER BY public_date;";

    /* Execute SQL query */
    result results(transac.exec(sql));

    if(results.size() > 30) {
        char filename[] = "consultaC_results.txt";
        ofstream outfile(filename);

        outfile << "\nEvolução diária das médias de avaliação do produto " << asin << ":\n\n";
        gen_date_rating_table(outfile, results);
        outfile.close();
        open_less(filename);
        } else {
        cout << "\nEvolução diária das médias de avaliação do produto " << asin << ":\n\n";
        gen_date_rating_table(cout, results);
    }
    gen_graphic(results);
}

void consultaD(connection& conn) {
    char filename[] = "consultaD_results.txt";
    ofstream outfile(filename);
    /* Create a non-transactional object. */
    nontransaction transac(conn);
    /* Create SQL statement */
    string sql = "SELECT * FROM (SELECT *, ROW_NUMBER() OVER (PARTITION BY product_group ORDER BY salesrank) AS Row_ID"\
    " FROM product WHERE salesrank > 0) AS A WHERE Row_ID <= 10 ORDER BY product_group;";

    /* Execute SQL query */
    result results(transac.exec(sql));

    outfile << "Produtos líderes de venda em cada grupo:\n\n";
    gen_product_table(outfile, results);
    outfile.close();
    open_less(filename);
}

void consultaE(connection& conn){
    nontransaction transac(conn);
    string sql = "SELECT asin_product, avg(rating) AS avg_rat \
                  FROM review WHERE votes> 0 and helpful/votes > 0.5 and rating >= 3 \
                  GROUP BY asin_product ORDER BY avg_rat desc LIMIT 10;";

   result results(transac.exec(sql));
   cout << "\n Os 10 produtos com a maior média de avaliações úteis positivas por produto:\n\n";
   gen_consulta_e_table(cout, results);
}
void consultaF(connection& conn) {
    /* Create a non-transactional object. */
    nontransaction transac(conn);
    /* Create SQL statement */
    string sql = "SELECT pc.id_category, c.name, avg(rating) as rat \
                FROM review AS r \
                JOIN product AS p ON p.asin = r.asin_product \
                JOIN product_category AS pc ON pc.asin_product = p.asin \
                JOIN category AS c ON c.id = pc.id_category \
                WHERE r.votes > 0 AND r.helpful/r.votes > 0.5 AND r.rating >= 3 \
                GROUP BY pc.id_category, c.name \
                ORDER BY rat DESC LIMIT 5;";

    /* Execute SQL query */
    result results(transac.exec(sql));
    cout << "\n As 5 categorias de produto com a maior média de avaliações úteis positivas por produto "<< ":\n\n";
    gen_id_category_rat_table(cout, results);
}

void consultaG(connection& conn) {
    char filename[] = "consultaG_results.txt";
    ofstream outfile(filename);
    /* Create a non-transactional object. */
    nontransaction transac(conn);
    /* Create SQL statement */
    string sql = "SELECT * FROM (SELECT *, ROW_NUMBER() OVER (PARTITION BY product_group ORDER BY qtd_review DESC) AS ranking \
                  FROM (SELECT costumer, COUNT(costumer) AS qtd_review, product_group \
                  FROM (SELECT costumer, asin, product_group FROM product, review WHERE asin = asin_product) as top \
                  GROUP BY costumer, product_group) AS top2 ) AS top3 \
                  WHERE ranking <= 10;";

    /* Execute SQL query */
    result results(transac.exec(sql));
    outfile << "\n Os 10 clientes que mais fizeram comentários por grupo de produto"<< ":\n\n";
    gen_consultaG_table(outfile, results);
    outfile.close();
    open_less(filename);
}

void gerarRelatorio(connection& conn, string asin_product) {
    char filename[] = "Relatorio.txt";
    ofstream outfile(filename);
    nontransaction transac(conn);
    string sql;
    /* Consulta A*/
    sql = "SELECT * FROM (SELECT * FROM review WHERE asin_product = '";
    sql += asin_product + "' ORDER BY helpful desc, rating DESC LIMIT 5) AS A ";
    sql += "UNION ALL (SELECT * FROM review WHERE asin_product = '";
    sql += asin_product + "' ORDER BY helpful DESC, rating LIMIT 5);";

    /* Execute SQL query */
    result results(transac.exec(sql));

    outfile << "\n5 comentários mais úteis e com maior avaliação do produto " + asin_product + ".\n\n";
    gen_review_table(outfile, results, make_pair(0, 5));
    outfile << "\n\n";
    outfile << "\n5 comentários mais úteis e com menor avaliação do produto " + asin_product + ".\n\n";
    gen_review_table(outfile, results, make_pair(5, results.size()));
    outfile << "\n\n";

    /* Consulta B*/
    sql = "SELECT pss.asin, pss.title, pss.product_group, pss.salesrank, pss.downloaded, p.salesrank, p.title FROM product AS p "\
    "JOIN (SELECT * FROM product_similar AS ps, product AS s WHERE ps.asin_product = '";
    sql += asin_product + "' AND ps.asin_similar = s.asin) AS pss ON p.asin = pss.asin_product "\
    "WHERE p.salesrank > pss.salesrank;";

    /* Execute SQL query */
    result results_1(transac.exec(sql));

    result::const_iterator c = results.begin();
    outfile << "\nASIN: " << asin_product << "\nTítulo: " << c[6].as<string>() << "\nSalesrank: " << c[5].as<int>() << "\n\n";
    outfile << "Produtos similares com maior número de vendas:\n\n";
    outfile << "\n\n";
    gen_product_table(outfile, results_1);

    /* Consulta C*/
    sql = "SELECT public_date, ROUND(AVG(rating),2) FROM review WHERE asin_product = '";
    sql += asin_product + "' GROUP BY public_date ORDER BY public_date;";

    /* Execute SQL query */
    result results_2(transac.exec(sql));
    outfile << "\nEvolução diária das médias de avaliação do produto " << asin_product << ":\n\n";
    gen_date_rating_table(outfile, results_2);
    outfile << "\n\n";
    /*Consulta D*/
    sql = "SELECT * FROM (SELECT *, ROW_NUMBER() OVER (PARTITION BY product_group ORDER BY salesrank) AS Row_ID"\
    " FROM product WHERE salesrank > 0) AS A WHERE Row_ID <= 10 ORDER BY product_group;";

    /* Execute SQL query */
    result results_3(transac.exec(sql));

    outfile << "Produtos líderes de venda em cada grupo:\n\n";
    gen_product_table(outfile, results_3);
    outfile << "\n\n";

    /*Consulta E*/
    sql = "SELECT asin_product, avg(rating) AS avg_rat \
                  FROM review WHERE votes> 0 and helpful/votes > 0.5 and rating >= 3 \
                  GROUP BY asin_product ORDER BY avg_rat desc LIMIT 10;";

   result results_4(transac.exec(sql));
   outfile << "\n Os 10 produtos com a maior média de avaliações úteis positivas por produto:\n\n";
   gen_consulta_e_table(outfile, results_4);
   outfile << "\n\n";

   /*Consulta F*/
   sql = "SELECT pc.id_category, c.name, avg(rating) as rat \
               FROM review AS r \
               JOIN product AS p ON p.asin = r.asin_product \
               JOIN product_category AS pc ON pc.asin_product = p.asin \
               JOIN category AS c ON c.id = pc.id_category \
               WHERE r.votes > 0 AND r.helpful/r.votes > 0.5 AND r.rating >= 3 \
               GROUP BY pc.id_category, c.name \
               ORDER BY rat DESC LIMIT 5;";

   /* Execute SQL query */
   result results_5(transac.exec(sql));
   outfile << "\n As 5 categorias de produto com a maior média de avaliações úteis positivas por produto "<< ":\n\n";
   gen_id_category_rat_table(outfile, results_5);
   outfile << "\n\n";

   /*Condulta G*/

   sql = "SELECT * FROM (SELECT *, ROW_NUMBER() OVER (PARTITION BY product_group ORDER BY qtd_review DESC) AS ranking \
                 FROM (SELECT costumer, COUNT(costumer) AS qtd_review, product_group \
                 FROM (SELECT costumer, asin, product_group FROM product, review WHERE asin = asin_product) as top \
                 GROUP BY costumer, product_group) AS top2 ) AS top3 \
                 WHERE ranking <= 10;";

   /* Execute SQL query */
   result results_6(transac.exec(sql));
   outfile << "\n Os 10 clientes que mais fizeram comentários por grupo de produto"<< ":\n\n";
   gen_consultaG_table(outfile, results_6);



    outfile.close();
    open_relatorio(results_2);

}


int main() {
    connection conn("dbname=trab1 user=clara password=admin hostaddr=127.0.0.1 port=5432");
    if (conn.is_open()) {
        cout << "Opened database successfully: " << conn.dbname() << "\n";

    } else {
        cout << "Can't open database\n";
    }
    cout << "--------------------------------------------------- Dashboard ---------------------------------------------------\n";
    cout << "1 - Listar os 5 comentários mais úteis e com maior avaliação e os 5 comentários mais úteis e com menor avaliação, dado um produto x \n";
    cout << "2 - Listar os produtos similares com maiores vendas, dado um produto x \n";
    cout << "3 - Mostrar a evolução diária das médias de avaliação de um produto x \n";
    cout << "4 - Listar os 10 produtos lideres de venda em cada grupo de produtos \n";
    cout << "5 - Listar os 10 produtos com a maior média de avaliações úteis positivas por produto\n";
    cout << "6 - Listar a 5 categorias de produto com a maior média de avaliações úteis positivas por produto\n";
    cout << "7 - Listar os 10 clientes que mais fizeram comentários por grupo de produto\n";
    cout << "8 - Gerar Relatório\n";
    int opcao;
    string asin_prod;
    cout << "Escolha uma opção: ";
    cin >> opcao;
    if(opcao == 1){
        cout << "Digite o asin do produto: ";
        cin >> asin_prod;
        consultaA(conn, asin_prod);
    }
    if(opcao == 2){
        cout << "Digite o asin do produto: ";
        cin >> asin_prod;
        consultaB(conn, asin_prod);
    }
    if(opcao == 3){
        cout << "Digite o asin do produto: ";
        cin >> asin_prod;
        consultaC(conn, asin_prod);
    }
    if(opcao == 4){
        consultaD(conn);
    }
    if(opcao == 5){
        consultaE(conn);
    }
    if(opcao == 6){
        consultaF(conn);
    }
    if(opcao == 7){
        consultaG(conn);
    }
    if(opcao == 8){
        cout << "Digite o asin do produto: ";
        cin >> asin_prod;
        gerarRelatorio(conn, asin_prod);
    }

    conn.disconnect();
}
