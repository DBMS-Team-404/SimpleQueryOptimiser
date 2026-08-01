/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 11 "src/parser.y"

    // --- THE MAGIC FIX: Include it here too for the .c file ---
    #include <nlohmann/json.hpp>
    // ----------------------------------------------------------

    using json = nlohmann::json;

    extern int yylex();
    extern int yylineno;
    void yyerror(const char *s);

    /* Explicitly use the full namespace */
    nlohmann::json final_ast;

#line 86 "parser.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "parser.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_ID = 3,                         /* ID  */
  YYSYMBOL_STRING_LITERAL = 4,             /* STRING_LITERAL  */
  YYSYMBOL_NUM = 5,                        /* NUM  */
  YYSYMBOL_SELECT = 6,                     /* SELECT  */
  YYSYMBOL_FROM = 7,                       /* FROM  */
  YYSYMBOL_WHERE = 8,                      /* WHERE  */
  YYSYMBOL_JOIN = 9,                       /* JOIN  */
  YYSYMBOL_ON = 10,                        /* ON  */
  YYSYMBOL_AND = 11,                       /* AND  */
  YYSYMBOL_OR = 12,                        /* OR  */
  YYSYMBOL_EQ = 13,                        /* EQ  */
  YYSYMBOL_LT = 14,                        /* LT  */
  YYSYMBOL_GT = 15,                        /* GT  */
  YYSYMBOL_LE = 16,                        /* LE  */
  YYSYMBOL_GE = 17,                        /* GE  */
  YYSYMBOL_NE = 18,                        /* NE  */
  YYSYMBOL_COMMA = 19,                     /* COMMA  */
  YYSYMBOL_SEMICOLON = 20,                 /* SEMICOLON  */
  YYSYMBOL_LPAREN = 21,                    /* LPAREN  */
  YYSYMBOL_RPAREN = 22,                    /* RPAREN  */
  YYSYMBOL_ASTERISK = 23,                  /* ASTERISK  */
  YYSYMBOL_GROUP = 24,                     /* GROUP  */
  YYSYMBOL_BY = 25,                        /* BY  */
  YYSYMBOL_HAVING = 26,                    /* HAVING  */
  YYSYMBOL_ALL = 27,                       /* ALL  */
  YYSYMBOL_DISTINCT = 28,                  /* DISTINCT  */
  YYSYMBOL_ORDER = 29,                     /* ORDER  */
  YYSYMBOL_ASC = 30,                       /* ASC  */
  YYSYMBOL_DESC = 31,                      /* DESC  */
  YYSYMBOL_USING = 32,                     /* USING  */
  YYSYMBOL_NULLS = 33,                     /* NULLS  */
  YYSYMBOL_FIRST = 34,                     /* FIRST  */
  YYSYMBOL_LAST = 35,                      /* LAST  */
  YYSYMBOL_UNION = 36,                     /* UNION  */
  YYSYMBOL_INTERSECT = 37,                 /* INTERSECT  */
  YYSYMBOL_EXCEPT = 38,                    /* EXCEPT  */
  YYSYMBOL_INSERT = 39,                    /* INSERT  */
  YYSYMBOL_INTO = 40,                      /* INTO  */
  YYSYMBOL_VALUES = 41,                    /* VALUES  */
  YYSYMBOL_DEFAULT = 42,                   /* DEFAULT  */
  YYSYMBOL_WITH = 43,                      /* WITH  */
  YYSYMBOL_RECURSIVE = 44,                 /* RECURSIVE  */
  YYSYMBOL_AS = 45,                        /* AS  */
  YYSYMBOL_UPDATE = 46,                    /* UPDATE  */
  YYSYMBOL_ONLY = 47,                      /* ONLY  */
  YYSYMBOL_SET = 48,                       /* SET  */
  YYSYMBOL_ROW = 49,                       /* ROW  */
  YYSYMBOL_IS = 50,                        /* IS  */
  YYSYMBOL_NOT = 51,                       /* NOT  */
  YYSYMBOL_NULL_VAL = 52,                  /* NULL_VAL  */
  YYSYMBOL_DELETE = 53,                    /* DELETE  */
  YYSYMBOL_EXISTS = 54,                    /* EXISTS  */
  YYSYMBOL_BETWEEN = 55,                   /* BETWEEN  */
  YYSYMBOL_IN = 56,                        /* IN  */
  YYSYMBOL_INNER = 57,                     /* INNER  */
  YYSYMBOL_LEFT = 58,                      /* LEFT  */
  YYSYMBOL_RIGHT = 59,                     /* RIGHT  */
  YYSYMBOL_FULL = 60,                      /* FULL  */
  YYSYMBOL_CROSS = 61,                     /* CROSS  */
  YYSYMBOL_NATURAL = 62,                   /* NATURAL  */
  YYSYMBOL_CONCAT = 63,                    /* CONCAT  */
  YYSYMBOL_PLUS = 64,                      /* PLUS  */
  YYSYMBOL_MINUS = 65,                     /* MINUS  */
  YYSYMBOL_DIV = 66,                       /* DIV  */
  YYSYMBOL_DOT = 67,                       /* DOT  */
  YYSYMBOL_YYACCEPT = 68,                  /* $accept  */
  YYSYMBOL_sql_statement = 69,             /* sql_statement  */
  YYSYMBOL_query = 70,                     /* query  */
  YYSYMBOL_where_clause = 71,              /* where_clause  */
  YYSYMBOL_group_by_clause = 72,           /* group_by_clause  */
  YYSYMBOL_group_by_list = 73,             /* group_by_list  */
  YYSYMBOL_select_list = 74,               /* select_list  */
  YYSYMBOL_select_item = 75,               /* select_item  */
  YYSYMBOL_join_type = 76,                 /* join_type  */
  YYSYMBOL_table_reference = 77,           /* table_reference  */
  YYSYMBOL_expression = 78,                /* expression  */
  YYSYMBOL_condition = 79                  /* condition  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  8
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   75

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  68
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  12
/* YYNRULES -- Number of rules.  */
#define YYNRULES  42
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  81

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   322


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,    62,    62,    69,   104,   105,   109,   113,   117,   121,
     125,   129,   134,   142,   147,   155,   159,   163,   167,   173,
     174,   175,   176,   177,   178,   182,   192,   202,   213,   232,
     255,   259,   264,   267,   274,   278,   282,   286,   290,   294,
     298,   302,   306
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "ID", "STRING_LITERAL",
  "NUM", "SELECT", "FROM", "WHERE", "JOIN", "ON", "AND", "OR", "EQ", "LT",
  "GT", "LE", "GE", "NE", "COMMA", "SEMICOLON", "LPAREN", "RPAREN",
  "ASTERISK", "GROUP", "BY", "HAVING", "ALL", "DISTINCT", "ORDER", "ASC",
  "DESC", "USING", "NULLS", "FIRST", "LAST", "UNION", "INTERSECT",
  "EXCEPT", "INSERT", "INTO", "VALUES", "DEFAULT", "WITH", "RECURSIVE",
  "AS", "UPDATE", "ONLY", "SET", "ROW", "IS", "NOT", "NULL_VAL", "DELETE",
  "EXISTS", "BETWEEN", "IN", "INNER", "LEFT", "RIGHT", "FULL", "CROSS",
  "NATURAL", "CONCAT", "PLUS", "MINUS", "DIV", "DOT", "$accept",
  "sql_statement", "query", "where_clause", "group_by_clause",
  "group_by_list", "select_list", "select_item", "join_type",
  "table_reference", "expression", "condition", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-45)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int8 yypact[] =
{
      30,     3,    15,    19,   -44,   -45,    -3,   -45,   -45,   -45,
       5,    34,     3,   -45,   -45,    -1,    -8,   -45,   -45,    37,
       8,   -45,    32,    33,    38,    39,    45,    40,    58,   -45,
     -24,   -45,   -45,     8,    17,    10,   -45,   -45,   -45,   -45,
     -45,    41,   -45,    -7,    59,    -2,    14,    14,    14,    14,
      14,    14,     8,     8,    60,     8,    62,   -45,   -45,   -45,
     -45,   -45,   -45,   -45,   -45,   -45,    56,     1,   -12,    10,
      61,    66,    67,     8,     8,   -45,     6,    10,    10,    69,
     -45
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,     0,     0,    15,    18,     0,    13,     1,     2,
       0,     0,     0,    16,    17,    25,     5,    14,    27,     0,
       0,    19,     0,     0,     0,     0,     0,     8,     0,    26,
      30,    33,    32,     0,     0,     4,    20,    21,    22,    24,
      23,     0,     3,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    31,    42,    34,
      36,    37,    38,    39,    35,    40,    41,     9,     6,    28,
       0,     0,     0,     0,     0,    10,    11,     7,    29,     0,
      12
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -45,   -45,   -45,   -45,   -45,   -45,   -45,    63,   -45,   -45,
       9,   -28
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     2,     3,    27,    42,    68,     6,     7,    28,    16,
      34,    35
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      20,    21,    18,    55,    11,    45,     4,    72,    13,    52,
      53,    30,    31,    32,    73,     8,    12,    30,    31,    32,
      58,    52,    53,    10,    65,    66,     5,    69,    14,    33,
      46,    47,    48,    49,    50,    51,     1,    15,    56,     9,
      29,    36,    37,    44,    19,    77,    78,    38,    39,    22,
      23,    24,    25,    26,    40,    59,    60,    61,    62,    63,
      64,    43,    57,    67,    41,    70,    54,    52,    71,    75,
      76,    74,    80,    79,     0,    17
};

