#ifndef lint
static char const 
yyrcsid[] = "$FreeBSD: src/usr.bin/yacc/skeleton.c,v 1.28 2000/01/17 02:04:06 bde Exp $";
#endif
#include <stdlib.h>
#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYLEX yylex()
#define YYEMPTY -1
#define yyclearin (yychar=(YYEMPTY))
#define yyerrok (yyerrflag=0)
#define YYRECOVERING() (yyerrflag!=0)
static int yygrowstack();
#define YYPREFIX "yy"
#line 16 "H5LTparse.y"
#include<stdio.h>
#include<string.h>
#include<hdf5.h>

extern int yylex();
extern int yyerror(char *);

#define STACK_SIZE      16

/*structure for compound type information*/
struct cmpd_info {
    hid_t       id;             /*type ID*/
    hbool_t     is_field;       /*flag to lexer for compound member*/
    hbool_t     first_memb;     /*flag for first compound member*/
};

/*stack for nested compound type*/
struct cmpd_info cmpd_stack[STACK_SIZE] = {
    {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1},
    {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1},
    {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1},
    {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1} };

int csindex = -1;                /*pointer to the top of compound stack*/

/*structure for array type information*/
struct arr_info {
    hsize_t             dims[H5S_MAX_RANK];     /*size of each dimension, limited to 32 dimensions*/
    int                 ndims;                  /*number of dimensions*/
    hbool_t             is_dim;                 /*flag to lexer for dimension*/
};
/*stack for nested array type*/
struct arr_info arr_stack[STACK_SIZE];
int asindex = -1;               /*pointer to the top of array stack*/ 

hbool_t     is_str_size = 0;        /*flag to lexer for string size*/
hbool_t     is_str_pad = 0;         /*flag to lexer for string padding*/
H5T_pad_t   str_pad;                /*variable for string padding*/
H5T_cset_t  str_cset;               /*variable for string character set*/
hbool_t     is_variable = 0;        /*variable for variable-length string*/
size_t      str_size;               /*variable for string size*/
   
hid_t       enum_id;                /*type ID*/
hbool_t     is_enum = 0;            /*flag to lexer for enum type*/
hbool_t     is_enum_memb = 0;       /*flag to lexer for enum member*/
char*       enum_memb_symbol;       /*enum member symbol string*/

hbool_t is_opq_size = 0;            /*flag to lexer for opaque type size*/
hbool_t is_opq_tag = 0;             /*flag to lexer for opaque type tag*/

