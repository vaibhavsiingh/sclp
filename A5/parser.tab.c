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
int main_defined = 0;

Ast *root;
static Data_Type current_decl_type = INT_TYPE;
static Data_Type current_func_return_type = INT_TYPE;
static int collecting_function_locals = 0;
static int current_function_local_bytes = 0;
static Ast_List *current_function_locals = NULL;

static void append_ast_list_node(Ast_List **head, Ast *stmt)
{
    Ast_List *node;
    Ast_List *cur;

    if (!head || !stmt)
        return;

    node = (Ast_List *)malloc(sizeof(Ast_List));
    if (!node)
    {
        yyerror("out of memory while recording locals");
        return;
    }

    node->stmt = stmt;
    node->next = NULL;

    if (!*head)
    {
        *head = node;
        return;
    }

    cur = *head;
    while (cur->next)
        cur = cur->next;
    cur->next = node;
}

static int type_size_bytes(Data_Type type)
{
    switch (type)
    {
    case VOID_TYPE:
        return 0;
    default:
        return 4;
    }
}

static int register_function(const char *name, Data_Type ret_type,
                             Ast_List *params_ast, int is_definition, int line)
{
    (void)line;
    Param_Type_List *params = param_types_from_ast_list(params_ast);
    Function_Entry *entry = lookup_function(name);

    if (entry)
    {
        if (!is_definition)
        {
            yyerror("multiple declarations/definitions of functions");
            free_param_type_list(params);
            return 0;
        }

        if (entry->defined)
        {
            yyerror("multiple declarations/definitions of functions");
            free_param_type_list(params);
            return 0;
        }

        if (entry->return_type != ret_type ||
            !compare_param_type_lists(entry->params, params))
        {
            yyerror("function definition parameter list does not match declaration");
            free_param_type_list(params);
            return 0;
        }

        entry->defined = 1;
        free_param_type_list(params);
    }
    else
    {
        if (!add_function_entry(name, ret_type, params, is_definition))
        {
            yyerror("unable to record function declaration");
            free_param_type_list(params);
            return 0;
        }
    }

    if (is_definition && strcmp(name, "main") == 0)
        main_defined = 1;

    return 1;
}

static int same_procedure_signature(const Procedure_Ast *lhs, const Procedure_Ast *rhs)
{
    Param_Type_List *lhs_params;
    Param_Type_List *rhs_params;
    int same = 0;

    if (!lhs || !rhs)
        return 0;

    if (!lhs->name || !rhs->name)
        return 0;

    if (strcmp(lhs->name, rhs->name) != 0)
        return 0;

    if (lhs->return_type != rhs->return_type)
        return 0;

    lhs_params = param_types_from_ast_list(lhs->params);
    rhs_params = param_types_from_ast_list(rhs->params);

    same = compare_param_type_lists(lhs_params, rhs_params);

    free_param_type_list(lhs_params);
    free_param_type_list(rhs_params);

    return same;
}

static Procedure_Ast *find_matching_decl(Program_Ast *prog, const Procedure_Ast *def_proc)
{
    Ast_List *cur;

    if (!prog || !def_proc)
        return NULL;

    cur = prog->procedures;
    while (cur)
    {
        if (cur->stmt && cur->stmt->kind == AST_PROCEDURE)
        {
            Procedure_Ast *proc = (Procedure_Ast *)cur->stmt;
            if (!proc->has_body && same_procedure_signature(proc, def_proc))
                return proc;
        }
        cur = cur->next;
    }

    return NULL;
}

static void merge_external_node(Program_Ast *prog, Ast *node)
{
    if (!prog || !node)
        return;

    if (node->kind == AST_PROCEDURE && ((Procedure_Ast *)node)->has_body)
    {
        Procedure_Ast *def_proc = (Procedure_Ast *)node;
        Procedure_Ast *decl_proc = find_matching_decl(prog, def_proc);

        if (decl_proc)
        {
            decl_proc->has_body = 1;
            decl_proc->body = def_proc->body;
            decl_proc->local_var_bytes = def_proc->local_var_bytes;
            decl_proc->locals = def_proc->locals;
        }
        else
        {
            program_append(prog, node);
        }
    }
    else
    {
        program_append(prog, node);
    }
}

