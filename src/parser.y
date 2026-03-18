%{
#include <iostream>
#include <string>
#include <vector>
#include "../include/nlohmann/json.hpp"

using json = nlohmann::json;

extern int yylex();
extern int yylineno;
void yyerror(const char *s);

/* output json object*/
json final_ast;
%}

/* types for our semantic values */
%union {
    int num;
    char* str;
    std::vector<std::string>* str_list;
    nlohmann::json* json_node;
}

%token <str> ID STRING_LITERAL
%token <num> NUM
%token SELECT FROM WHERE JOIN ON AND OR EQ LT GT LE GE NE
%token COMMA SEMICOLON LPAREN RPAREN
%token ASTERISK

/* Define types for non-terminals */
%type <str_list> select_list
%type <json_node> table_reference condition query expression

%start sql_statement

%%

sql_statement:
    query SEMICOLON {
        final_ast = *$1;
        delete $1; // Clean up
    }
    ;

query:
    SELECT select_list FROM table_reference WHERE condition {
        // Wraping the GET node in a FILTER node
        json* filter_node = new json({
            {"node_type", "LOGICAL_FILTER"},
            {"properties", { {"predicate", *$6} }},
            {"children", { *$4 }} // table_reference 
        });

        // Wraping the FILTER node in a PROJECT node
        $$ = new json({
            {"node_type", "LOGICAL_PROJECT"},
            {"properties", { {"columns", *$2} }},
            {"children", { *filter_node }}
        });

        delete $2; delete $4; delete $6; delete filter_node;
    }

    | SELECT select_list FROM table_reference {
        // no where clause just project
        $$ = new json({
            {"node_type", "LOGICAL_PROJECT"},
            {"properties", { {"columns", *$2} }},
            {"children", { *$4 }}
        });
        
        delete $2; delete $4;
    }
    ;

select_list:
    ID {
        $$ = new std::vector<std::string>(); 
        $$->push_back(std::string($1));
        free($1); // Freeing the strdup from flex 
    }
    | select_list COMMA ID {
        $1->push_back(std::string($3)); // pushing id into the list formed till now
        $$ = $1;
        free($3); // freeing the strdup form flex
    }
    | ASTERISK {
        $$ = new std::vector<std::string>{"*"}; // pick everything
    }
    ;

table_reference:
    ID {
        $$ = new json({
            {"node_type", "LOGICAL_GET"},
            {"properties", {
                {"table", std::string($1)},
                {"alias", std::string($1) + "_alias"}
            }}
        });
        free($1); // freeing the strdup from flex
    }
    | table_reference JOIN ID ON condition {
        // creating right-side node
            json* right_table = new json({
            {"node_type", "LOGICAL_GET"},
            {"properties", {
                {"table", std::string($3)},
                {"alias", std::string($3) + "_alias"}
            }}
        });

        // combine left and right table into a join node
        $$ = new json({
            {"node_type", "LOGICAL_JOIN"},
            {"properties", {
                {"join_type", "INNER"},
                {"condition", *$5}
            }},
            {"children", {*$1, *right_table}}
        });
        delete $1; delete right_table; delete $5; free($3);
    }
    ;

expression:
    ID {
        $$ = new json({{"expression_type", "COLUMN"}, {"value", std :: string($1)}});
        free($1);
    }
    | NUM {
        $$ = new josn({{"expression_type", "CONSTANT"}, {"value", std :: to_string($1)}});
    }
    | STRING_LITERAL{
        $$ = new json({{"expression_type", "CONSTANT"}, {"value", std :: string($1)}});
        free($1);
    }
    ;

condition:
    expression EQ expression {
        $$ = new json({ {"expression_type", "COMPARISON"}, {"operator", "="}, {"left", *$1}, {"right", *$3} });
        delete $1; delete $3;
    }
    | expression NE expression {
        $$ = new json({ {"expression_type", "COMPARISON"}, {"operator", "<>"}, {"left", *$1}, {"right", *$3} });
        delete $1; delete $3;
    }
    | expression LT expression {
        $$ = new json({ {"expression_type", "COMPARISON"}, {"operator", "<"}, {"left", *$1}, {"right", *$3} });
        delete $1; delete $3;
    }
    | expression GT expression {
        $$ = new json({ {"expression_type", "COMPARISON"}, {"operator", ">"}, {"left", *$1}, {"right", *$3} });
        delete $1; delete $3;
    }
    | expression LE expression {
        $$ = new json({ {"expression_type", "COMPARISON"}, {"operator", "<="}, {"left", *$1}, {"right", *$3} });
        delete $1; delete $3;
    }
    | expression GE expression {
        $$ = new json({ {"expression_type", "COMPARISON"}, {"operator", ">="}, {"left", *$1}, {"right", *$3} });
        delete $1; delete $3;
    }
    | condition AND condition {
        $$ = new json({ {"expression_type", "LOGICAL_AND"}, {"left", *$1}, {"right", *$3} });
        delete $1; delete $3;
    }
    | condition OR condition {
        $$ = new json({ {"expression_type", "LOGICAL_OR"}, {"left", *$1}, {"right", *$3} });
        delete $1; delete $3;
    }
    | LPAREN condition RPAREN {
        $$ = $2; 
    }
    ;


%%

void yyerror(const char *s) 
{
    std::cerr << "Parse Error at line " << yylineno << ": " << s << "\n";
}

// helper function 
json parse_sql_to_json() {
    yyparse();
    return final_ast;
}

