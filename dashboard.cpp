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

int main() {
    connection conn("dbname=trab1 user=anavi password=admin hostaddr=127.0.0.1 port=5432");
    if (conn.is_open()) {
        cout << "Opened database successfully: " << conn.dbname() << "\n";

    } else {
        cout << "Can't open database\n";
    }

    //consultaA(conn, "6304286961");
    //consultaB(conn, "6304286961");
    consultaC(conn, "6304286961");
    //consultaD(conn);

    conn.disconnect();
}