static const yytype_int8 yycheck[] =
{
       8,     9,     3,    10,     7,    33,     3,    19,     3,    11,
      12,     3,     4,     5,    26,     0,    19,     3,     4,     5,
      22,    11,    12,    67,    52,    53,    23,    55,    23,    21,
      13,    14,    15,    16,    17,    18,     6,     3,    45,    20,
       3,     9,     9,    67,    45,    73,    74,     9,     9,    57,
      58,    59,    60,    61,     9,    46,    47,    48,    49,    50,
      51,     3,     3,     3,    24,     3,    25,    11,    67,     3,
       3,    10,     3,    67,    -1,    12
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     6,    69,    70,     3,    23,    74,    75,     0,    20,
      67,     7,    19,     3,    23,     3,    77,    75,     3,    45,
       8,     9,    57,    58,    59,    60,    61,    71,    76,     3,
       3,     4,     5,    21,    78,    79,     9,     9,     9,     9,
       9,    24,    72,     3,    67,    79,    13,    14,    15,    16,
      17,    18,    11,    12,    25,    10,    45,     3,    22,    78,
      78,    78,    78,    78,    78,    79,    79,     3,    73,    79,
       3,    67,    19,    26,    10,     3,     3,    79,    79,    67,
       3
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    68,    69,    70,    71,    71,    72,    72,    72,    73,
      73,    73,    73,    74,    74,    75,    75,    75,    75,    76,
      76,    76,    76,    76,    76,    77,    77,    77,    77,    77,
      78,    78,    78,    78,    79,    79,    79,    79,    79,    79,
      79,    79,    79
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     6,     2,     0,     3,     5,     0,     1,
       3,     3,     5,     1,     3,     1,     3,     3,     1,     1,
       2,     2,     2,     2,     2,     1,     3,     2,     5,     7,
       1,     3,     1,     1,     3,     3,     3,     3,     3,     3,
       3,     3,     3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* sql_statement: query SEMICOLON  */
#line 62 "src/parser.y"
                    {
        final_ast = *(yyvsp[-1].json_node);
        delete (yyvsp[-1].json_node); 
    }
#line 1210 "parser.tab.c"
    break;

  case 3: /* query: SELECT select_list FROM table_reference where_clause group_by_clause  */
#line 69 "src/parser.y"
                                                                         {
        json* current_node = (yyvsp[-2].json_node); 

        if ((yyvsp[-1].json_node) != nullptr) {
            current_node = new json({
                {"node_type", "LOGICAL_FILTER"},
                {"properties", { {"predicate", *(yyvsp[-1].json_node)} }},
                {"children", { *current_node }}
            });
            delete (yyvsp[-1].json_node);
        }

        if ((yyvsp[0].json_node) != nullptr) {
            current_node = new json({
                {"node_type", "LOGICAL_AGGREGATE"},
                {"properties", { 
                    {"group_columns", (*(yyvsp[0].json_node))["groups"]},
                    {"having", (*(yyvsp[0].json_node))["having"]} 
                }},
                {"children", { *current_node }}
            });
            delete (yyvsp[0].json_node);
        }

        (yyval.json_node) = new json({
            {"node_type", "LOGICAL_PROJECT"},
            {"properties", { {"columns", *(yyvsp[-4].str_list)} }},
            {"children", { *current_node }}
        });

        delete (yyvsp[-4].str_list);
    }
#line 1247 "parser.tab.c"
    break;

  case 4: /* where_clause: WHERE condition  */
#line 104 "src/parser.y"
                    { (yyval.json_node) = (yyvsp[0].json_node); }
#line 1253 "parser.tab.c"
    break;

  case 5: /* where_clause: %empty  */
#line 105 "src/parser.y"
                    { (yyval.json_node) = nullptr; }
#line 1259 "parser.tab.c"
    break;

  case 6: /* group_by_clause: GROUP BY group_by_list  */
#line 109 "src/parser.y"
                           {
        (yyval.json_node) = new json({{"groups", *(yyvsp[0].str_list)}, {"having", nullptr}});
        delete (yyvsp[0].str_list);
    }
#line 1268 "parser.tab.c"
    break;

  case 7: /* group_by_clause: GROUP BY group_by_list HAVING condition  */
#line 113 "src/parser.y"
                                              {
        (yyval.json_node) = new json({{"groups", *(yyvsp[-2].str_list)}, {"having", *(yyvsp[0].json_node)}});
        delete (yyvsp[-2].str_list); delete (yyvsp[0].json_node);
    }
#line 1277 "parser.tab.c"
    break;

  case 8: /* group_by_clause: %empty  */
#line 117 "src/parser.y"
                  { (yyval.json_node) = nullptr; }
#line 1283 "parser.tab.c"
    break;

  case 9: /* group_by_list: ID  */
#line 121 "src/parser.y"
       {
        (yyval.str_list) = new std::vector<std::string>{std::string((yyvsp[0].str))};
        free((yyvsp[0].str));
    }
#line 1292 "parser.tab.c"
    break;

  case 10: /* group_by_list: ID DOT ID  */
#line 125 "src/parser.y"
                {
        (yyval.str_list) = new std::vector<std::string>{std::string((yyvsp[-2].str)) + "." + std::string((yyvsp[0].str))};
        free((yyvsp[-2].str)); free((yyvsp[0].str));
    }
#line 1301 "parser.tab.c"
    break;

  case 11: /* group_by_list: group_by_list COMMA ID  */
#line 129 "src/parser.y"
                             {
        (yyvsp[-2].str_list)->push_back(std::string((yyvsp[0].str)));
        (yyval.str_list) = (yyvsp[-2].str_list);
        free((yyvsp[0].str));
    }
#line 1311 "parser.tab.c"
    break;

  case 12: /* group_by_list: group_by_list COMMA ID DOT ID  */
#line 134 "src/parser.y"
                                    {
        (yyvsp[-4].str_list)->push_back(std::string((yyvsp[-2].str)) + "." + std::string((yyvsp[0].str)));
        (yyval.str_list) = (yyvsp[-4].str_list);
        free((yyvsp[-2].str)); free((yyvsp[0].str));
    }
#line 1321 "parser.tab.c"
    break;

  case 13: /* select_list: select_item  */
#line 142 "src/parser.y"
                {
        (yyval.str_list) = new std::vector<std::string>();
        (yyval.str_list)->push_back(*(yyvsp[0].str));
        delete (yyvsp[0].str);
    }
#line 1331 "parser.tab.c"
    break;

  case 14: /* select_list: select_list COMMA select_item  */
#line 147 "src/parser.y"
                                    {
        (yyvsp[-2].str_list)->push_back(*(yyvsp[0].str));
        (yyval.str_list) = (yyvsp[-2].str_list);
        delete (yyvsp[0].str);
    }
#line 1341 "parser.tab.c"
    break;

  case 15: /* select_item: ID  */
#line 155 "src/parser.y"
       {
        (yyval.str) = new std::string((yyvsp[0].str));
        free((yyvsp[0].str));
    }
#line 1350 "parser.tab.c"
    break;

  case 16: /* select_item: ID DOT ID  */
#line 159 "src/parser.y"
                {
        (yyval.str) = new std::string(std::string((yyvsp[-2].str)) + "." + std::string((yyvsp[0].str)));
        free((yyvsp[-2].str)); free((yyvsp[0].str));
    }
#line 1359 "parser.tab.c"
    break;

  case 17: /* select_item: ID DOT ASTERISK  */
#line 163 "src/parser.y"
                      {
        (yyval.str) = new std::string(std::string((yyvsp[-2].str)) + ".*");
        free((yyvsp[-2].str));
    }
#line 1368 "parser.tab.c"
    break;

  case 18: /* select_item: ASTERISK  */
#line 167 "src/parser.y"
               {
        (yyval.str) = new std::string("*");
    }
#line 1376 "parser.tab.c"
    break;

  case 19: /* join_type: JOIN  */
#line 173 "src/parser.y"
                 { (yyval.str) = strdup("INNER"); }
#line 1382 "parser.tab.c"
    break;

  case 20: /* join_type: INNER JOIN  */
#line 174 "src/parser.y"
                 { (yyval.str) = strdup("INNER"); }
#line 1388 "parser.tab.c"
    break;

  case 21: /* join_type: LEFT JOIN  */
#line 175 "src/parser.y"
                 { (yyval.str) = strdup("LEFT");  }
#line 1394 "parser.tab.c"
    break;

  case 22: /* join_type: RIGHT JOIN  */
#line 176 "src/parser.y"
                 { (yyval.str) = strdup("RIGHT"); }
#line 1400 "parser.tab.c"
    break;

  case 23: /* join_type: CROSS JOIN  */
#line 177 "src/parser.y"
                 { (yyval.str) = strdup("CROSS"); }
#line 1406 "parser.tab.c"
    break;

  case 24: /* join_type: FULL JOIN  */
#line 178 "src/parser.y"
                 { (yyval.str) = strdup("FULL");  }
#line 1412 "parser.tab.c"
    break;

  case 25: /* table_reference: ID  */
#line 182 "src/parser.y"
       {
        (yyval.json_node) = new json({
            {"node_type", "LOGICAL_GET"},
            {"properties", {
                {"table", std::string((yyvsp[0].str))},
                {"alias", std::string((yyvsp[0].str))}
            }}
        });
        free((yyvsp[0].str));
    }
#line 1427 "parser.tab.c"
    break;

  case 26: /* table_reference: ID AS ID  */
#line 192 "src/parser.y"
               {
        (yyval.json_node) = new json({
            {"node_type", "LOGICAL_GET"},
            {"properties", {
                {"table", std::string((yyvsp[-2].str))},
                {"alias", std::string((yyvsp[0].str))}
            }}
        });
        free((yyvsp[-2].str)); free((yyvsp[0].str));
    }
#line 1442 "parser.tab.c"
    break;

  case 27: /* table_reference: ID ID  */
#line 202 "src/parser.y"
            {
        /* bare alias: FROM users u */
        (yyval.json_node) = new json({
            {"node_type", "LOGICAL_GET"},
            {"properties", {
                {"table", std::string((yyvsp[-1].str))},
                {"alias", std::string((yyvsp[0].str))}
            }}
        });
        free((yyvsp[-1].str)); free((yyvsp[0].str));
    }
#line 1458 "parser.tab.c"
    break;

  case 28: /* table_reference: table_reference join_type ID ON condition  */
#line 213 "src/parser.y"
                                                {
        json* right_table = new json({
            {"node_type", "LOGICAL_GET"},
            {"properties", {
                {"table", std::string((yyvsp[-2].str))},
                {"alias", std::string((yyvsp[-2].str))}
            }}
        });
        (yyval.json_node) = new json({
            {"node_type", "LOGICAL_JOIN"},
            {"properties", {
                {"join_type", std::string((yyvsp[-3].str))},
                {"condition", *(yyvsp[0].json_node)}
            }},
            {"children", {*(yyvsp[-4].json_node), *right_table}}
        });
        delete (yyvsp[-4].json_node); delete right_table; delete (yyvsp[0].json_node);
        free((yyvsp[-3].str)); free((yyvsp[-2].str));
    }
#line 1482 "parser.tab.c"
    break;

  case 29: /* table_reference: table_reference join_type ID AS ID ON condition  */
#line 232 "src/parser.y"
                                                      {
        /* JOIN with alias: JOIN orders AS o ON ... */
        json* right_table = new json({
            {"node_type", "LOGICAL_GET"},
            {"properties", {
                {"table", std::string((yyvsp[-4].str))},
                {"alias", std::string((yyvsp[-2].str))}
            }}
        });
        (yyval.json_node) = new json({
            {"node_type", "LOGICAL_JOIN"},
            {"properties", {
                {"join_type", std::string((yyvsp[-5].str))},
                {"condition", *(yyvsp[0].json_node)}
            }},
            {"children", {*(yyvsp[-6].json_node), *right_table}}
        });
        delete (yyvsp[-6].json_node); delete right_table; delete (yyvsp[0].json_node);
        free((yyvsp[-5].str)); free((yyvsp[-4].str)); free((yyvsp[-2].str));
    }
#line 1507 "parser.tab.c"
    break;

  case 30: /* expression: ID  */
#line 255 "src/parser.y"
       {
        (yyval.json_node) = new json({{"expression_type", "COLUMN"}, {"value", std::string((yyvsp[0].str))}});
        free((yyvsp[0].str));
    }
#line 1516 "parser.tab.c"
    break;

  case 31: /* expression: ID DOT ID  */
#line 259 "src/parser.y"
                { 
        std::string full_column = std::string((yyvsp[-2].str)) + "." + std::string((yyvsp[0].str));
        (yyval.json_node) = new json({{"expression_type", "COLUMN"}, {"value", full_column}});
        free((yyvsp[-2].str)); free((yyvsp[0].str));
    }
#line 1526 "parser.tab.c"
    break;

  case 32: /* expression: NUM  */
#line 264 "src/parser.y"
          {
        (yyval.json_node) = new json({{"expression_type", "CONSTANT"}, {"value", std::to_string((yyvsp[0].num))}});
    }
#line 1534 "parser.tab.c"
    break;

  case 33: /* expression: STRING_LITERAL  */
#line 267 "src/parser.y"
                    {
        (yyval.json_node) = new json({{"expression_type", "CONSTANT"}, {"value", std::string((yyvsp[0].str))}});
        free((yyvsp[0].str));
    }
#line 1543 "parser.tab.c"
    break;

  case 34: /* condition: expression EQ expression  */
#line 274 "src/parser.y"
                             {
        (yyval.json_node) = new json({ {"expression_type", "COMPARISON"}, {"operator", "="}, {"left", *(yyvsp[-2].json_node)}, {"right", *(yyvsp[0].json_node)} });
        delete (yyvsp[-2].json_node); delete (yyvsp[0].json_node);
    }
#line 1552 "parser.tab.c"
    break;

  case 35: /* condition: expression NE expression  */
#line 278 "src/parser.y"
                               {
        (yyval.json_node) = new json({ {"expression_type", "COMPARISON"}, {"operator", "<>"}, {"left", *(yyvsp[-2].json_node)}, {"right", *(yyvsp[0].json_node)} });
        delete (yyvsp[-2].json_node); delete (yyvsp[0].json_node);
    }
#line 1561 "parser.tab.c"
    break;

  case 36: /* condition: expression LT expression  */
#line 282 "src/parser.y"
                               {
        (yyval.json_node) = new json({ {"expression_type", "COMPARISON"}, {"operator", "<"}, {"left", *(yyvsp[-2].json_node)}, {"right", *(yyvsp[0].json_node)} });
        delete (yyvsp[-2].json_node); delete (yyvsp[0].json_node);
    }
#line 1570 "parser.tab.c"
    break;

  case 37: /* condition: expression GT expression  */
#line 286 "src/parser.y"
                               {
        (yyval.json_node) = new json({ {"expression_type", "COMPARISON"}, {"operator", ">"}, {"left", *(yyvsp[-2].json_node)}, {"right", *(yyvsp[0].json_node)} });
        delete (yyvsp[-2].json_node); delete (yyvsp[0].json_node);
    }
#line 1579 "parser.tab.c"
    break;

  case 38: /* condition: expression LE expression  */
#line 290 "src/parser.y"
                               {
        (yyval.json_node) = new json({ {"expression_type", "COMPARISON"}, {"operator", "<="}, {"left", *(yyvsp[-2].json_node)}, {"right", *(yyvsp[0].json_node)} });
        delete (yyvsp[-2].json_node); delete (yyvsp[0].json_node);
    }
#line 1588 "parser.tab.c"
    break;

  case 39: /* condition: expression GE expression  */
#line 294 "src/parser.y"
                               {
        (yyval.json_node) = new json({ {"expression_type", "COMPARISON"}, {"operator", ">="}, {"left", *(yyvsp[-2].json_node)}, {"right", *(yyvsp[0].json_node)} });
        delete (yyvsp[-2].json_node); delete (yyvsp[0].json_node);
    }
#line 1597 "parser.tab.c"
    break;

  case 40: /* condition: condition AND condition  */
#line 298 "src/parser.y"
                              {
        (yyval.json_node) = new json({ {"expression_type", "LOGICAL_AND"}, {"left", *(yyvsp[-2].json_node)}, {"right", *(yyvsp[0].json_node)} });
        delete (yyvsp[-2].json_node); delete (yyvsp[0].json_node);
    }
#line 1606 "parser.tab.c"
    break;

  case 41: /* condition: condition OR condition  */
#line 302 "src/parser.y"
                             {
        (yyval.json_node) = new json({ {"expression_type", "LOGICAL_OR"}, {"left", *(yyvsp[-2].json_node)}, {"right", *(yyvsp[0].json_node)} });
        delete (yyvsp[-2].json_node); delete (yyvsp[0].json_node);
    }
#line 1615 "parser.tab.c"
    break;

  case 42: /* condition: LPAREN condition RPAREN  */
#line 306 "src/parser.y"
                              {
        (yyval.json_node) = (yyvsp[-1].json_node); 
    }
#line 1623 "parser.tab.c"
    break;


#line 1627 "parser.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 311 "src/parser.y"


void yyerror(const char *s) 
{
    std::cerr << "Parse Error at line " << yylineno << ": " << s << "\n";
}

nlohmann::json parse_sql_to_json() {
    yyparse();
    return final_ast;
}