#line 67 "H5LTparse.y"
typedef union {
    int   ival;         /*for integer token*/
    char  *sval;        /*for name string*/
} YYSTYPE;
#line 73 "H5LTparse.c"
#define YYERRCODE 256
#define H5T_STD_I8BE_TOKEN 257
#define H5T_STD_I8LE_TOKEN 258
#define H5T_STD_I16BE_TOKEN 259
#define H5T_STD_I16LE_TOKEN 260
#define H5T_STD_I32BE_TOKEN 261
#define H5T_STD_I32LE_TOKEN 262
#define H5T_STD_I64BE_TOKEN 263
#define H5T_STD_I64LE_TOKEN 264
#define H5T_STD_U8BE_TOKEN 265
#define H5T_STD_U8LE_TOKEN 266
#define H5T_STD_U16BE_TOKEN 267
#define H5T_STD_U16LE_TOKEN 268
#define H5T_STD_U32BE_TOKEN 269
#define H5T_STD_U32LE_TOKEN 270
#define H5T_STD_U64BE_TOKEN 271
#define H5T_STD_U64LE_TOKEN 272
#define H5T_NATIVE_CHAR_TOKEN 273
#define H5T_NATIVE_SCHAR_TOKEN 274
#define H5T_NATIVE_UCHAR_TOKEN 275
#define H5T_NATIVE_SHORT_TOKEN 276
#define H5T_NATIVE_USHORT_TOKEN 277
#define H5T_NATIVE_INT_TOKEN 278
#define H5T_NATIVE_UINT_TOKEN 279
#define H5T_NATIVE_LONG_TOKEN 280
#define H5T_NATIVE_ULONG_TOKEN 281
#define H5T_NATIVE_LLONG_TOKEN 282
#define H5T_NATIVE_ULLONG_TOKEN 283
#define H5T_IEEE_F32BE_TOKEN 284
#define H5T_IEEE_F32LE_TOKEN 285
#define H5T_IEEE_F64BE_TOKEN 286
#define H5T_IEEE_F64LE_TOKEN 287
#define H5T_NATIVE_FLOAT_TOKEN 288
#define H5T_NATIVE_DOUBLE_TOKEN 289
#define H5T_NATIVE_LDOUBLE_TOKEN 290
#define H5T_STRING_TOKEN 291
#define STRSIZE_TOKEN 292
#define STRPAD_TOKEN 293
#define CSET_TOKEN 294
#define CTYPE_TOKEN 295
#define H5T_VARIABLE_TOKEN 296
#define H5T_STR_NULLTERM_TOKEN 297
#define H5T_STR_NULLPAD_TOKEN 298
#define H5T_STR_SPACEPAD_TOKEN 299
#define H5T_CSET_ASCII_TOKEN 300
#define H5T_CSET_UTF8_TOKEN 301
#define H5T_C_S1_TOKEN 302
#define H5T_FORTRAN_S1_TOKEN 303
#define H5T_OPAQUE_TOKEN 304
#define OPQ_SIZE_TOKEN 305
#define OPQ_TAG_TOKEN 306
#define H5T_COMPOUND_TOKEN 307
#define H5T_ENUM_TOKEN 308
#define H5T_ARRAY_TOKEN 309
#define H5T_VLEN_TOKEN 310
#define STRING 311
#define NUMBER 312
const short yylhs[] = {                                        -1,
    0,    0,    1,    1,    1,    1,    2,    2,    2,    2,
    2,    6,    6,    6,    6,    6,    6,    6,    6,    6,
    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,
    6,    6,    6,    6,    6,    6,    6,    6,    7,    7,
    7,    7,    7,    7,    7,   11,    3,   12,   12,   14,
   13,   15,   16,   16,   17,   18,    4,   19,   19,   22,
   23,   20,   21,    5,   25,   26,   27,   29,   10,   24,
   28,   31,   32,   34,   36,   38,    8,   30,   30,   33,
   33,   33,   35,   35,   37,   37,   40,    9,   39,   39,
   44,   41,   42,   43,
};
const short yylen[] = {                                         2,
    0,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    0,    5,    0,    2,    0,
    7,    1,    0,    2,    1,    0,    6,    0,    2,    0,
    0,    5,    1,    4,    0,    0,    0,    0,   15,    1,
    1,    0,    0,    0,    0,    0,   20,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    0,    7,    0,    2,
    0,    6,    1,    1,
};
const short yydefred[] = {                                      0,
   12,   13,   14,   15,   16,   17,   18,   19,   20,   21,
   22,   23,   24,   25,   26,   27,   28,   29,   30,   31,
   32,   33,   34,   35,   36,   37,   38,   39,   40,   41,
   42,   43,   44,   45,    0,    0,   46,    0,   56,    0,
    0,    2,    3,    4,    5,    6,    7,    8,    9,   10,
   11,    0,    0,    0,    0,    0,    0,   72,   65,   48,
    0,   58,    0,    0,    0,    0,   87,    0,   64,   78,
   79,    0,   70,    0,   47,   50,   49,   89,   60,    0,
   59,   73,   66,    0,    0,    0,   57,    0,    0,    0,
   88,    0,   90,   63,   61,    0,   67,   52,    0,   93,
    0,    0,   80,   81,   82,    0,    0,    0,   91,   62,
   74,    0,    0,    0,    0,    0,   71,    0,   55,   54,
   51,   94,    0,    0,    0,   92,   83,   84,    0,   68,
   75,    0,    0,   69,    0,   85,   86,    0,   76,    0,
   77,
};
const short yydgoto[] = {                                      41,
   42,   43,   44,   45,   46,   47,   48,   49,   50,   51,
   54,   66,   77,   84,   99,  114,  120,   56,   68,   81,
   95,   86,  102,   74,   65,   89,  107,  118,  132,   72,
   64,   88,  106,  116,  129,  133,  138,  140,   85,   78,
   93,  101,  123,  115,
};
const short yysindex[] = {                                   -255,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  -82,  -79,    0,  -78,    0,  -76,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0, -208, -220,  -36, -201,  -34, -255,    0,    0,    0,
   27,    0,  -35, -213, -221,  -37,    0,  -91,    0,    0,
    0,   34,    0,   35,    0,    0,    0,    0,    0,  -30,
    0,    0,    0,   62,  -33, -215,    0, -195, -206, -210,
    0, -209,    0,    0,    0, -259,    0,    0,   69,    0,
   70,   12,    0,    0,    0,   47,   73,   50,    0,    0,
    0, -202, -200,   51, -199, -183,    0,   80,    0,    0,
    0,    0,   56, -258,   57,    0,    0,    0,   58,    0,
    0,   -7, -176,    0, -252,    0,    0,   61,    0,   -4,
    0,
};
const short yyrindex[] = {                                    122,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   64,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,
};
const short yygindex[] = {                                      0,
  -20,    0,    0,    0,    0,   71,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,
};
#define YYTABLESIZE 273
const short yytable[] = {                                      79,
   92,    1,    2,    3,    4,    5,    6,    7,    8,    9,
   10,   11,   12,   13,   14,   15,   16,   17,   18,   19,
   20,   21,   22,   23,   24,   25,   26,   27,   28,   29,
   30,   31,   32,   33,   34,   35,   63,  103,  104,  105,
   52,  127,  128,   53,   55,   76,   57,   80,   36,  136,
  137,   37,   38,   39,   40,    1,    2,    3,    4,    5,
    6,    7,    8,    9,   10,   11,   12,   13,   14,   15,
   16,   17,   18,   19,   20,   21,   22,   23,   24,   25,
   26,   27,   70,   58,   59,   67,   60,   75,   62,   69,
   73,   91,   82,   83,   87,   90,   94,   96,   71,   97,
   98,  100,  108,  109,  110,  111,  112,  113,  117,  121,
  124,  119,  122,  125,  126,  130,  131,  134,  135,  139,
  141,    1,   53,    0,    0,   61,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    1,    2,    3,    4,    5,
    6,    7,    8,    9,   10,   11,   12,   13,   14,   15,
   16,   17,   18,   19,   20,   21,   22,   23,   24,   25,
   26,   27,   28,   29,   30,   31,   32,   33,   34,   35,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   36,    0,    0,   37,   38,   39,   40,    1,
    2,    3,    4,    5,    6,    7,    8,    9,   10,   11,
   12,   13,   14,   15,   16,   17,   18,   19,   20,   21,
   22,   23,   24,   25,   26,   27,   28,   29,   30,   31,
   32,   33,   34,   35,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   36,    0,    0,   37,
   38,   39,   40,
};
const short yycheck[] = {                                      91,
   34,  257,  258,  259,  260,  261,  262,  263,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,  282,  283,  284,  285,
  286,  287,  288,  289,  290,  291,   57,  297,  298,  299,
  123,  300,  301,  123,  123,   66,  123,   68,  304,  302,
  303,  307,  308,  309,  310,  257,  258,  259,  260,  261,
  262,  263,  264,  265,  266,  267,  268,  269,  270,  271,
  272,  273,  274,  275,  276,  277,  278,  279,  280,  281,
  282,  283,  296,  292,  305,   59,  123,  125,  123,  125,
  312,  125,   59,   59,  125,   34,  312,  293,  312,  306,
  311,  311,   34,   34,   93,   59,   34,   58,  311,   59,
  294,  312,  312,   34,   59,   59,   59,  125,  295,   59,
  125,    0,   59,   -1,   -1,   55,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  257,  258,  259,  260,  261,
  262,  263,  264,  265,  266,  267,  268,  269,  270,  271,
  272,  273,  274,  275,  276,  277,  278,  279,  280,  281,
  282,  283,  284,  285,  286,  287,  288,  289,  290,  291,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  304,   -1,   -1,  307,  308,  309,  310,  257,
  258,  259,  260,  261,  262,  263,  264,  265,  266,  267,
  268,  269,  270,  271,  272,  273,  274,  275,  276,  277,
  278,  279,  280,  281,  282,  283,  284,  285,  286,  287,
  288,  289,  290,  291,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  304,   -1,   -1,  307,
  308,  309,  310,
};
#define YYFINAL 41
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 312
#if YYDEBUG
const char * const yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
"'\"'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"':'","';'",0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"'['",0,"']'",0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"'{'",0,"'}'",0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
"H5T_STD_I8BE_TOKEN","H5T_STD_I8LE_TOKEN","H5T_STD_I16BE_TOKEN",
"H5T_STD_I16LE_TOKEN","H5T_STD_I32BE_TOKEN","H5T_STD_I32LE_TOKEN",
"H5T_STD_I64BE_TOKEN","H5T_STD_I64LE_TOKEN","H5T_STD_U8BE_TOKEN",
"H5T_STD_U8LE_TOKEN","H5T_STD_U16BE_TOKEN","H5T_STD_U16LE_TOKEN",
"H5T_STD_U32BE_TOKEN","H5T_STD_U32LE_TOKEN","H5T_STD_U64BE_TOKEN",
"H5T_STD_U64LE_TOKEN","H5T_NATIVE_CHAR_TOKEN","H5T_NATIVE_SCHAR_TOKEN",
"H5T_NATIVE_UCHAR_TOKEN","H5T_NATIVE_SHORT_TOKEN","H5T_NATIVE_USHORT_TOKEN",
"H5T_NATIVE_INT_TOKEN","H5T_NATIVE_UINT_TOKEN","H5T_NATIVE_LONG_TOKEN",
"H5T_NATIVE_ULONG_TOKEN","H5T_NATIVE_LLONG_TOKEN","H5T_NATIVE_ULLONG_TOKEN",
"H5T_IEEE_F32BE_TOKEN","H5T_IEEE_F32LE_TOKEN","H5T_IEEE_F64BE_TOKEN",
"H5T_IEEE_F64LE_TOKEN","H5T_NATIVE_FLOAT_TOKEN","H5T_NATIVE_DOUBLE_TOKEN",
"H5T_NATIVE_LDOUBLE_TOKEN","H5T_STRING_TOKEN","STRSIZE_TOKEN","STRPAD_TOKEN",
"CSET_TOKEN","CTYPE_TOKEN","H5T_VARIABLE_TOKEN","H5T_STR_NULLTERM_TOKEN",
"H5T_STR_NULLPAD_TOKEN","H5T_STR_SPACEPAD_TOKEN","H5T_CSET_ASCII_TOKEN",
"H5T_CSET_UTF8_TOKEN","H5T_C_S1_TOKEN","H5T_FORTRAN_S1_TOKEN",
"H5T_OPAQUE_TOKEN","OPQ_SIZE_TOKEN","OPQ_TAG_TOKEN","H5T_COMPOUND_TOKEN",
"H5T_ENUM_TOKEN","H5T_ARRAY_TOKEN","H5T_VLEN_TOKEN","STRING","NUMBER",
};
const char * const yyrule[] = {
"$accept : start",
"start :",
"start : ddl_type",
"ddl_type : atomic_type",
"ddl_type : compound_type",
"ddl_type : array_type",
"ddl_type : vlen_type",
"atomic_type : integer_type",
"atomic_type : fp_type",
"atomic_type : string_type",
"atomic_type : enum_type",
"atomic_type : opaque_type",
"integer_type : H5T_STD_I8BE_TOKEN",
"integer_type : H5T_STD_I8LE_TOKEN",
"integer_type : H5T_STD_I16BE_TOKEN",
"integer_type : H5T_STD_I16LE_TOKEN",
"integer_type : H5T_STD_I32BE_TOKEN",
"integer_type : H5T_STD_I32LE_TOKEN",
"integer_type : H5T_STD_I64BE_TOKEN",
"integer_type : H5T_STD_I64LE_TOKEN",
"integer_type : H5T_STD_U8BE_TOKEN",
"integer_type : H5T_STD_U8LE_TOKEN",
"integer_type : H5T_STD_U16BE_TOKEN",
"integer_type : H5T_STD_U16LE_TOKEN",
"integer_type : H5T_STD_U32BE_TOKEN",
"integer_type : H5T_STD_U32LE_TOKEN",
"integer_type : H5T_STD_U64BE_TOKEN",
"integer_type : H5T_STD_U64LE_TOKEN",
"integer_type : H5T_NATIVE_CHAR_TOKEN",
"integer_type : H5T_NATIVE_SCHAR_TOKEN",
"integer_type : H5T_NATIVE_UCHAR_TOKEN",
"integer_type : H5T_NATIVE_SHORT_TOKEN",
"integer_type : H5T_NATIVE_USHORT_TOKEN",
"integer_type : H5T_NATIVE_INT_TOKEN",
"integer_type : H5T_NATIVE_UINT_TOKEN",
"integer_type : H5T_NATIVE_LONG_TOKEN",
"integer_type : H5T_NATIVE_ULONG_TOKEN",
"integer_type : H5T_NATIVE_LLONG_TOKEN",
"integer_type : H5T_NATIVE_ULLONG_TOKEN",
"fp_type : H5T_IEEE_F32BE_TOKEN",
"fp_type : H5T_IEEE_F32LE_TOKEN",
"fp_type : H5T_IEEE_F64BE_TOKEN",
"fp_type : H5T_IEEE_F64LE_TOKEN",
"fp_type : H5T_NATIVE_FLOAT_TOKEN",
"fp_type : H5T_NATIVE_DOUBLE_TOKEN",
"fp_type : H5T_NATIVE_LDOUBLE_TOKEN",
"$$1 :",
"compound_type : H5T_COMPOUND_TOKEN $$1 '{' memb_list '}'",
"memb_list :",
"memb_list : memb_list memb_def",
"$$2 :",
"memb_def : ddl_type $$2 '\"' field_name '\"' field_offset ';'",
"field_name : STRING",
"field_offset :",
"field_offset : ':' offset",
"offset : NUMBER",
"$$3 :",
"array_type : H5T_ARRAY_TOKEN $$3 '{' dim_list ddl_type '}'",
"dim_list :",
"dim_list : dim_list dim",
"$$4 :",
"$$5 :",
"dim : '[' $$4 dimsize $$5 ']'",
"dimsize : NUMBER",
"vlen_type : H5T_VLEN_TOKEN '{' ddl_type '}'",
"$$6 :",
"$$7 :",
"$$8 :",
"$$9 :",
"opaque_type : H5T_OPAQUE_TOKEN '{' OPQ_SIZE_TOKEN $$6 opaque_size ';' $$7 OPQ_TAG_TOKEN $$8 '\"' opaque_tag '\"' ';' $$9 '}'",
"opaque_size : NUMBER",
"opaque_tag : STRING",
"$$10 :",
"$$11 :",
"$$12 :",
"$$13 :",
"$$14 :",
"string_type : H5T_STRING_TOKEN '{' STRSIZE_TOKEN $$10 strsize ';' $$11 STRPAD_TOKEN strpad ';' $$12 CSET_TOKEN cset ';' $$13 CTYPE_TOKEN ctype ';' $$14 '}'",
"strsize : H5T_VARIABLE_TOKEN",
"strsize : NUMBER",
"strpad : H5T_STR_NULLTERM_TOKEN",
"strpad : H5T_STR_NULLPAD_TOKEN",
"strpad : H5T_STR_SPACEPAD_TOKEN",
"cset : H5T_CSET_ASCII_TOKEN",
"cset : H5T_CSET_UTF8_TOKEN",
"ctype : H5T_C_S1_TOKEN",
"ctype : H5T_FORTRAN_S1_TOKEN",
"$$15 :",
"enum_type : H5T_ENUM_TOKEN '{' integer_type ';' $$15 enum_list '}'",
"enum_list :",
"enum_list : enum_list enum_def",
"$$16 :",
"enum_def : '\"' enum_symbol '\"' $$16 enum_val ';'",
"enum_symbol : STRING",
"enum_val : NUMBER",
};
#endif
#if YYDEBUG
#include <stdio.h>
#endif
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 10000
#define YYMAXDEPTH 10000
#endif
#endif
#define YYINITSTACKSIZE 200
int yydebug;
int yynerrs;
int yyerrflag;
int yychar;
short *yyssp;
YYSTYPE *yyvsp;
YYSTYPE yyval;
YYSTYPE yylval;
short *yyss;
short *yysslim;
YYSTYPE *yyvs;
int yystacksize;
/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack()
{
    int newsize, i;
    short *newss;
    YYSTYPE *newvs;

    if ((newsize = yystacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return -1;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;
    i = yyssp - yyss;
    newss = yyss ? (short *)realloc(yyss, newsize * sizeof *newss) :
      (short *)malloc(newsize * sizeof *newss);
    if (newss == NULL)
        return -1;
    yyss = newss;
    yyssp = newss + i;
    newvs = yyvs ? (YYSTYPE *)realloc(yyvs, newsize * sizeof *newvs) :
      (YYSTYPE *)malloc(newsize * sizeof *newvs);
    if (newvs == NULL)
        return -1;
    yyvs = newvs;
    yyvsp = newvs + i;
    yystacksize = newsize;
    yysslim = yyss + newsize - 1;
    return 0;
}

#define YYABORT goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab

#ifndef YYPARSE_PARAM
#if defined(__cplusplus) || __STDC__
#define YYPARSE_PARAM_ARG void
#define YYPARSE_PARAM_DECL
#else	/* ! ANSI-C/C++ */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif	/* ANSI-C/C++ */
#else	/* YYPARSE_PARAM */
#ifndef YYPARSE_PARAM_TYPE
#define YYPARSE_PARAM_TYPE void *
#endif
#if defined(__cplusplus) || __STDC__
#define YYPARSE_PARAM_ARG YYPARSE_PARAM_TYPE YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else	/* ! ANSI-C/C++ */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL YYPARSE_PARAM_TYPE YYPARSE_PARAM;
#endif	/* ANSI-C/C++ */
#endif	/* ! YYPARSE_PARAM */

int
yyparse (YYPARSE_PARAM_ARG)
    YYPARSE_PARAM_DECL
{
    register int yym, yyn, yystate;
#if YYDEBUG
    register const char *yys;

    if ((yys = getenv("YYDEBUG")))
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = (-1);

    if (yyss == NULL && yygrowstack()) goto yyoverflow;
    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if ((yyn = yydefred[yystate])) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yyssp >= yysslim && yygrowstack())
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
        yychar = (-1);
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;
#if defined(lint) || defined(__GNUC__)
    goto yynewerror;
#endif
yynewerror:
    yyerror("syntax error");
#if defined(lint) || defined(__GNUC__)
    goto yyerrlab;
#endif
yyerrlab:
    ++yynerrs;
yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yysslim && yygrowstack())
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = (-1);
        goto yyloop;
    }
yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    yyval = yyvsp[1-yym];
    switch (yyn)
    {
case 1:
#line 99 "H5LTparse.y"
{ memset(arr_stack, 0, STACK_SIZE*sizeof(struct arr_info)); /*initialize here?*/ }
break;
case 2:
#line 100 "H5LTparse.y"
{ return yyval.ival;}
break;
case 12:
#line 114 "H5LTparse.y"
{ yyval.ival = H5Tcopy(H5T_STD_I8BE); }
break;
case 13:
#line 115 "H5LTparse.y"
{ yyval.ival = H5Tcopy(H5T_STD_I8LE); }
break;
case 14:
#line 116 "H5LTparse.y"
{ yyval.ival = H5Tcopy(H5T_STD_I16BE); }
break;
case 15:
#line 117 "H5LTparse.y"
{ yyval.ival = H5Tcopy(H5T_STD_I16LE); }
break;
case 16:
#line 118 "H5LTparse.y"
{ yyval.ival = H5Tcopy(H5T_STD_I32BE); }
break;
case 17:
#line 119 "H5LTparse.y"
{ yyval.ival = H5Tcopy(H5T_STD_I32LE); }
break;
case 18:
#line 120 "H5LTparse.y"
{ yyval.ival = H5Tcopy(H5T_STD_I64BE); }
break;
case 19:
#line 121 "H5LTparse.y"
{ yyval.ival = H5Tcopy(H5T_STD_I64LE); }
break;
case 20:
#line 122 "H5LTparse.y"
{ yyval.ival = H5Tcopy(H5T_STD_U8BE); }
break;
case 21:
#line 123 "H5LTparse.y"
{ yyval.ival = H5Tcopy(H5T_STD_U8LE); }
break;
case 22:
#line 124 "H5LTparse.y"
{ yyval.ival = H5Tcopy(H5T_STD_U16BE); }
break;
case 23:
#line 125 "H5LTparse.y"
{ yyval.ival = H5Tcopy(H5T_STD_U16LE); }
break;
case 24:
#line 126 "H5LTparse.y"
{ yyval.ival = H5Tcopy(H5T_STD_U32BE); }
break;
case 25:
#line 127 "H5LTparse.y"
{ yyval.ival = H5Tcopy(H5T_STD_U32LE); }
break;
case 26:
#line 128 "H5LTparse.y"
{ yyval.ival = H5Tcopy(H5T_STD_U64BE); }
break;
case 27:
#line 129 "H5LTparse.y"
{ yyval.ival = H5Tcopy(H5T_STD_U64LE); }
break;
case 28:
#line 130 "H5LTparse.y"
{ yyval.ival = H5Tcopy(H5T_NATIVE_CHAR); }
break;
case 29:
#line 131 "H5LTparse.y"
{ yyval.ival = H5Tcopy(H5T_NATIVE_SCHAR); }
break;
case 30:
#line 132 "H5LTparse.y"
{ yyval.ival = H5Tcopy(H5T_NATIVE_UCHAR); }
break;
case 31:
#line 133 "H5LTparse.y"
{ yyval.ival = H5Tcopy(H5T_NATIVE_SHORT); }
break;
case 32:
#line 134 "H5LTparse.y"
{ yyval.ival = H5Tcopy(H5T_NATIVE_USHORT); }
break;
case 33:
#line 135 "H5LTparse.y"
{ yyval.ival = H5Tcopy(H5T_NATIVE_INT); }
break;
case 34:
#line 136 "H5LTparse.y"
{ yyval.ival = H5Tcopy(H5T_NATIVE_UINT); }
break;
case 35:
#line 137 "H5LTparse.y"
{ yyval.ival = H5Tcopy(H5T_NATIVE_LONG); }
break;
case 36:
#line 138 "H5LTparse.y"
{ yyval.ival = H5Tcopy(H5T_NATIVE_ULONG); }
break;
case 37:
#line 139 "H5LTparse.y"
{ yyval.ival = H5Tcopy(H5T_NATIVE_LLONG); }
break;
case 38:
#line 140 "H5LTparse.y"
{ yyval.ival = H5Tcopy(H5T_NATIVE_ULLONG); }
break;
case 39:
#line 143 "H5LTparse.y"
{ yyval.ival = H5Tcopy(H5T_IEEE_F32BE); }
break;
case 40:
#line 144 "H5LTparse.y"
{ yyval.ival = H5Tcopy(H5T_IEEE_F32LE); }
break;
case 41:
#line 145 "H5LTparse.y"
{ yyval.ival = H5Tcopy(H5T_IEEE_F64BE); }
break;
case 42:
#line 146 "H5LTparse.y"
{ yyval.ival = H5Tcopy(H5T_IEEE_F64LE); }
break;
case 43:
#line 147 "H5LTparse.y"
{ yyval.ival = H5Tcopy(H5T_NATIVE_FLOAT); }
break;
case 44:
#line 148 "H5LTparse.y"
{ yyval.ival = H5Tcopy(H5T_NATIVE_DOUBLE); }
break;
case 45:
#line 149 "H5LTparse.y"
{ yyval.ival = H5Tcopy(H5T_NATIVE_LDOUBLE); }
break;
case 46:
#line 153 "H5LTparse.y"
{ csindex++; cmpd_stack[csindex].id = H5Tcreate(H5T_COMPOUND, 1); /*temporarily set size to 1*/ }
break;
case 47:
#line 155 "H5LTparse.y"
{ yyval.ival = cmpd_stack[csindex].id; 
                              cmpd_stack[csindex].id = 0;
                              cmpd_stack[csindex].first_memb = 1; 
                              csindex--;
                            }
break;
case 50:
#line 164 "H5LTparse.y"
{ cmpd_stack[csindex].is_field = 1; /*notify lexer a compound member is parsed*/ }
break;
case 51:
#line 166 "H5LTparse.y"
{   
                            size_t origin_size, new_size;
                            hid_t dtype_id = cmpd_stack[csindex].id;

                            /*Adjust size and insert member, consider both member size and offset.*/
                            if(cmpd_stack[csindex].first_memb) { /*reclaim the size 1 temporarily set*/
                                new_size = H5Tget_size(yyvsp[-6].ival) + yyvsp[-1].ival;
                                H5Tset_size(dtype_id, new_size);
                                /*member name is saved in yylval.sval by lexer*/
                                H5Tinsert(dtype_id, yyvsp[-3].sval, yyvsp[-1].ival, yyvsp[-6].ival);

                                cmpd_stack[csindex].first_memb = 0;
                            } else {
                                origin_size = H5Tget_size(dtype_id);
                                
                                if(yyvsp[-1].ival == 0) {
                                    new_size = origin_size + H5Tget_size(yyvsp[-6].ival);
                                    H5Tset_size(dtype_id, new_size);
                                    H5Tinsert(dtype_id, yyvsp[-3].sval, origin_size, yyvsp[-6].ival);
                                } else {
                                    new_size = yyvsp[-1].ival + H5Tget_size(yyvsp[-6].ival);
                                    H5Tset_size(dtype_id, new_size);
                                    H5Tinsert(dtype_id, yyvsp[-3].sval, yyvsp[-1].ival, yyvsp[-6].ival);
                                }
                            }
                          
                            cmpd_stack[csindex].is_field = 0;
                            H5Tclose(yyvsp[-6].ival);
                             
                            new_size = H5Tget_size(dtype_id);
                        }
break;
case 52:
#line 199 "H5LTparse.y"
{
                            yyval.sval = yylval.sval;
                        }
break;
case 53:
#line 204 "H5LTparse.y"
{ yyval.ival = 0; }
break;
case 54:
#line 206 "H5LTparse.y"
{ yyval.ival = yylval.ival; }
break;
case 56:
#line 210 "H5LTparse.y"
{ asindex++; /*pushd onto the stack*/ }
break;
case 57:
#line 212 "H5LTparse.y"
{ 
                          yyval.ival = H5Tarray_create(yyvsp[-1].ival, arr_stack[asindex].ndims, arr_stack[asindex].dims, NULL);
                          arr_stack[asindex].ndims = 0;
                          asindex--;
                          H5Tclose(yyvsp[-1].ival);
                        }
break;
case 60:
#line 222 "H5LTparse.y"
{ arr_stack[asindex].is_dim = 1; /*notice lexer of dimension size*/ }
break;
case 61:
#line 223 "H5LTparse.y"
{ int ndims = arr_stack[asindex].ndims;
                                  arr_stack[asindex].dims[ndims] = (hsize_t)yylval.ival; 
                                  arr_stack[asindex].ndims++;
                                  arr_stack[asindex].is_dim = 0; 
                                }
break;
case 64:
#line 234 "H5LTparse.y"
{ yyval.ival = H5Tvlen_create(yyvsp[-1].ival); H5Tclose(yyvsp[-1].ival); }
break;
case 65:
#line 239 "H5LTparse.y"
{ is_opq_size = 1; }
break;
case 66:
#line 240 "H5LTparse.y"
{   
                                size_t size = (size_t)yylval.ival;
                                yyval.ival = H5Tcreate(H5T_OPAQUE, size);
                                is_opq_size = 0;    
                            }
break;
case 67:
#line 245 "H5LTparse.y"
{ is_opq_tag = 1; }
break;
case 68:
#line 246 "H5LTparse.y"
{  
                                H5Tset_tag(yyvsp[-6].ival, yylval.sval);
                                is_opq_tag = 0;
                            }
break;
case 69:
#line 250 "H5LTparse.y"
{ yyval.ival = yyvsp[-8].ival; }
break;
case 72:
#line 258 "H5LTparse.y"
{ is_str_size = 1; }
break;
case 73:
#line 259 "H5LTparse.y"
{  
                                if(yyvsp[-1].ival == H5T_VARIABLE_TOKEN)
                                    is_variable = 1;
                                else 
                                    str_size = yylval.ival;
                                is_str_size = 0; 
                            }
break;
case 74:
#line 267 "H5LTparse.y"
{
                                if(yyvsp[-1].ival == H5T_STR_NULLTERM_TOKEN)
                                    str_pad = H5T_STR_NULLTERM;
                                else if(yyvsp[-1].ival == H5T_STR_NULLPAD_TOKEN)
                                    str_pad = H5T_STR_NULLPAD;
                                else if(yyvsp[-1].ival == H5T_STR_SPACEPAD_TOKEN)
                                    str_pad = H5T_STR_SPACEPAD;
                            }
break;
case 75:
#line 276 "H5LTparse.y"
{  
                                if(yyvsp[-1].ival == H5T_CSET_ASCII_TOKEN)
                                    str_cset = H5T_CSET_ASCII;
                                else if(yyvsp[-1].ival == H5T_CSET_UTF8_TOKEN)
                                    str_cset = H5T_CSET_UTF8;
                            }
break;
case 76:
#line 283 "H5LTparse.y"
{
                                if(yyvsp[-1].ival == H5T_C_S1_TOKEN)
                                    yyval.ival = H5Tcopy(H5T_C_S1);
                                else if(yyvsp[-1].ival == H5T_FORTRAN_S1_TOKEN)
                                    yyval.ival = H5Tcopy(H5T_FORTRAN_S1);
                            }
break;
case 77:
#line 290 "H5LTparse.y"
{   
                                hid_t str_id = yyvsp[-1].ival;

                                /*set string size*/
                                if(is_variable) {
                                    H5Tset_size(str_id, H5T_VARIABLE);
                                    is_variable = 0;
                                } else
                                    H5Tset_size(str_id, str_size);
                                
                                /*set string padding and character set*/
                                H5Tset_strpad(str_id, str_pad);
                                H5Tset_cset(str_id, str_cset);

                                yyval.ival = str_id; 
                            }
break;
case 78:
#line 307 "H5LTparse.y"
{yyval.ival = H5T_VARIABLE_TOKEN;}
break;
case 80:
#line 310 "H5LTparse.y"
{yyval.ival = H5T_STR_NULLTERM_TOKEN;}
break;
case 81:
#line 311 "H5LTparse.y"
{yyval.ival = H5T_STR_NULLPAD_TOKEN;}
break;
case 82:
#line 312 "H5LTparse.y"
{yyval.ival = H5T_STR_SPACEPAD_TOKEN;}
break;
case 83:
#line 314 "H5LTparse.y"
{yyval.ival = H5T_CSET_ASCII_TOKEN;}
break;
case 84:
#line 315 "H5LTparse.y"
{yyval.ival = H5T_CSET_UTF8_TOKEN;}
break;
case 85:
#line 317 "H5LTparse.y"
{yyval.ival = H5T_C_S1_TOKEN;}
break;
case 86:
#line 318 "H5LTparse.y"
{yyval.ival = H5T_FORTRAN_S1_TOKEN;}
break;
case 87:
#line 322 "H5LTparse.y"
{ is_enum = 1; enum_id = H5Tenum_create(yyvsp[-1].ival); H5Tclose(yyvsp[-1].ival); }
break;
case 88:
#line 324 "H5LTparse.y"
{ is_enum = 0; /*reset*/ yyval.ival = enum_id; }
break;
case 91:
#line 329 "H5LTparse.y"
{
                                                is_enum_memb = 1; /*indicate member of enum*/
                                                enum_memb_symbol = strdup(yylval.sval); 
                                            }
break;
case 92:
#line 334 "H5LTparse.y"
{
                                char char_val=(char)yylval.ival;
                                short short_val=(short)yylval.ival;
                                int int_val=(int)yylval.ival;
                                long long_val=(long)yylval.ival;
                                long long llong_val=(long long)yylval.ival;
                                hid_t super = H5Tget_super(enum_id);
                                hid_t native = H5Tget_native_type(super, H5T_DIR_ASCEND);
                                
                                if(is_enum && is_enum_memb) { /*if it's an enum member*/
                                    /*To handle machines of different endianness*/
                                    if(H5Tequal(native, H5T_NATIVE_SCHAR) || H5Tequal(native, H5T_NATIVE_UCHAR))
                                        H5Tenum_insert(enum_id, enum_memb_symbol, &char_val);
                                    else if(H5Tequal(native, H5T_NATIVE_SHORT) || H5Tequal(native, H5T_NATIVE_USHORT))
                                        H5Tenum_insert(enum_id, enum_memb_symbol, &short_val);
                                    else if(H5Tequal(native, H5T_NATIVE_INT) || H5Tequal(native, H5T_NATIVE_UINT))
                                        H5Tenum_insert(enum_id, enum_memb_symbol, &int_val);
                                    else if(H5Tequal(native, H5T_NATIVE_LONG) || H5Tequal(native, H5T_NATIVE_ULONG))
                                        H5Tenum_insert(enum_id, enum_memb_symbol, &long_val);
                                    else if(H5Tequal(native, H5T_NATIVE_LLONG) || H5Tequal(native, H5T_NATIVE_ULLONG))
                                        H5Tenum_insert(enum_id, enum_memb_symbol, &llong_val);

                                    is_enum_memb = 0; 
                                    if(enum_memb_symbol) free(enum_memb_symbol);
                                }

                                H5Tclose(super);
                                H5Tclose(native);
                            }
break;
#line 1037 "H5LTparse.c"
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yyssp, yystate);
#endif
    if (yyssp >= yysslim && yygrowstack())
    {
        goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;
yyoverflow:
    yyerror("yacc stack overflow");
yyabort:
    return (1);
yyaccept:
    return (0);
}
