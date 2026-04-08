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
#line 1 "parser.y"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "symbol_table.h"

int yylex(void);
void yyerror(const char *s);
extern int yylineno;
extern FILE *yyin;

/* External declarations from main.c */
extern int dump_tokens;
extern int sa_scan;
extern int sa_parse;
extern FILE *tokfile;
extern void dump_token(const char *type, const char *lexeme, int line);

/* Parser globals */
int main_seen = 0;

typedef struct Param_Type_List {
    Data_Type type;
    struct Param_Type_List *next;
} Param_Type_List;

Param_Type_List *main_decl_params = NULL;

/* Helper function to compare parameter lists */
int compare_param_lists(Param_Type_List *decl_params, Ast_List *def_params) {
    Param_Type_List *d = decl_params;
    Ast_List *f = def_params;

    while (d != NULL && f != NULL) {
        Name_Ast *def_param = (Name_Ast *)f->stmt;

        if (d->type != def_param->entry->type) {
            return 0;
        }

        d = d->next;
        f = f->next;
    }

    return (d == NULL && f == NULL);
}


Param_Type_List *extract_param_types(Ast_List *params) {
    Param_Type_List *head = NULL;
    Param_Type_List *tail = NULL;

    while (params) {
        Name_Ast *param = (Name_Ast *)params->stmt;

        Param_Type_List *node = malloc(sizeof(Param_Type_List));
        node->type = param->entry->type;
        node->next = NULL;

        if (!head) {
            head = node;
            tail = node;
        }
        else {
            tail->next = node;
            tail = node;
        }

        params = params->next;
    }

    return head;
}

Ast *root;
static Data_Type current_decl_type = INT_TYPE;