#line 271 "parser.tab.c"

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
  YYSYMBOL_RETURN = 15,                    /* RETURN  */
  YYSYMBOL_IF = 16,                        /* IF  */
  YYSYMBOL_ELSE = 17,                      /* ELSE  */
  YYSYMBOL_WHILE = 18,                     /* WHILE  */
  YYSYMBOL_DO = 19,                        /* DO  */
  YYSYMBOL_EQ = 20,                        /* EQ  */
  YYSYMBOL_NE = 21,                        /* NE  */
  YYSYMBOL_LT = 22,                        /* LT  */
  YYSYMBOL_LE = 23,                        /* LE  */
  YYSYMBOL_GT = 24,                        /* GT  */
  YYSYMBOL_GE = 25,                        /* GE  */
  YYSYMBOL_AND = 26,                       /* AND  */
  YYSYMBOL_OR = 27,                        /* OR  */
  YYSYMBOL_NOT = 28,                       /* NOT  */
  YYSYMBOL_29_ = 29,                       /* '?'  */
  YYSYMBOL_30_ = 30,                       /* ':'  */
  YYSYMBOL_31_ = 31,                       /* '+'  */
  YYSYMBOL_32_ = 32,                       /* '-'  */
  YYSYMBOL_33_ = 33,                       /* '*'  */
  YYSYMBOL_34_ = 34,                       /* '/'  */
  YYSYMBOL_UMINUS = 35,                    /* UMINUS  */
  YYSYMBOL_IFX = 36,                       /* IFX  */
  YYSYMBOL_37_ = 37,                       /* ','  */
  YYSYMBOL_38_ = 38,                       /* '('  */
  YYSYMBOL_39_ = 39,                       /* ')'  */
  YYSYMBOL_40_ = 40,                       /* ';'  */
  YYSYMBOL_41_ = 41,                       /* '{'  */
  YYSYMBOL_42_ = 42,                       /* '}'  */
  YYSYMBOL_YYACCEPT = 43,                  /* $accept  */
  YYSYMBOL_program = 44,                   /* program  */
  YYSYMBOL_external_decl_list = 45,        /* external_decl_list  */
  YYSYMBOL_external_decl_section = 46,     /* external_decl_section  */
  YYSYMBOL_external_decl_decl = 47,        /* external_decl_decl  */
  YYSYMBOL_function_def_section = 48,      /* function_def_section  */
  YYSYMBOL_formal_param_list = 49,         /* formal_param_list  */
  YYSYMBOL_formal_param = 50,              /* formal_param  */
  YYSYMBOL_param_type = 51,                /* param_type  */
  YYSYMBOL_function_decl = 52,             /* function_decl  */
  YYSYMBOL_function_def = 53,              /* function_def  */
  YYSYMBOL_54_1 = 54,                      /* $@1  */
  YYSYMBOL_55_2 = 55,                      /* $@2  */
  YYSYMBOL_var_decl_stmt = 56,             /* var_decl_stmt  */
  YYSYMBOL_var_decl_item_list = 57,        /* var_decl_item_list  */
  YYSYMBOL_var_decl_item = 58,             /* var_decl_item  */
  YYSYMBOL_named_type = 59,                /* named_type  */
  YYSYMBOL_optional_local_var_decl_stmt_list = 60, /* optional_local_var_decl_stmt_list  */
  YYSYMBOL_var_decl_stmt_list = 61,        /* var_decl_stmt_list  */
  YYSYMBOL_statement_list = 62,            /* statement_list  */
  YYSYMBOL_statement = 63,                 /* statement  */
  YYSYMBOL_block_statement = 64,           /* block_statement  */
  YYSYMBOL_if_statement = 65,              /* if_statement  */
  YYSYMBOL_while_statement = 66,           /* while_statement  */
  YYSYMBOL_assignment_statement = 67,      /* assignment_statement  */
  YYSYMBOL_variable_as_operand = 68,       /* variable_as_operand  */
  YYSYMBOL_variable_name = 69,             /* variable_name  */
  YYSYMBOL_function_call = 70,             /* function_call  */
  YYSYMBOL_argument_list = 71,             /* argument_list  */
  YYSYMBOL_expression = 72,                /* expression  */
  YYSYMBOL_constant_as_operand = 73,       /* constant_as_operand  */
  YYSYMBOL_print_statement = 74,           /* print_statement  */
  YYSYMBOL_read_statement = 75,            /* read_statement  */
  YYSYMBOL_return_statement = 76,          /* return_statement  */
  YYSYMBOL_call_statement = 77             /* call_statement  */
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
#define YYFINAL  4
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   392

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  43
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  35
/* YYNRULES -- Number of rules.  */
#define YYNRULES  84
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  156

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   285


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
      38,    39,    33,    31,    37,    32,     2,    34,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    30,    40,
       2,     2,     2,    29,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    41,     2,    42,     2,     2,     2,     2,
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
      25,    26,    27,    28,    35,    36
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   263,   263,   274,   288,   296,   302,   306,   313,   321,
     338,   356,   366,   376,   377,   378,   379,   382,   391,   404,
     403,   429,   428,   461,   465,   466,   470,   483,   484,   485,
     486,   487,   491,   492,   496,   497,   501,   511,   517,   518,
     519,   520,   521,   522,   523,   524,   528,   539,   543,   550,
     554,   561,   568,   582,   586,   596,   609,   625,   635,   639,
     643,   647,   651,   655,   659,   663,   667,   671,   675,   679,
     683,   687,   691,   695,   699,   703,   707,   714,   718,   722,
     729,   736,   751,   755,   762
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
  "STRING_CONSTANT", "ASSIGNMENT", "PRINT", "READ", "RETURN", "IF", "ELSE",
  "WHILE", "DO", "EQ", "NE", "LT", "LE", "GT", "GE", "AND", "OR", "NOT",
  "'?'", "':'", "'+'", "'-'", "'*'", "'/'", "UMINUS", "IFX", "','", "'('",
  "')'", "';'", "'{'", "'}'", "$accept", "program", "external_decl_list",
  "external_decl_section", "external_decl_decl", "function_def_section",
  "formal_param_list", "formal_param", "param_type", "function_decl",
  "function_def", "$@1", "$@2", "var_decl_stmt", "var_decl_item_list",
  "var_decl_item", "named_type", "optional_local_var_decl_stmt_list",
  "var_decl_stmt_list", "statement_list", "statement", "block_statement",
  "if_statement", "while_statement", "assignment_statement",
  "variable_as_operand", "variable_name", "function_call", "argument_list",
  "expression", "constant_as_operand", "print_statement", "read_statement",
  "return_statement", "call_statement", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-59)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     -59,     8,   -59,    69,   -59,   -59,   -59,   -59,   -59,   -59,
     -59,    69,   -59,   -59,    26,   -59,    30,    39,   -17,   -59,
      40,     1,    71,   -59,    10,   -59,   -59,   -59,   -59,    41,
     -18,   -59,    74,   -59,   -59,    42,     2,   -59,    48,    56,
     -59,   -59,    54,   -59,   -59,    69,   -59,   -59,    71,   -59,
      69,    69,    14,   -59,   -59,    59,   151,    95,   126,    73,
      82,   109,   -59,   -59,   -59,   -59,   -59,   -59,   -59,   114,
     -59,    72,   -59,   -59,   -59,   -59,    91,   135,   -59,   -59,
     -59,   151,   151,   151,   -59,   -59,   170,   -59,   -59,    81,
     -59,   191,   151,   151,   111,   100,   151,   -59,   -59,   -59,
       5,   328,   358,   -59,   233,   151,   151,   151,   151,   151,
     151,   151,   151,   151,   151,   151,   151,   151,   -59,   -59,
     -59,   253,   273,    92,   -59,   212,   151,   -59,   -59,    55,
      55,    55,    55,    55,    55,   358,   343,   313,    24,    24,
     -59,   -59,   109,   109,   151,   -59,   328,   151,   121,   -59,
     293,   328,   109,    99,   -59,   -59
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       5,     0,     2,     9,     1,    27,    28,    29,    30,    31,
       4,     3,     7,     6,     0,     8,     0,    26,     0,    25,
       0,     0,     0,    23,     0,    13,    14,    15,    16,     0,
       0,    11,     0,    26,    24,     0,     0,    18,     0,     0,
      12,    19,     0,    10,    17,    33,    21,    35,     0,    37,
      32,    33,     0,    34,    37,    53,     0,     0,     0,     0,
       0,     0,    37,    20,    36,    45,    43,    44,    38,     0,
      52,     0,    39,    40,    41,    42,     0,     0,    77,    78,
      79,     0,     0,     0,    75,    74,     0,    76,    53,     0,
      82,     0,     0,     0,     0,     0,     0,    84,    22,    54,
       0,    57,    63,    62,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    80,    81,
      83,     0,     0,     0,    46,     0,     0,    55,    73,    64,
      65,    66,    67,    68,    69,    70,    71,     0,    58,    59,
      60,    61,     0,     0,     0,    51,    56,     0,    47,    49,
       0,    72,     0,     0,    48,    50
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -59,   -59,   -59,   -59,   -59,   -59,   107,   102,   -59,   -59,
     -59,   -59,   -59,    -2,   -59,   125,     7,    97,   -59,   -37,
     -58,   -59,   -59,   -59,   -59,   -52,    94,   -50,   -59,   -46,
     -59,   -59,   -59,   -59,   -59
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     1,     2,     3,    10,    11,    30,    31,    32,    12,
      15,    45,    51,    47,    18,    19,    48,    49,    50,    52,
      64,    65,    66,    67,    68,    84,    70,    85,   100,    86,
      87,    72,    73,    74,    75
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      69,    13,    71,    94,    25,    26,    27,    28,     4,    69,
      14,    71,    91,    25,    26,    27,    28,    76,    16,    38,
      22,    39,    55,    23,    69,    95,    71,    56,    57,    58,
      59,   101,    60,    61,    17,   102,   103,   104,    20,    38,
      29,    42,   126,    69,   127,    71,   121,   122,    53,    35,
     125,    25,    26,    27,    28,    62,    63,   116,   117,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,     5,     6,     7,     8,     9,    21,    24,    33,
     146,    37,    40,    41,   148,   149,   114,   115,   116,   117,
      69,    69,    71,    71,   154,    46,    44,    77,   150,    55,
      69,   151,    71,    88,    56,    57,    58,    59,    55,    60,
      61,    92,    97,    56,    57,    58,    59,    55,    60,    61,
      93,   119,    56,    57,    58,    59,    96,    60,    61,   123,
     144,    36,    62,    98,    55,    78,    79,    80,   152,   155,
      43,    62,   124,    55,    78,    79,    80,    34,    54,     0,
      62,    89,     0,     0,    81,     0,     0,     0,    82,    55,
      78,    79,    80,    81,    83,     0,    90,    82,     0,     0,
       0,     0,     0,    83,    99,     0,     0,     0,     0,    81,
       0,     0,     0,    82,     0,     0,     0,     0,     0,    83,
     105,   106,   107,   108,   109,   110,   111,   112,     0,   113,
       0,   114,   115,   116,   117,     0,     0,     0,     0,     0,
     118,   105,   106,   107,   108,   109,   110,   111,   112,     0,
     113,     0,   114,   115,   116,   117,     0,     0,     0,     0,
       0,   120,   105,   106,   107,   108,   109,   110,   111,   112,
       0,   113,     0,   114,   115,   116,   117,     0,     0,     0,
       0,     0,   145,   105,   106,   107,   108,   109,   110,   111,
     112,     0,   113,     0,   114,   115,   116,   117,     0,     0,
       0,     0,   128,   105,   106,   107,   108,   109,   110,   111,
     112,     0,   113,     0,   114,   115,   116,   117,     0,     0,
       0,     0,   142,   105,   106,   107,   108,   109,   110,   111,
     112,     0,   113,     0,   114,   115,   116,   117,     0,     0,
       0,     0,   143,   105,   106,   107,   108,   109,   110,   111,
     112,     0,   113,     0,   114,   115,   116,   117,     0,     0,
       0,     0,   153,   105,   106,   107,   108,   109,   110,   111,
     112,     0,   113,   147,   114,   115,   116,   117,   105,   106,
     107,   108,   109,   110,   111,   112,     0,   113,     0,   114,
     115,   116,   117,   105,   106,   107,   108,   109,   110,   111,
       0,     0,     0,     0,   114,   115,   116,   117,   105,   106,
     107,   108,   109,   110,     0,     0,     0,     0,     0,   114,
     115,   116,   117
};

