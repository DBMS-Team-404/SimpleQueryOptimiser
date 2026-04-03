%code requires {
    #include <iostream>
    #include <string>
    #include <vector>
    #include <nlohmann/json.hpp>
    
    // For the header file and union
    using json = nlohmann::json;
}

%{
    // --- THE MAGIC FIX: Include it here too for the .c file ---
    #include <nlohmann/json.hpp>
    // ----------------------------------------------------------

    using json = nlohmann::json;

    extern int yylex();
    extern int yylineno;
    void yyerror(const char *s);

    /* Explicitly use the full namespace */
    nlohmann::json final_ast;
%}

/* types for our semantic values */
%union {
    int num;
    char* str;
    std::vector<std::string>* str_list;
    json* json_node; 
}

/* Base Tokens used in your rules */
%token <str> ID STRING_LITERAL
%token <num> NUM
%token SELECT FROM WHERE JOIN ON AND OR EQ LT GT LE GE NE
%token COMMA SEMICOLON LPAREN RPAREN ASTERISK GROUP BY HAVING

/* Declare ALL the extra tokens your lexer is trying to return */
%token ALL DISTINCT ORDER ASC DESC USING NULLS FIRST LAST UNION INTERSECT
%token EXCEPT INSERT INTO VALUES DEFAULT WITH RECURSIVE AS UPDATE ONLY SET
%token ROW IS NOT NULL_VAL DELETE EXISTS BETWEEN IN INNER LEFT RIGHT FULL
%token CROSS NATURAL CONCAT PLUS MINUS DIV DOT

/* Define types for non-terminals */
%type <str_list> select_list group_by_list
%type <json_node> table_reference condition query expression where_clause group_by_clause

%start sql_statement

%%

sql_statement:
    query SEMICOLON {
        final_ast = *$1;
        delete $1; 
    }
    ;

query:
    SELECT select_list FROM table_reference where_clause group_by_clause {
        json* current_node = $4; 

        if ($5 != nullptr) {
            current_node = new json({
                {"node_type", "LOGICAL_FILTER"},
                {"properties", { {"predicate", *$5} }},
                {"children", { *current_node }}
            });
            delete $5;
        }

        if ($6 != nullptr) {
            current_node = new json({
                {"node_type", "LOGICAL_AGGREGATE"},
                {"properties", { 
                    {"group_columns", (*$6)["groups"]},
                    {"having", (*$6)["having"]} 
                }},
                {"children", { *current_node }}
            });
            delete $6;
        }

        $$ = new json({
            {"node_type", "LOGICAL_PROJECT"},
            {"properties", { {"columns", *$2} }},
            {"children", { *current_node }}
        });

        delete $2;
    }
    ;

where_clause:
    WHERE condition { $$ = $2; }
    | /* empty */   { $$ = nullptr; }
    ;

group_by_clause:
    GROUP BY group_by_list {
        $$ = new json({{"groups", *$3}, {"having", nullptr}});
        delete $3;
    }
    | GROUP BY group_by_list HAVING condition {
        $$ = new json({{"groups", *$3}, {"having", *$5}});
        delete $3; delete $5;
    }
    | /* empty */ { $$ = nullptr; }
    ;

group_by_list:
    ID {
        $$ = new std::vector<std::string>{std::string($1)};
        free($1);
    }
    | group_by_list COMMA ID {
        $1->push_back(std::string($3));
        $$ = $1;
        free($3);
    }
    ;

select_list:
    ID {
        $$ = new std::vector<std::string>(); 
        $$->push_back(std::string($1));
        free($1); 
    }
    | select_list COMMA ID {
        $1->push_back(std::string($3)); 
        $$ = $1;
        free($3); 
    }
    | ASTERISK {
        $$ = new std::vector<std::string>{"*"}; 
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
        free($1); 
    }
    | table_reference JOIN ID ON condition {
        json* right_table = new json({
            {"node_type", "LOGICAL_GET"},
            {"properties", {
                {"table", std::string($3)},
                {"alias", std::string($3) + "_alias"}
            }}
        });

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
        $$ = new json({{"expression_type", "COLUMN"}, {"value", std::string($1)}});
        free($1);
    }
    | ID DOT ID { 
        std::string full_column = std::string($1) + "." + std::string($3);
        $$ = new json({{"expression_type", "COLUMN"}, {"value", full_column}});
        free($1); free($3);
    }
    | NUM {
        $$ = new json({{"expression_type", "CONSTANT"}, {"value", std::to_string($1)}});
    }
    | STRING_LITERAL{
        $$ = new json({{"expression_type", "CONSTANT"}, {"value", std::string($1)}});
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

nlohmann::json parse_sql_to_json() {
    yyparse();
    return final_ast;
}