#line 150 "y.tab.c"

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

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    INT = 258,                     /* INT  */
    FLOAT = 259,                   /* FLOAT  */
    BOOL = 260,                    /* BOOL  */
    STRING = 261,                  /* STRING  */
    VOID = 262,                    /* VOID  */
    IDENTIFIER = 263,              /* IDENTIFIER  */
    INTEGER_NUMBER = 264,          /* INTEGER_NUMBER  */
    FLOAT_NUMBER = 265,            /* FLOAT_NUMBER  */
    STRING_CONSTANT = 266,         /* STRING_CONSTANT  */
    ASSIGNMENT = 267,              /* ASSIGNMENT  */
    PRINT = 268,                   /* PRINT  */
    READ = 269,                    /* READ  */
    IF = 270,                      /* IF  */
    ELSE = 271,                    /* ELSE  */
    WHILE = 272,                   /* WHILE  */
    DO = 273,                      /* DO  */
    EQ = 274,                      /* EQ  */
    NE = 275,                      /* NE  */
    LT = 276,                      /* LT  */
    LE = 277,                      /* LE  */
    GT = 278,                      /* GT  */
    GE = 279,                      /* GE  */
    AND = 280,                     /* AND  */
    OR = 281,                      /* OR  */
    NOT = 282,                     /* NOT  */
    UMINUS = 283,                  /* UMINUS  */
    IFX = 284                      /* IFX  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define INT 258
#define FLOAT 259
#define BOOL 260
#define STRING 261
#define VOID 262
#define IDENTIFIER 263
#define INTEGER_NUMBER 264
#define FLOAT_NUMBER 265
#define STRING_CONSTANT 266
#define ASSIGNMENT 267
#define PRINT 268
#define READ 269
#define IF 270
#define ELSE 271
#define WHILE 272
#define DO 273
#define EQ 274
#define NE 275
#define LT 276
#define LE 277
#define GT 278
#define GE 279
#define AND 280
#define OR 281
#define NOT 282
#define UMINUS 283
#define IFX 284

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 80 "parser.y"

    struct {
        char *lexeme;
        int line;
    } token;
    Ast *ast;
    Ast_List *list;
  Data_Type dtype;

#line 271 "y.tab.c"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_INT = 3,                        /* INT  */
  YYSYMBOL_FLOAT = 4,                      /* FLOAT  */
  YYSYMBOL_BOOL = 5,                       /* BOOL  */
  YYSYMBOL_STRING = 6,                     /* STRING  */
  YYSYMBOL_VOID = 7,                       /* VOID  */
  YYSYMBOL_IDENTIFIER = 8,                 /* IDENTIFIER  */
  YYSYMBOL_INTEGER_NUMBER = 9,             /* INTEGER_NUMBER  */
  YYSYMBOL_FLOAT_NUMBER = 10,              /* FLOAT_NUMBER  */
  YYSYMBOL_STRING_CONSTANT = 11,           /* STRING_CONSTANT  */
  YYSYMBOL_ASSIGNMENT = 12,                /* ASSIGNMENT  */
  YYSYMBOL_PRINT = 13,                     /* PRINT  */
  YYSYMBOL_READ = 14,                      /* READ  */
  YYSYMBOL_IF = 15,                        /* IF  */
  YYSYMBOL_ELSE = 16,                      /* ELSE  */
  YYSYMBOL_WHILE = 17,                     /* WHILE  */
  YYSYMBOL_DO = 18,                        /* DO  */
  YYSYMBOL_EQ = 19,                        /* EQ  */
  YYSYMBOL_NE = 20,                        /* NE  */
  YYSYMBOL_LT = 21,                        /* LT  */
  YYSYMBOL_LE = 22,                        /* LE  */
  YYSYMBOL_GT = 23,                        /* GT  */
  YYSYMBOL_GE = 24,                        /* GE  */
  YYSYMBOL_AND = 25,                       /* AND  */
  YYSYMBOL_OR = 26,                        /* OR  */
  YYSYMBOL_NOT = 27,                       /* NOT  */
  YYSYMBOL_28_ = 28,                       /* '?'  */
  YYSYMBOL_29_ = 29,                       /* ':'  */
  YYSYMBOL_30_ = 30,                       /* '+'  */
  YYSYMBOL_31_ = 31,                       /* '-'  */
  YYSYMBOL_32_ = 32,                       /* '*'  */
  YYSYMBOL_33_ = 33,                       /* '/'  */
  YYSYMBOL_UMINUS = 34,                    /* UMINUS  */
  YYSYMBOL_IFX = 35,                       /* IFX  */
  YYSYMBOL_36_ = 36,                       /* '('  */
  YYSYMBOL_37_ = 37,                       /* ')'  */
  YYSYMBOL_38_ = 38,                       /* ';'  */
  YYSYMBOL_39_ = 39,                       /* ','  */
  YYSYMBOL_40_ = 40,                       /* '{'  */
  YYSYMBOL_41_ = 41,                       /* '}'  */
  YYSYMBOL_YYACCEPT = 42,                  /* $accept  */
  YYSYMBOL_program = 43,                   /* program  */
  YYSYMBOL_global_decl_statement_list = 44, /* global_decl_statement_list  */
  YYSYMBOL_main_decl = 45,                 /* main_decl  */
  YYSYMBOL_formal_param_list = 46,         /* formal_param_list  */
  YYSYMBOL_formal_param = 47,              /* formal_param  */
  YYSYMBOL_param_type = 48,                /* param_type  */
  YYSYMBOL_var_decl_stmt = 49,             /* var_decl_stmt  */
  YYSYMBOL_var_decl_item_list = 50,        /* var_decl_item_list  */
  YYSYMBOL_var_decl_item = 51,             /* var_decl_item  */
  YYSYMBOL_named_type = 52,                /* named_type  */
  YYSYMBOL_void_main_def = 53,             /* void_main_def  */
  YYSYMBOL_54_1 = 54,                      /* $@1  */
  YYSYMBOL_55_2 = 55,                      /* $@2  */
  YYSYMBOL_optional_local_var_decl_stmt_list = 56, /* optional_local_var_decl_stmt_list  */
  YYSYMBOL_var_decl_stmt_list = 57,        /* var_decl_stmt_list  */
  YYSYMBOL_statement_list = 58,            /* statement_list  */
  YYSYMBOL_statement = 59,                 /* statement  */
  YYSYMBOL_block_statement = 60,           /* block_statement  */
  YYSYMBOL_if_statement = 61,              /* if_statement  */
  YYSYMBOL_while_statement = 62,           /* while_statement  */
  YYSYMBOL_assignment_statement = 63,      /* assignment_statement  */
  YYSYMBOL_variable_as_operand = 64,       /* variable_as_operand  */
  YYSYMBOL_variable_name = 65,             /* variable_name  */
  YYSYMBOL_expression = 66,                /* expression  */
  YYSYMBOL_constant_as_operand = 67,       /* constant_as_operand  */
  YYSYMBOL_print_statement = 68,           /* print_statement  */
  YYSYMBOL_read_statement = 69             /* read_statement  */
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
typedef yytype_uint8 yy_state_t;

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
#define YYFINAL  12
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   293

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  42
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  28
/* YYNRULES -- Number of rules.  */
#define YYNRULES  72
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  133

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   284


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
      36,    37,    32,    30,    39,    31,     2,    33,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    29,    38,
       2,     2,     2,    28,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    40,     2,    41,     2,     2,     2,     2,
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
      25,    26,    27,    34,    35
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   131,   131,   136,   144,   145,   154,   155,   168,   182,
     199,   217,   227,   237,   238,   239,   240,   248,   252,   253,
     257,   266,   267,   268,   269,   270,   275,   274,   301,   300,
     330,   331,   335,   336,   340,   350,   356,   357,   358,   359,
     360,   361,   365,   376,   380,   387,   391,   398,   405,   419,
     423,   427,   431,   435,   439,   443,   447,   451,   455,   459,
     463,   467,   471,   475,   479,   483,   487,   491,   498,   502,
     506,   513,   520
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
  "\"end of file\"", "error", "\"invalid token\"", "INT", "FLOAT", "BOOL",
  "STRING", "VOID", "IDENTIFIER", "INTEGER_NUMBER", "FLOAT_NUMBER",
  "STRING_CONSTANT", "ASSIGNMENT", "PRINT", "READ", "IF", "ELSE", "WHILE",
  "DO", "EQ", "NE", "LT", "LE", "GT", "GE", "AND", "OR", "NOT", "'?'",
  "':'", "'+'", "'-'", "'*'", "'/'", "UMINUS", "IFX", "'('", "')'", "';'",
  "','", "'{'", "'}'", "$accept", "program", "global_decl_statement_list",
  "main_decl", "formal_param_list", "formal_param", "param_type",
  "var_decl_stmt", "var_decl_item_list", "var_decl_item", "named_type",
  "void_main_def", "$@1", "$@2", "optional_local_var_decl_stmt_list",
  "var_decl_stmt_list", "statement_list", "statement", "block_statement",
  "if_statement", "while_statement", "assignment_statement",
  "variable_as_operand", "variable_name", "expression",
  "constant_as_operand", "print_statement", "read_statement", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-66)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      36,   -66,   -66,   -66,   -66,   -66,    14,    36,   -66,   -66,
      40,   -66,   -66,   -66,   -66,   -66,     8,   -20,   -66,    32,
     -66,    45,   -66,   -66,   -66,   -66,   -23,    10,   -66,    56,
     -66,   -66,   -66,   -66,    20,    67,   -66,    36,   -66,   -66,
     -66,   -66,    45,   -66,    36,    36,    37,   -66,   -66,   -66,
      96,    76,    60,    61,    85,   -66,   -66,   -66,   -66,   -66,
     -66,   -66,   103,   -66,   -66,   -66,    48,   -66,   -66,   -66,
      96,    96,    96,   -66,   114,   -66,    63,    96,    96,    99,
      77,    96,   -66,   260,   -66,   154,    96,    96,    96,    96,
      96,    96,    96,    96,    96,    96,    96,    96,    96,   -66,
     -66,   173,   192,    88,   -66,   134,   -66,    49,    49,    49,
      49,    49,    49,   260,    89,   230,    54,    54,   -66,   -66,
      85,    85,    96,   -66,    96,   110,   -66,   211,   245,    85,
      90,   -66,   -66
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,    21,    22,    23,    24,    25,     0,     0,     7,     6,
       0,     3,     1,     5,     4,     2,    20,     0,    19,     0,
      17,     0,    13,    14,    15,    16,     0,     0,    11,     0,
      20,    18,     9,    26,     0,     0,    12,    31,     8,    28,
      10,    33,     0,    35,    30,    31,     0,    32,    35,    49,
       0,     0,     0,     0,     0,    35,    27,    34,    41,    39,
      40,    36,     0,    48,    37,    38,     0,    68,    69,    70,
       0,     0,     0,    66,     0,    67,     0,     0,     0,     0,
       0,     0,    29,    55,    54,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    71,
      72,     0,     0,     0,    42,     0,    65,    56,    57,    58,
      59,    60,    61,    62,    63,     0,    50,    51,    52,    53,
       0,     0,     0,    47,     0,    43,    45,     0,    64,     0,
       0,    44,    46
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -66,   -66,   -66,   122,   -66,    95,   -66,     2,   -66,   120,
       3,   124,   -66,   -66,    98,   -66,   -44,   -53,   -66,   -66,
     -66,   -66,   -46,    97,   -65,   -66,   -66,   -66
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     6,     7,     8,    27,    28,    29,    41,    17,    18,
      42,    11,    37,    45,    43,    44,    46,    57,    58,    59,
      60,    61,    73,    63,    74,    75,    64,    65
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      62,    79,     9,    10,    66,    83,    84,    85,    62,    14,
      10,    80,   101,   102,    12,    32,   105,    33,    20,    21,
      62,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,    62,    22,    23,    24,    25,     1,
       2,     3,     4,     5,    19,    49,    47,    34,    16,    35,
      50,    51,    52,    30,    53,    54,    49,   127,    38,   128,
      39,    50,    51,    52,    36,    53,    54,   125,   126,    26,
      22,    23,    24,    25,    62,    62,   131,    55,    56,    95,
      96,    97,    98,    62,    49,    49,    97,    98,    55,    82,
      50,    51,    52,    49,    53,    54,    77,    78,    50,    51,
      52,   100,    53,    54,    49,    67,    68,    69,    86,    87,
      88,    89,    90,    91,    92,    81,   103,    55,   104,    95,
      96,    97,    98,    70,   122,    55,   129,    71,   132,    13,
      40,    15,    72,    86,    87,    88,    89,    90,    91,    92,
      93,    31,    94,    48,    95,    96,    97,    98,    76,     0,
       0,     0,    99,    86,    87,    88,    89,    90,    91,    92,
      93,     0,    94,     0,    95,    96,    97,    98,     0,     0,
       0,     0,   123,    86,    87,    88,    89,    90,    91,    92,
      93,     0,    94,     0,    95,    96,    97,    98,     0,     0,
       0,   106,    86,    87,    88,    89,    90,    91,    92,    93,
       0,    94,     0,    95,    96,    97,    98,     0,     0,     0,
     120,    86,    87,    88,    89,    90,    91,    92,    93,     0,
      94,     0,    95,    96,    97,    98,     0,     0,     0,   121,
      86,    87,    88,    89,    90,    91,    92,    93,     0,    94,
       0,    95,    96,    97,    98,     0,     0,     0,   130,    86,
      87,    88,    89,    90,    91,    92,    93,     0,    94,   124,
      95,    96,    97,    98,    86,    87,    88,    89,    90,    91,
      92,    93,     0,    94,     0,    95,    96,    97,    98,    86,
      87,    88,    89,    90,    91,     0,     0,     0,     0,     0,
      95,    96,    97,    98
};