static const yytype_int16 yycheck[] =
{
      52,     3,    52,    61,     3,     4,     5,     6,     0,    61,
       3,    61,    58,     3,     4,     5,     6,    54,    11,    37,
      37,    39,     8,    40,    76,    62,    76,    13,    14,    15,
      16,    77,    18,    19,     8,    81,    82,    83,     8,    37,
      39,    39,    37,    95,    39,    95,    92,    93,    50,    39,
      96,     3,     4,     5,     6,    41,    42,    33,    34,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,     3,     4,     5,     6,     7,    38,    38,     8,
     126,    40,     8,    41,   142,   143,    31,    32,    33,    34,
     142,   143,   142,   143,   152,    41,    40,    38,   144,     8,
     152,   147,   152,     8,    13,    14,    15,    16,     8,    18,
      19,    38,    40,    13,    14,    15,    16,     8,    18,    19,
      38,    40,    13,    14,    15,    16,    12,    18,    19,    18,
      38,    24,    41,    42,     8,     9,    10,    11,    17,    40,
      38,    41,    42,     8,     9,    10,    11,    22,    51,    -1,
      41,    57,    -1,    -1,    28,    -1,    -1,    -1,    32,     8,
       9,    10,    11,    28,    38,    -1,    40,    32,    -1,    -1,
      -1,    -1,    -1,    38,    39,    -1,    -1,    -1,    -1,    28,
      -1,    -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    38,
      20,    21,    22,    23,    24,    25,    26,    27,    -1,    29,
      -1,    31,    32,    33,    34,    -1,    -1,    -1,    -1,    -1,
      40,    20,    21,    22,    23,    24,    25,    26,    27,    -1,
      29,    -1,    31,    32,    33,    34,    -1,    -1,    -1,    -1,
      -1,    40,    20,    21,    22,    23,    24,    25,    26,    27,
      -1,    29,    -1,    31,    32,    33,    34,    -1,    -1,    -1,
      -1,    -1,    40,    20,    21,    22,    23,    24,    25,    26,
      27,    -1,    29,    -1,    31,    32,    33,    34,    -1,    -1,
      -1,    -1,    39,    20,    21,    22,    23,    24,    25,    26,
      27,    -1,    29,    -1,    31,    32,    33,    34,    -1,    -1,
      -1,    -1,    39,    20,    21,    22,    23,    24,    25,    26,
      27,    -1,    29,    -1,    31,    32,    33,    34,    -1,    -1,
      -1,    -1,    39,    20,    21,    22,    23,    24,    25,    26,
      27,    -1,    29,    -1,    31,    32,    33,    34,    -1,    -1,
      -1,    -1,    39,    20,    21,    22,    23,    24,    25,    26,
      27,    -1,    29,    30,    31,    32,    33,    34,    20,    21,
      22,    23,    24,    25,    26,    27,    -1,    29,    -1,    31,
      32,    33,    34,    20,    21,    22,    23,    24,    25,    26,
      -1,    -1,    -1,    -1,    31,    32,    33,    34,    20,    21,
      22,    23,    24,    25,    -1,    -1,    -1,    -1,    -1,    31,
      32,    33,    34
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    44,    45,    46,     0,     3,     4,     5,     6,     7,
      47,    48,    52,    56,    59,    53,    59,     8,    57,    58,
       8,    38,    37,    40,    38,     3,     4,     5,     6,    39,
      49,    50,    51,     8,    58,    39,    49,    40,    37,    39,
       8,    41,    39,    50,    40,    54,    41,    56,    59,    60,
      61,    55,    62,    56,    60,     8,    13,    14,    15,    16,
      18,    19,    41,    42,    63,    64,    65,    66,    67,    68,
      69,    70,    74,    75,    76,    77,    62,    38,     9,    10,
      11,    28,    32,    38,    68,    70,    72,    73,     8,    69,
      40,    72,    38,    38,    63,    62,    12,    40,    42,    39,
      71,    72,    72,    72,    72,    20,    21,    22,    23,    24,
      25,    26,    27,    29,    31,    32,    33,    34,    40,    40,
      40,    72,    72,    18,    42,    72,    37,    39,    39,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    72,
      72,    72,    39,    39,    38,    40,    72,    30,    63,    63,
      72,    72,    17,    39,    63,    40
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    43,    44,    45,    46,    46,    47,    47,    48,    48,
      49,    49,    50,    51,    51,    51,    51,    52,    52,    54,
      53,    55,    53,    56,    57,    57,    58,    59,    59,    59,
      59,    59,    60,    60,    61,    61,    62,    62,    63,    63,
      63,    63,    63,    63,    63,    63,    64,    65,    65,    66,
      66,    67,    68,    69,    70,    70,    71,    71,    72,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    72,
      72,    72,    72,    72,    72,    72,    72,    73,    73,    73,
      74,    75,    76,    76,    77
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     2,     2,     0,     1,     1,     2,     0,
       3,     1,     2,     1,     1,     1,     1,     6,     5,     0,
       9,     0,    10,     3,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     0,     2,     1,     2,     0,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     5,     7,     5,
       7,     4,     1,     1,     3,     4,     3,     1,     3,     3,
       3,     3,     2,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     5,     3,     1,     1,     1,     1,     1,     1,
       3,     3,     2,     3,     2
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
  case 2: /* program: external_decl_list  */
#line 264 "parser.y"
            {
            root = (yyvsp[0].ast);
                    if (!sa_parse && !main_defined) {
                            yyerror("main function must be defined");
                            YYERROR;
                    }
            }
#line 1496 "parser.tab.c"
    break;

  case 3: /* external_decl_list: external_decl_section function_def_section  */
#line 275 "parser.y"
            {
            Ast_List *cur;

            (yyval.ast) = (yyvsp[-1].ast);
            cur = ((Program_Ast *)(yyvsp[0].ast))->procedures;
            while (cur) {
                merge_external_node((Program_Ast *)(yyval.ast), cur->stmt);
                cur = cur->next;
            }
            }
#line 1511 "parser.tab.c"
    break;

  case 4: /* external_decl_section: external_decl_section external_decl_decl  */
#line 289 "parser.y"
            {
            (yyval.ast) = (yyvsp[-1].ast);
            if ((yyvsp[0].ast)) {
                merge_external_node((Program_Ast *)(yyval.ast), (yyvsp[0].ast));
            }
            }
#line 1522 "parser.tab.c"
    break;

  case 5: /* external_decl_section: %empty  */
#line 296 "parser.y"
            {
            (yyval.ast) = make_program_ast(yylineno);
            }
#line 1530 "parser.tab.c"
    break;

  case 6: /* external_decl_decl: var_decl_stmt  */
#line 303 "parser.y"
            {
                    (yyval.ast) = NULL;
            }
#line 1538 "parser.tab.c"
    break;

  case 7: /* external_decl_decl: function_decl  */
#line 307 "parser.y"
            {
            (yyval.ast) = (yyvsp[0].ast);
            }
#line 1546 "parser.tab.c"
    break;

  case 8: /* function_def_section: function_def_section function_def  */
#line 314 "parser.y"
            {
            (yyval.ast) = (yyvsp[-1].ast);
            if ((yyvsp[0].ast)) {
                merge_external_node((Program_Ast *)(yyval.ast), (yyvsp[0].ast));
            }
            }
#line 1557 "parser.tab.c"
    break;

  case 9: /* function_def_section: %empty  */
#line 321 "parser.y"
            {
            (yyval.ast) = make_program_ast(yylineno);
            }
#line 1565 "parser.tab.c"
    break;

  case 10: /* formal_param_list: formal_param_list ',' formal_param  */
#line 339 "parser.y"
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
#line 1587 "parser.tab.c"
    break;

  case 11: /* formal_param_list: formal_param  */
#line 357 "parser.y"
      {
          Ast_List *new_node = malloc(sizeof(Ast_List));                    
          new_node->stmt = (yyvsp[0].ast);
          new_node->next = NULL;
          (yyval.list) = new_node;
      }
#line 1598 "parser.tab.c"
    break;

  case 12: /* formal_param: param_type IDENTIFIER  */
#line 367 "parser.y"
      {   set_scope(LOCAL_SCOPE);
          Symbol_Table_Entry *entry = insert_symbol((yyvsp[0].token).lexeme, (yyvsp[-1].dtype));                    
          (yyval.ast) = make_name_ast(entry, (yyvsp[0].token).line);
          set_scope(GLOBAL_SCOPE);
      }
#line 1608 "parser.tab.c"
    break;

  case 13: /* param_type: INT  */
#line 376 "parser.y"
                { (yyval.dtype) = INT_TYPE; }
#line 1614 "parser.tab.c"
    break;

  case 14: /* param_type: FLOAT  */
#line 377 "parser.y"
                { (yyval.dtype) = FLOAT_TYPE; }
#line 1620 "parser.tab.c"
    break;

  case 15: /* param_type: BOOL  */
#line 378 "parser.y"
                { (yyval.dtype) = BOOL_TYPE; }
#line 1626 "parser.tab.c"
    break;

  case 16: /* param_type: STRING  */
#line 379 "parser.y"
                { (yyval.dtype) = STRING_TYPE; }
#line 1632 "parser.tab.c"
    break;

  case 17: /* function_decl: named_type IDENTIFIER '(' formal_param_list ')' ';'  */
#line 383 "parser.y"
      {
          current_func_return_type = current_decl_type;
          if (!register_function((yyvsp[-4].token).lexeme, current_func_return_type, (yyvsp[-2].list), 0, (yyvsp[-4].token).line)) {
              YYERROR;
          }
          (yyval.ast) = make_procedure_ast((yyvsp[-4].token).lexeme, current_func_return_type, (yyvsp[-2].list), 0, 0, NULL, (yyvsp[-4].token).line);
          clear_local_scope();
      }
#line 1645 "parser.tab.c"
    break;

  case 18: /* function_decl: named_type IDENTIFIER '(' ')' ';'  */
#line 392 "parser.y"
      {
          current_func_return_type = current_decl_type;
          if (!register_function((yyvsp[-3].token).lexeme, current_func_return_type, NULL, 0, (yyvsp[-3].token).line)) {
              YYERROR;
          }
          (yyval.ast) = make_procedure_ast((yyvsp[-3].token).lexeme, current_func_return_type, NULL, 0, 0, NULL, (yyvsp[-3].token).line);
          clear_local_scope();
      }
#line 1658 "parser.tab.c"
    break;

  case 19: /* $@1: %empty  */
#line 404 "parser.y"
      {
          current_func_return_type = current_decl_type;
          if (!register_function((yyvsp[-3].token).lexeme, current_func_return_type, NULL, 1, (yyvsp[-3].token).line)) {
              YYERROR;
          }
          set_scope(LOCAL_SCOPE);
          collecting_function_locals = 1;
          current_function_local_bytes = 0;
          current_function_locals = NULL;
      }
#line 1673 "parser.tab.c"
    break;

  case 20: /* function_def: named_type IDENTIFIER '(' ')' '{' $@1 optional_local_var_decl_stmt_list statement_list '}'  */
#line 417 "parser.y"
      {
          Ast *body = (yyvsp[-1].ast) ? (yyvsp[-1].ast) : make_sequence_ast((yyvsp[-7].token).line);
          Ast *proc = make_procedure_ast((yyvsp[-7].token).lexeme, current_func_return_type, NULL, current_function_local_bytes, 1, body, (yyvsp[-7].token).line);
          ((Procedure_Ast *)proc)->locals = current_function_locals;
          (yyval.ast) = proc;
          collecting_function_locals = 0;
          current_function_local_bytes = 0;
          current_function_locals = NULL;
          set_scope(GLOBAL_SCOPE);
          clear_local_scope();
      }
#line 1689 "parser.tab.c"
    break;

  case 21: /* $@2: %empty  */
#line 429 "parser.y"
      {
          current_func_return_type = current_decl_type;
          if (!register_function((yyvsp[-4].token).lexeme, current_func_return_type, (yyvsp[-2].list), 1, (yyvsp[-4].token).line)) {
              YYERROR;
          }
          set_scope(LOCAL_SCOPE);
          collecting_function_locals = 1;
          current_function_local_bytes = 0;
          current_function_locals = NULL;
      }
#line 1704 "parser.tab.c"
    break;

  case 22: /* function_def: named_type IDENTIFIER '(' formal_param_list ')' '{' $@2 optional_local_var_decl_stmt_list statement_list '}'  */
#line 442 "parser.y"
      {
          Ast *body = (yyvsp[-1].ast) ? (yyvsp[-1].ast) : make_sequence_ast((yyvsp[-8].token).line);
          Ast *proc = make_procedure_ast((yyvsp[-8].token).lexeme, current_func_return_type, (yyvsp[-6].list), current_function_local_bytes, 1, body, (yyvsp[-8].token).line);
          ((Procedure_Ast *)proc)->locals = current_function_locals;
          (yyval.ast) = proc;
          collecting_function_locals = 0;
          current_function_local_bytes = 0;
          current_function_locals = NULL;
          set_scope(GLOBAL_SCOPE);
          clear_local_scope();
      }
#line 1720 "parser.tab.c"
    break;

  case 26: /* var_decl_item: IDENTIFIER  */
#line 471 "parser.y"
      {     
          Symbol_Table_Entry *entry = insert_symbol((yyvsp[0].token).lexeme, current_decl_type);
          if (collecting_function_locals)
          {
              current_function_local_bytes += type_size_bytes(current_decl_type);
              if (entry)
                  append_ast_list_node(&current_function_locals, make_name_ast(entry, (yyvsp[0].token).line));
          }
      }
#line 1734 "parser.tab.c"
    break;

  case 27: /* named_type: INT  */
#line 483 "parser.y"
             { current_decl_type = INT_TYPE; }
#line 1740 "parser.tab.c"
    break;

  case 28: /* named_type: FLOAT  */
#line 484 "parser.y"
             { current_decl_type = FLOAT_TYPE; }
#line 1746 "parser.tab.c"
    break;

  case 29: /* named_type: BOOL  */
#line 485 "parser.y"
             { current_decl_type = BOOL_TYPE; }
#line 1752 "parser.tab.c"
    break;

  case 30: /* named_type: STRING  */
#line 486 "parser.y"
             { current_decl_type = STRING_TYPE; }
#line 1758 "parser.tab.c"
    break;

  case 31: /* named_type: VOID  */
#line 487 "parser.y"
             { current_decl_type = VOID_TYPE; }
#line 1764 "parser.tab.c"
    break;

  case 36: /* statement_list: statement_list statement  */
#line 502 "parser.y"
      {
          if ((yyvsp[-1].ast) == NULL) {
              (yyval.ast) = make_sequence_ast(yylineno);
          } else {
              (yyval.ast) = (yyvsp[-1].ast);
          }
          sequence_append((Sequence_Ast *)(yyval.ast), (yyvsp[0].ast));
      }
#line 1777 "parser.tab.c"
    break;

  case 37: /* statement_list: %empty  */
#line 511 "parser.y"
      {
          (yyval.ast) = NULL;
      }
#line 1785 "parser.tab.c"
    break;

  case 38: /* statement: assignment_statement  */
#line 517 "parser.y"
                            { (yyval.ast) = (yyvsp[0].ast); }
#line 1791 "parser.tab.c"
    break;

  case 39: /* statement: print_statement  */
#line 518 "parser.y"
                            { (yyval.ast) = (yyvsp[0].ast); }
#line 1797 "parser.tab.c"
    break;

  case 40: /* statement: read_statement  */
#line 519 "parser.y"
                            { (yyval.ast) = (yyvsp[0].ast); }
#line 1803 "parser.tab.c"
    break;

  case 41: /* statement: return_statement  */
#line 520 "parser.y"
                            { (yyval.ast) = (yyvsp[0].ast); }
#line 1809 "parser.tab.c"
    break;

  case 42: /* statement: call_statement  */
#line 521 "parser.y"
                            { (yyval.ast) = (yyvsp[0].ast); }
#line 1815 "parser.tab.c"
    break;

  case 43: /* statement: if_statement  */
#line 522 "parser.y"
                            { (yyval.ast) = (yyvsp[0].ast); }
#line 1821 "parser.tab.c"
    break;

  case 44: /* statement: while_statement  */
#line 523 "parser.y"
                            { (yyval.ast) = (yyvsp[0].ast); }
#line 1827 "parser.tab.c"
    break;

  case 45: /* statement: block_statement  */
#line 524 "parser.y"
                            { (yyval.ast) = (yyvsp[0].ast); }
#line 1833 "parser.tab.c"
    break;

  case 46: /* block_statement: '{' statement_list '}'  */
#line 529 "parser.y"
      {
          if ((yyvsp[-1].ast)) {
              (yyval.ast) = (yyvsp[-1].ast);
          } else {
              (yyval.ast) = make_sequence_ast(yylineno);
          }
      }
#line 1845 "parser.tab.c"
    break;

  case 47: /* if_statement: IF '(' expression ')' statement  */
#line 540 "parser.y"
      {
          (yyval.ast) = make_if_else_stmt_ast((yyvsp[-2].ast), (yyvsp[0].ast), NULL, (yyvsp[-4].token).line);
      }
#line 1853 "parser.tab.c"
    break;

  case 48: /* if_statement: IF '(' expression ')' statement ELSE statement  */
#line 544 "parser.y"
      {
          (yyval.ast) = make_if_else_stmt_ast((yyvsp[-4].ast), (yyvsp[-2].ast), (yyvsp[0].ast), (yyvsp[-6].token).line);
      }
#line 1861 "parser.tab.c"
    break;

  case 49: /* while_statement: WHILE '(' expression ')' statement  */
#line 551 "parser.y"
      {
          (yyval.ast) = make_while_ast((yyvsp[-2].ast), (yyvsp[0].ast), 0, (yyvsp[-4].token).line);
      }
#line 1869 "parser.tab.c"
    break;

  case 50: /* while_statement: DO statement WHILE '(' expression ')' ';'  */
#line 555 "parser.y"
            {
                    (yyval.ast) = make_while_ast((yyvsp[-2].ast), (yyvsp[-5].ast), 1, (yyvsp[-6].token).line);
            }
#line 1877 "parser.tab.c"
    break;

  case 51: /* assignment_statement: variable_as_operand ASSIGNMENT expression ';'  */
#line 562 "parser.y"
      {
          (yyval.ast) = make_assignment_ast((yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[-2].token).line);
      }
#line 1885 "parser.tab.c"
    break;

  case 52: /* variable_as_operand: variable_name  */
#line 569 "parser.y"
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
#line 1900 "parser.tab.c"
    break;

  case 54: /* function_call: IDENTIFIER '(' ')'  */
#line 587 "parser.y"
      {
          if (!sa_parse && !lookup_function((yyvsp[-2].token).lexeme)) {
              char buf[256];
              snprintf(buf, sizeof(buf), "Function '%s' not declared", (yyvsp[-2].token).lexeme);
              yyerror(buf);
              YYERROR;
          }
          (yyval.ast) = make_call_ast((yyvsp[-2].token).lexeme, NULL, (yyvsp[-2].token).line);
      }
#line 1914 "parser.tab.c"
    break;

  case 55: /* function_call: IDENTIFIER '(' argument_list ')'  */
#line 597 "parser.y"
      {
          if (!sa_parse && !lookup_function((yyvsp[-3].token).lexeme)) {
              char buf[256];
              snprintf(buf, sizeof(buf), "Function '%s' not declared", (yyvsp[-3].token).lexeme);
              yyerror(buf);
              YYERROR;
          }
          (yyval.ast) = make_call_ast((yyvsp[-3].token).lexeme, (yyvsp[-1].list), (yyvsp[-3].token).line);
      }
#line 1928 "parser.tab.c"
    break;

  case 56: /* argument_list: argument_list ',' expression  */
#line 610 "parser.y"
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
#line 1948 "parser.tab.c"
    break;

  case 57: /* argument_list: expression  */
#line 626 "parser.y"
      {
          Ast_List *new_node = malloc(sizeof(Ast_List));
          new_node->stmt = (yyvsp[0].ast);
          new_node->next = NULL;
          (yyval.list) = new_node;
      }
#line 1959 "parser.tab.c"
    break;

  case 58: /* expression: expression '+' expression  */
#line 636 "parser.y"
      {
          (yyval.ast) = make_binary_ast(AST_PLUS, (yyvsp[-2].ast), (yyvsp[0].ast), yylineno);
      }
#line 1967 "parser.tab.c"
    break;

  case 59: /* expression: expression '-' expression  */
#line 640 "parser.y"
      {
          (yyval.ast) = make_binary_ast(AST_MINUS, (yyvsp[-2].ast), (yyvsp[0].ast), yylineno);
      }
#line 1975 "parser.tab.c"
    break;

  case 60: /* expression: expression '*' expression  */
#line 644 "parser.y"
      {
          (yyval.ast) = make_binary_ast(AST_MULT, (yyvsp[-2].ast), (yyvsp[0].ast), yylineno);
      }
#line 1983 "parser.tab.c"
    break;

  case 61: /* expression: expression '/' expression  */
#line 648 "parser.y"
      {
          (yyval.ast) = make_binary_ast(AST_DIV, (yyvsp[-2].ast), (yyvsp[0].ast), yylineno);
      }
#line 1991 "parser.tab.c"
    break;

  case 62: /* expression: '-' expression  */
#line 652 "parser.y"
      {
          (yyval.ast) = make_unary_ast(AST_UMINUS, (yyvsp[0].ast), yylineno);
      }
#line 1999 "parser.tab.c"
    break;

  case 63: /* expression: NOT expression  */
#line 656 "parser.y"
      {
          (yyval.ast) = make_unary_ast(AST_NOT, (yyvsp[0].ast), yylineno);
      }
#line 2007 "parser.tab.c"
    break;

  case 64: /* expression: expression EQ expression  */
#line 660 "parser.y"
      {
          (yyval.ast) = make_relational_ast((yyvsp[-2].ast), REL_EQ, (yyvsp[0].ast), yylineno);
      }
#line 2015 "parser.tab.c"
    break;

  case 65: /* expression: expression NE expression  */
#line 664 "parser.y"
      {
          (yyval.ast) = make_relational_ast((yyvsp[-2].ast), REL_NE, (yyvsp[0].ast), yylineno);
      }
#line 2023 "parser.tab.c"
    break;

  case 66: /* expression: expression LT expression  */
#line 668 "parser.y"
      {
          (yyval.ast) = make_relational_ast((yyvsp[-2].ast), REL_LT, (yyvsp[0].ast), yylineno);
      }
#line 2031 "parser.tab.c"
    break;

  case 67: /* expression: expression LE expression  */
#line 672 "parser.y"
      {
          (yyval.ast) = make_relational_ast((yyvsp[-2].ast), REL_LE, (yyvsp[0].ast), yylineno);
      }
#line 2039 "parser.tab.c"
    break;

  case 68: /* expression: expression GT expression  */
#line 676 "parser.y"
      {
          (yyval.ast) = make_relational_ast((yyvsp[-2].ast), REL_GT, (yyvsp[0].ast), yylineno);
      }
#line 2047 "parser.tab.c"
    break;

  case 69: /* expression: expression GE expression  */
#line 680 "parser.y"
      {
          (yyval.ast) = make_relational_ast((yyvsp[-2].ast), REL_GE, (yyvsp[0].ast), yylineno);
      }
#line 2055 "parser.tab.c"
    break;

  case 70: /* expression: expression AND expression  */
#line 684 "parser.y"
      {
          (yyval.ast) = make_logical_ast((yyvsp[-2].ast), LOGICAL_AND, (yyvsp[0].ast), yylineno);
      }
#line 2063 "parser.tab.c"
    break;

  case 71: /* expression: expression OR expression  */
#line 688 "parser.y"
      {
          (yyval.ast) = make_logical_ast((yyvsp[-2].ast), LOGICAL_OR, (yyvsp[0].ast), yylineno);
      }
#line 2071 "parser.tab.c"
    break;

  case 72: /* expression: expression '?' expression ':' expression  */
#line 692 "parser.y"
      {
          (yyval.ast) = make_if_ast((yyvsp[-4].ast), (yyvsp[-2].ast), (yyvsp[0].ast), yylineno);
      }
#line 2079 "parser.tab.c"
    break;

  case 73: /* expression: '(' expression ')'  */
#line 696 "parser.y"
      {
          (yyval.ast) = (yyvsp[-1].ast);
      }
#line 2087 "parser.tab.c"
    break;

  case 74: /* expression: function_call  */
#line 700 "parser.y"
            {
                    (yyval.ast) = (yyvsp[0].ast);
            }
#line 2095 "parser.tab.c"
    break;

  case 75: /* expression: variable_as_operand  */
#line 704 "parser.y"
      {
          (yyval.ast) = (yyvsp[0].ast);
      }
#line 2103 "parser.tab.c"
    break;

  case 76: /* expression: constant_as_operand  */
#line 708 "parser.y"
      {
          (yyval.ast) = (yyvsp[0].ast);
      }
#line 2111 "parser.tab.c"
    break;

  case 77: /* constant_as_operand: INTEGER_NUMBER  */
#line 715 "parser.y"
      {
          (yyval.ast) = make_number_ast((yyvsp[0].token).lexeme, INT_TYPE, (yyvsp[0].token).line);
      }
#line 2119 "parser.tab.c"
    break;

  case 78: /* constant_as_operand: FLOAT_NUMBER  */
#line 719 "parser.y"
      {
          (yyval.ast) = make_number_ast((yyvsp[0].token).lexeme, FLOAT_TYPE, (yyvsp[0].token).line);
      }
#line 2127 "parser.tab.c"
    break;

  case 79: /* constant_as_operand: STRING_CONSTANT  */
#line 723 "parser.y"
      {
          (yyval.ast) = make_number_ast((yyvsp[0].token).lexeme, STRING_TYPE, (yyvsp[0].token).line);
      }
#line 2135 "parser.tab.c"
    break;

  case 80: /* print_statement: PRINT expression ';'  */
#line 730 "parser.y"
      {
          (yyval.ast) = make_print_ast((yyvsp[-1].ast), (yyvsp[-2].token).line);
      }
#line 2143 "parser.tab.c"
    break;

  case 81: /* read_statement: READ variable_name ';'  */
#line 737 "parser.y"
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
#line 2159 "parser.tab.c"
    break;

  case 82: /* return_statement: RETURN ';'  */
#line 752 "parser.y"
            {
                    (yyval.ast) = make_return_ast(NULL, (yyvsp[-1].token).line);
            }
#line 2167 "parser.tab.c"
    break;

  case 83: /* return_statement: RETURN expression ';'  */
#line 756 "parser.y"
            {
                    (yyval.ast) = make_return_ast((yyvsp[-1].ast), (yyvsp[-2].token).line);
            }
#line 2175 "parser.tab.c"
    break;

  case 84: /* call_statement: function_call ';'  */
#line 763 "parser.y"
            {
            Call_Ast *call = (Call_Ast *)(yyvsp[-1].ast);
            if (call) {
                call->used_as_statement = 1;
            }
            (yyval.ast) = (yyvsp[-1].ast);
            }
#line 2187 "parser.tab.c"
    break;


#line 2191 "parser.tab.c"

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

#line 772 "parser.y"


void yyerror(const char *s) {
    printf("Parse error at line %d: %s\n", yylineno, s);
}
