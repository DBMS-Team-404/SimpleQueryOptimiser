/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_PARSER_TAB_H_INCLUDED
# define YY_YY_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 1 "src/parser.y"

    #include <iostream>
    #include <string>
    #include <vector>
    #include <nlohmann/json.hpp>
    
    // For the header file and union
    using json = nlohmann::json;

#line 59 "parser.tab.h"

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    ID = 258,                      /* ID  */
    STRING_LITERAL = 259,          /* STRING_LITERAL  */
    NUM = 260,                     /* NUM  */
    SELECT = 261,                  /* SELECT  */
    FROM = 262,                    /* FROM  */
    WHERE = 263,                   /* WHERE  */
    JOIN = 264,                    /* JOIN  */
    ON = 265,                      /* ON  */
    AND = 266,                     /* AND  */
    OR = 267,                      /* OR  */
    EQ = 268,                      /* EQ  */
    LT = 269,                      /* LT  */
    GT = 270,                      /* GT  */
    LE = 271,                      /* LE  */
    GE = 272,                      /* GE  */
    NE = 273,                      /* NE  */
    COMMA = 274,                   /* COMMA  */
    SEMICOLON = 275,               /* SEMICOLON  */
    LPAREN = 276,                  /* LPAREN  */
    RPAREN = 277,                  /* RPAREN  */
    ASTERISK = 278,                /* ASTERISK  */
    GROUP = 279,                   /* GROUP  */
    BY = 280,                      /* BY  */
    HAVING = 281,                  /* HAVING  */
    ALL = 282,                     /* ALL  */
    DISTINCT = 283,                /* DISTINCT  */
    ORDER = 284,                   /* ORDER  */
    ASC = 285,                     /* ASC  */
    DESC = 286,                    /* DESC  */
    USING = 287,                   /* USING  */
    NULLS = 288,                   /* NULLS  */
    FIRST = 289,                   /* FIRST  */
    LAST = 290,                    /* LAST  */
    UNION = 291,                   /* UNION  */
    INTERSECT = 292,               /* INTERSECT  */
    EXCEPT = 293,                  /* EXCEPT  */
    INSERT = 294,                  /* INSERT  */
    INTO = 295,                    /* INTO  */
    VALUES = 296,                  /* VALUES  */
    DEFAULT = 297,                 /* DEFAULT  */
    WITH = 298,                    /* WITH  */
    RECURSIVE = 299,               /* RECURSIVE  */
    AS = 300,                      /* AS  */
    UPDATE = 301,                  /* UPDATE  */
    ONLY = 302,                    /* ONLY  */
    SET = 303,                     /* SET  */
    ROW = 304,                     /* ROW  */
    IS = 305,                      /* IS  */
    NOT = 306,                     /* NOT  */
    NULL_VAL = 307,                /* NULL_VAL  */
    DELETE = 308,                  /* DELETE  */
    EXISTS = 309,                  /* EXISTS  */
    BETWEEN = 310,                 /* BETWEEN  */
    IN = 311,                      /* IN  */
    INNER = 312,                   /* INNER  */
    LEFT = 313,                    /* LEFT  */
    RIGHT = 314,                   /* RIGHT  */
    FULL = 315,                    /* FULL  */
    CROSS = 316,                   /* CROSS  */
    NATURAL = 317,                 /* NATURAL  */
    CONCAT = 318,                  /* CONCAT  */
    PLUS = 319,                    /* PLUS  */
    MINUS = 320,                   /* MINUS  */
    DIV = 321,                     /* DIV  */
    DOT = 322                      /* DOT  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 27 "src/parser.y"

    int num;
    char* str;
    std::vector<std::string>* str_list;
    json* json_node; 

#line 150 "parser.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_PARSER_TAB_H_INCLUDED  */