static const yytype_int16 yycheck[] =
{
      46,    54,     0,     0,    48,    70,    71,    72,    54,     7,
       7,    55,    77,    78,     0,    38,    81,    40,    38,    39,
      66,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    80,     3,     4,     5,     6,     3,
       4,     5,     6,     7,    36,     8,    44,    37,     8,    39,
      13,    14,    15,     8,    17,    18,     8,   122,    38,   124,
      40,    13,    14,    15,     8,    17,    18,   120,   121,    37,
       3,     4,     5,     6,   120,   121,   129,    40,    41,    30,
      31,    32,    33,   129,     8,     8,    32,    33,    40,    41,
      13,    14,    15,     8,    17,    18,    36,    36,    13,    14,
      15,    38,    17,    18,     8,     9,    10,    11,    19,    20,
      21,    22,    23,    24,    25,    12,    17,    40,    41,    30,
      31,    32,    33,    27,    36,    40,    16,    31,    38,     7,
      35,     7,    36,    19,    20,    21,    22,    23,    24,    25,
      26,    21,    28,    45,    30,    31,    32,    33,    51,    -1,
      -1,    -1,    38,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,    28,    -1,    30,    31,    32,    33,    -1,    -1,
      -1,    -1,    38,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,    28,    -1,    30,    31,    32,    33,    -1,    -1,
      -1,    37,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    28,    -1,    30,    31,    32,    33,    -1,    -1,    -1,
      37,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
      28,    -1,    30,    31,    32,    33,    -1,    -1,    -1,    37,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,    28,
      -1,    30,    31,    32,    33,    -1,    -1,    -1,    37,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    28,    29,
      30,    31,    32,    33,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    28,    -1,    30,    31,    32,    33,    19,
      20,    21,    22,    23,    24,    -1,    -1,    -1,    -1,    -1,
      30,    31,    32,    33
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,     4,     5,     6,     7,    43,    44,    45,    49,
      52,    53,     0,    45,    49,    53,     8,    50,    51,    36,
      38,    39,     3,     4,     5,     6,    37,    46,    47,    48,
       8,    51,    38,    40,    37,    39,     8,    54,    38,    40,
      47,    49,    52,    56,    57,    55,    58,    49,    56,     8,
      13,    14,    15,    17,    18,    40,    41,    59,    60,    61,
      62,    63,    64,    65,    68,    69,    58,     9,    10,    11,
      27,    31,    36,    64,    66,    67,    65,    36,    36,    59,
      58,    12,    41,    66,    66,    66,    19,    20,    21,    22,
      23,    24,    25,    26,    28,    30,    31,    32,    33,    38,
      38,    66,    66,    17,    41,    66,    37,    66,    66,    66,
      66,    66,    66,    66,    66,    66,    66,    66,    66,    66,
      37,    37,    36,    38,    29,    59,    59,    66,    66,    16,
      37,    59,    38
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    42,    43,    43,    44,    44,    44,    44,    45,    45,
      46,    46,    47,    48,    48,    48,    48,    49,    50,    50,
      51,    52,    52,    52,    52,    52,    54,    53,    55,    53,
      56,    56,    57,    57,    58,    58,    59,    59,    59,    59,
      59,    59,    60,    61,    61,    62,    62,    63,    64,    65,
      66,    66,    66,    66,    66,    66,    66,    66,    66,    66,
      66,    66,    66,    66,    66,    66,    66,    66,    67,    67,
      67,    68,    69
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     1,     2,     2,     1,     1,     6,     5,
       3,     1,     2,     1,     1,     1,     1,     3,     3,     1,
       1,     1,     1,     1,     1,     1,     0,     9,     0,    10,
       1,     0,     2,     1,     2,     0,     1,     1,     1,     1,
       1,     1,     3,     5,     7,     5,     7,     4,     1,     1,
       3,     3,     3,     3,     2,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     5,     3,     1,     1,     1,     1,
       1,     3,     3
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
  case 2: /* program: global_decl_statement_list void_main_def  */
#line 132 "parser.y"
      {
          root = make_program_ast(yylineno);
          program_append((Program_Ast *)root, (yyvsp[0].ast));
      }
#line 1444 "y.tab.c"
    break;

  case 3: /* program: void_main_def  */
#line 137 "parser.y"
      {
          root = make_program_ast(yylineno);
          program_append((Program_Ast *)root, (yyvsp[0].ast));
      }
#line 1453 "y.tab.c"
    break;

  case 5: /* global_decl_statement_list: global_decl_statement_list main_decl  */
#line 146 "parser.y"
      {
          if (!sa_parse && main_seen) {
              yyerror("multiple declarations/definitions of functions");
              YYERROR;
          }
          main_seen = 1;
          clear_local_scope();
      }
#line 1466 "y.tab.c"
    break;

  case 7: /* global_decl_statement_list: main_decl  */
#line 156 "parser.y"
      {
          if (!sa_parse && main_seen) {
              yyerror("multiple declarations/definitions of main");
              YYERROR;
          }
          main_seen = 1;
          clear_local_scope();
      }
#line 1479 "y.tab.c"
    break;

  case 8: /* main_decl: named_type IDENTIFIER '(' formal_param_list ')' ';'  */
#line 169 "parser.y"
      {
          if (!sa_parse) {
              if (strcmp((yyvsp[-4].token).lexeme, "main") != 0) {
                  yyerror("function declaration must be 'main'");
                  YYERROR;
              }
              if (current_decl_type != VOID_TYPE) {
                  yyerror("main function must have void return type");
                  YYERROR;
              }
          }
          main_decl_params = extract_param_types((yyvsp[-2].list));                    
      }
#line 1497 "y.tab.c"
    break;

  case 9: /* main_decl: named_type IDENTIFIER '(' ')' ';'  */
#line 183 "parser.y"
      {
          if (!sa_parse) {
              if (strcmp((yyvsp[-3].token).lexeme, "main") != 0) {
                  yyerror("function declaration must be 'main'");
                  YYERROR;
              }
              if (current_decl_type != VOID_TYPE) {
                  yyerror("main function must have void return type");
                  YYERROR;
              }
          }
          main_decl_params = NULL;  /* No parameters */
      }
#line 1515 "y.tab.c"
    break;

  case 10: /* formal_param_list: formal_param_list ',' formal_param  */
#line 200 "parser.y"
      {
          
          Ast_List *new_node = malloc(sizeof(Ast_List));
          new_node->stmt = (yyvsp[0].ast);
          new_node->next = NULL;
          
          if ((yyvsp[-2].list) == NULL) {
              (yyval.list) = new_node;
          } else {
              Ast_List *temp = (yyvsp[-2].list);
              while (temp->next)
                  temp = temp->next;
              temp->next = new_node;
              (yyval.list) = (yyvsp[-2].list);
          }
          
      }
#line 1537 "y.tab.c"
    break;

  case 11: /* formal_param_list: formal_param  */
#line 218 "parser.y"
      {
          Ast_List *new_node = malloc(sizeof(Ast_List));                    
          new_node->stmt = (yyvsp[0].ast);
          new_node->next = NULL;
          (yyval.list) = new_node;
      }
#line 1548 "y.tab.c"
    break;

  case 12: /* formal_param: param_type IDENTIFIER  */
#line 228 "parser.y"
      {   set_scope(LOCAL_SCOPE);
          Symbol_Table_Entry *entry = insert_symbol((yyvsp[0].token).lexeme, (yyvsp[-1].dtype));                    
          (yyval.ast) = make_name_ast(entry, (yyvsp[0].token).line);
          set_scope(GLOBAL_SCOPE);
      }
#line 1558 "y.tab.c"
    break;

  case 13: /* param_type: INT  */
#line 237 "parser.y"
                { (yyval.dtype) = INT_TYPE; }
#line 1564 "y.tab.c"
    break;

  case 14: /* param_type: FLOAT  */
#line 238 "parser.y"
                { (yyval.dtype) = FLOAT_TYPE; }
#line 1570 "y.tab.c"
    break;

  case 15: /* param_type: BOOL  */
#line 239 "parser.y"
                { (yyval.dtype) = BOOL_TYPE; }
#line 1576 "y.tab.c"
    break;

  case 16: /* param_type: STRING  */
#line 240 "parser.y"
                { (yyval.dtype) = STRING_TYPE; }
#line 1582 "y.tab.c"
    break;

  case 20: /* var_decl_item: IDENTIFIER  */
#line 258 "parser.y"
      {     
          //printf("%s, %d\n", $1.lexeme, (int)current_decl_type);
          insert_symbol((yyvsp[0].token).lexeme, current_decl_type);
          //print_symbol_table();
      }
#line 1592 "y.tab.c"
    break;

  case 21: /* named_type: INT  */
#line 266 "parser.y"
             { current_decl_type = INT_TYPE; }
#line 1598 "y.tab.c"
    break;

  case 22: /* named_type: FLOAT  */
#line 267 "parser.y"
             { current_decl_type = FLOAT_TYPE; }
#line 1604 "y.tab.c"
    break;

  case 23: /* named_type: BOOL  */
#line 268 "parser.y"
             { current_decl_type = BOOL_TYPE; }
#line 1610 "y.tab.c"
    break;

  case 24: /* named_type: STRING  */
#line 269 "parser.y"
             { current_decl_type = STRING_TYPE; }
#line 1616 "y.tab.c"
    break;

  case 25: /* named_type: VOID  */
#line 270 "parser.y"
             { current_decl_type = VOID_TYPE; }
#line 1622 "y.tab.c"
    break;

  case 26: /* $@1: %empty  */
#line 275 "parser.y"
      {
          if (!sa_parse) {
              if (strcmp((yyvsp[-3].token).lexeme, "main") != 0) {
                  yyerror("function definition must be 'main'");
                  YYERROR;
              }
              if (current_decl_type != VOID_TYPE) {
                  yyerror("main function must have void return type");
                  YYERROR;
              }
              /* Check if declaration exists and parameters match */
              if (main_seen && main_decl_params != NULL) {
                  yyerror("function definition parameter list does not match declaration");
                  YYERROR;
              }
          }
          set_scope(LOCAL_SCOPE);
      }
#line 1645 "y.tab.c"
    break;

  case 27: /* void_main_def: named_type IDENTIFIER '(' ')' '{' $@1 optional_local_var_decl_stmt_list statement_list '}'  */
#line 296 "parser.y"
      {
          (yyval.ast) = make_procedure_ast((yyvsp[-7].token).lexeme, NULL, (yyvsp[-1].ast), (yyvsp[-7].token).line);
          set_scope(GLOBAL_SCOPE);
      }
#line 1654 "y.tab.c"
    break;

  case 28: /* $@2: %empty  */
#line 301 "parser.y"
      {
          if (!sa_parse) {
              if (strcmp((yyvsp[-4].token).lexeme, "main") != 0) {
                  yyerror("function definition must be 'main'");
                  YYERROR;
              }
              if (current_decl_type != VOID_TYPE) {
                  yyerror("main function must have void return type");
                  YYERROR;
              }
              /* Check if declaration exists and parameters match */
              
              if (main_seen && !compare_param_lists(main_decl_params, (yyvsp[-2].list))) {
                  yyerror("function definition parameter list does not match declaration");
                  YYERROR;
              }
          }
          set_scope(LOCAL_SCOPE);
      }
#line 1678 "y.tab.c"
    break;

  case 29: /* void_main_def: named_type IDENTIFIER '(' formal_param_list ')' '{' $@2 optional_local_var_decl_stmt_list statement_list '}'  */
#line 323 "parser.y"
      {
          (yyval.ast) = make_procedure_ast((yyvsp[-8].token).lexeme, (yyvsp[-6].list), (yyvsp[-1].ast), (yyvsp[-8].token).line);
          set_scope(GLOBAL_SCOPE);
      }
#line 1687 "y.tab.c"
    break;

  case 34: /* statement_list: statement_list statement  */
#line 341 "parser.y"
      {
          if ((yyvsp[-1].ast) == NULL) {
              (yyval.ast) = make_sequence_ast(yylineno);
          } else {
              (yyval.ast) = (yyvsp[-1].ast);
          }
          sequence_append((Sequence_Ast *)(yyval.ast), (yyvsp[0].ast));
      }
#line 1700 "y.tab.c"
    break;

  case 35: /* statement_list: %empty  */
#line 350 "parser.y"
      {
          (yyval.ast) = NULL;
      }
#line 1708 "y.tab.c"
    break;

  case 36: /* statement: assignment_statement  */
#line 356 "parser.y"
                            { (yyval.ast) = (yyvsp[0].ast); }
#line 1714 "y.tab.c"
    break;

  case 37: /* statement: print_statement  */
#line 357 "parser.y"
                            { (yyval.ast) = (yyvsp[0].ast); }
#line 1720 "y.tab.c"
    break;

  case 38: /* statement: read_statement  */
#line 358 "parser.y"
                            { (yyval.ast) = (yyvsp[0].ast); }
#line 1726 "y.tab.c"
    break;

  case 39: /* statement: if_statement  */
#line 359 "parser.y"
                            { (yyval.ast) = (yyvsp[0].ast); }
#line 1732 "y.tab.c"
    break;

  case 40: /* statement: while_statement  */
#line 360 "parser.y"
                            { (yyval.ast) = (yyvsp[0].ast); }
#line 1738 "y.tab.c"
    break;

  case 41: /* statement: block_statement  */
#line 361 "parser.y"
                            { (yyval.ast) = (yyvsp[0].ast); }
#line 1744 "y.tab.c"
    break;

  case 42: /* block_statement: '{' statement_list '}'  */
#line 366 "parser.y"
      {
          if ((yyvsp[-1].ast)) {
              (yyval.ast) = (yyvsp[-1].ast);
          } else {
              (yyval.ast) = make_sequence_ast(yylineno);
          }
      }
#line 1756 "y.tab.c"
    break;

  case 43: /* if_statement: IF '(' expression ')' statement  */
#line 377 "parser.y"
      {
          (yyval.ast) = make_if_else_stmt_ast((yyvsp[-2].ast), (yyvsp[0].ast), NULL, (yyvsp[-4].token).line);
      }
#line 1764 "y.tab.c"
    break;

  case 44: /* if_statement: IF '(' expression ')' statement ELSE statement  */
#line 381 "parser.y"
      {
          (yyval.ast) = make_if_else_stmt_ast((yyvsp[-4].ast), (yyvsp[-2].ast), (yyvsp[0].ast), (yyvsp[-6].token).line);
      }
#line 1772 "y.tab.c"
    break;

  case 45: /* while_statement: WHILE '(' expression ')' statement  */
#line 388 "parser.y"
      {
          (yyval.ast) = make_while_ast((yyvsp[-2].ast), (yyvsp[0].ast), 0, (yyvsp[-4].token).line);
      }
#line 1780 "y.tab.c"
    break;

  case 46: /* while_statement: DO statement WHILE '(' expression ')' ';'  */
#line 392 "parser.y"
            {
                    (yyval.ast) = make_while_ast((yyvsp[-2].ast), (yyvsp[-5].ast), 1, (yyvsp[-6].token).line);
            }
#line 1788 "y.tab.c"
    break;

  case 47: /* assignment_statement: variable_as_operand ASSIGNMENT expression ';'  */
#line 399 "parser.y"
      {
          (yyval.ast) = make_assignment_ast((yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[-2].token).line);
      }
#line 1796 "y.tab.c"
    break;

  case 48: /* variable_as_operand: variable_name  */
#line 406 "parser.y"
      {
          Symbol_Table_Entry *entry = lookup_symbol((yyvsp[0].token).lexeme);
          if (!sa_parse && !entry) {
              char buf[256];
              snprintf(buf, sizeof(buf), "Variable '%s' not declared", (yyvsp[0].token).lexeme);
              yyerror(buf);
              YYERROR;
          }
          (yyval.ast) = make_name_ast(entry, (yyvsp[0].token).line);
      }
#line 1811 "y.tab.c"
    break;

  case 50: /* expression: expression '+' expression  */
#line 424 "parser.y"
      {
          (yyval.ast) = make_binary_ast(AST_PLUS, (yyvsp[-2].ast), (yyvsp[0].ast), yylineno);
      }
#line 1819 "y.tab.c"
    break;

  case 51: /* expression: expression '-' expression  */
#line 428 "parser.y"
      {
          (yyval.ast) = make_binary_ast(AST_MINUS, (yyvsp[-2].ast), (yyvsp[0].ast), yylineno);
      }
#line 1827 "y.tab.c"
    break;

  case 52: /* expression: expression '*' expression  */
#line 432 "parser.y"
      {
          (yyval.ast) = make_binary_ast(AST_MULT, (yyvsp[-2].ast), (yyvsp[0].ast), yylineno);
      }
#line 1835 "y.tab.c"
    break;

  case 53: /* expression: expression '/' expression  */
#line 436 "parser.y"
      {
          (yyval.ast) = make_binary_ast(AST_DIV, (yyvsp[-2].ast), (yyvsp[0].ast), yylineno);
      }
#line 1843 "y.tab.c"
    break;

  case 54: /* expression: '-' expression  */
#line 440 "parser.y"
      {
          (yyval.ast) = make_unary_ast(AST_UMINUS, (yyvsp[0].ast), yylineno);
      }
#line 1851 "y.tab.c"
    break;

  case 55: /* expression: NOT expression  */
#line 444 "parser.y"
      {
          (yyval.ast) = make_unary_ast(AST_NOT, (yyvsp[0].ast), yylineno);
      }
#line 1859 "y.tab.c"
    break;

  case 56: /* expression: expression EQ expression  */
#line 448 "parser.y"
      {
          (yyval.ast) = make_relational_ast((yyvsp[-2].ast), REL_EQ, (yyvsp[0].ast), yylineno);
      }
#line 1867 "y.tab.c"
    break;

  case 57: /* expression: expression NE expression  */
#line 452 "parser.y"
      {
          (yyval.ast) = make_relational_ast((yyvsp[-2].ast), REL_NE, (yyvsp[0].ast), yylineno);
      }
#line 1875 "y.tab.c"
    break;

  case 58: /* expression: expression LT expression  */
#line 456 "parser.y"
      {
          (yyval.ast) = make_relational_ast((yyvsp[-2].ast), REL_LT, (yyvsp[0].ast), yylineno);
      }
#line 1883 "y.tab.c"
    break;

  case 59: /* expression: expression LE expression  */
#line 460 "parser.y"
      {
          (yyval.ast) = make_relational_ast((yyvsp[-2].ast), REL_LE, (yyvsp[0].ast), yylineno);
      }
#line 1891 "y.tab.c"
    break;

  case 60: /* expression: expression GT expression  */
#line 464 "parser.y"
      {
          (yyval.ast) = make_relational_ast((yyvsp[-2].ast), REL_GT, (yyvsp[0].ast), yylineno);
      }
#line 1899 "y.tab.c"
    break;

  case 61: /* expression: expression GE expression  */
#line 468 "parser.y"
      {
          (yyval.ast) = make_relational_ast((yyvsp[-2].ast), REL_GE, (yyvsp[0].ast), yylineno);
      }
#line 1907 "y.tab.c"
    break;

  case 62: /* expression: expression AND expression  */
#line 472 "parser.y"
      {
          (yyval.ast) = make_logical_ast((yyvsp[-2].ast), LOGICAL_AND, (yyvsp[0].ast), yylineno);
      }
#line 1915 "y.tab.c"
    break;

  case 63: /* expression: expression OR expression  */
#line 476 "parser.y"
      {
          (yyval.ast) = make_logical_ast((yyvsp[-2].ast), LOGICAL_OR, (yyvsp[0].ast), yylineno);
      }
#line 1923 "y.tab.c"
    break;

  case 64: /* expression: expression '?' expression ':' expression  */
#line 480 "parser.y"
      {
          (yyval.ast) = make_if_ast((yyvsp[-4].ast), (yyvsp[-2].ast), (yyvsp[0].ast), yylineno);
      }
#line 1931 "y.tab.c"
    break;

  case 65: /* expression: '(' expression ')'  */
#line 484 "parser.y"
      {
          (yyval.ast) = (yyvsp[-1].ast);
      }
#line 1939 "y.tab.c"
    break;

  case 66: /* expression: variable_as_operand  */
#line 488 "parser.y"
      {
          (yyval.ast) = (yyvsp[0].ast);
      }
#line 1947 "y.tab.c"
    break;

  case 67: /* expression: constant_as_operand  */
#line 492 "parser.y"
      {
          (yyval.ast) = (yyvsp[0].ast);
      }
#line 1955 "y.tab.c"
    break;

  case 68: /* constant_as_operand: INTEGER_NUMBER  */
#line 499 "parser.y"
      {
          (yyval.ast) = make_number_ast((yyvsp[0].token).lexeme, INT_TYPE, (yyvsp[0].token).line);
      }
#line 1963 "y.tab.c"
    break;

  case 69: /* constant_as_operand: FLOAT_NUMBER  */
#line 503 "parser.y"
      {
          (yyval.ast) = make_number_ast((yyvsp[0].token).lexeme, FLOAT_TYPE, (yyvsp[0].token).line);
      }
#line 1971 "y.tab.c"
    break;

  case 70: /* constant_as_operand: STRING_CONSTANT  */
#line 507 "parser.y"
      {
          (yyval.ast) = make_number_ast((yyvsp[0].token).lexeme, STRING_TYPE, (yyvsp[0].token).line);
      }
#line 1979 "y.tab.c"
    break;

  case 71: /* print_statement: PRINT expression ';'  */
#line 514 "parser.y"
      {
          (yyval.ast) = make_print_ast((yyvsp[-1].ast), (yyvsp[-2].token).line);
      }
#line 1987 "y.tab.c"
    break;

  case 72: /* read_statement: READ variable_name ';'  */
#line 521 "parser.y"
      {
          Symbol_Table_Entry *entry = lookup_symbol((yyvsp[-1].token).lexeme);
          if (!sa_parse && !entry) {
              char buf[256];
              snprintf(buf, sizeof(buf), "Variable '%s' not declared", (yyvsp[-1].token).lexeme);
              yyerror(buf);
              YYERROR;
          }
          Ast *var = make_name_ast(entry, (yyvsp[-1].token).line);
          (yyval.ast) = make_read_ast(var, (yyvsp[-2].token).line);
      }
#line 2003 "y.tab.c"
    break;


#line 2007 "y.tab.c"

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

#line 534 "parser.y"


void yyerror(const char *s) {
    printf("Parse error at line %d: %s\n", yylineno, s);
}
