#ifndef SURFACE_H
#define SURFACE_H

#include <vector>
#include <stdio.h>
#include "types.h"

enum TK_Type
{ 
    TK_LEFTP, TK_RIGHTP, TK_LEFTS, TK_RIGHTS, TK_COMMA, TK_PLUS, TK_MINUS, TK_TIMES,
    TK_DIVIDE, TK_POW, TK_NUM, TK_CONST, TK_VAR, TK_FUNC, TK_END
};

enum TK_Const
{ PI, E };

enum TK_Var
{ U, V };

enum TK_Func
{ SIN, COS, TAN, ARCSIN, ARCCOS, ARCTAN, EXP, LOG, SQRT };

enum NT
{ NT_VECTOR, NT_ADDITIVE, NT_MULTIPLICATIVE, NT_UNARY, NT_EXPONENTIAL, NT_FACTOR };

struct Token
{
    TK_Type type;
    TK_Const constant;
    TK_Var variable;
    TK_Func function;
    float number;
};

struct Lexer
{
    const char *lexeme;
    int lexemeLength;
    Token token;
    void init(const char*);
    void nextToken();
};

enum Expr
{
    EXPR_SURFACE, EXPR_VECTOR, EXPR_PLUS, EXPR_MINUS, EXPR_TIMES, EXPR_DIVIDE,
    EXPR_POW, EXPR_NUM, EXPR_CONST, EXPR_VAR, EXPR_FUNC
};

struct Expr_Quinary
{
    Expr type;
    Expr *s, *w, *x, *y, *z;
};

struct Expr_Ternary
{
    Expr type;
    Expr *x, *y, *z;
};

struct Expr_Binary
{
    Expr type;
    Expr *x, *y;
};

/*
struct Expr_Unary
{
    Expr type;
    Expr *x;
};*/

struct Expr_Num
{
    Expr type;
    float x;
};

struct Expr_Const
{
    Expr type;
    TK_Const x;
};

struct Expr_Var
{
    Expr type;
    TK_Var x;
};

struct Expr_Func
{
    Expr type;
    TK_Func x;
    Expr *y;
};

struct Compiler
{
    int var_num;
    FILE *file;
    void *library;

    ~Compiler();
    
    quad   (*limits)(float, float);
    vector (*surface)(float, float);
    vector (*partial_u)(float, float);
    vector (*partial_v)(float, float);
    float  (*metric_e)(float, float);
    float  (*metric_f)(float, float);
    float  (*metric_g)(float, float);
    float (*metric_e_u)(float, float);
    float (*metric_e_v)(float, float);
    float (*metric_f_u)(float, float);
    float (*metric_f_v)(float, float);
    float (*metric_g_u)(float, float);
    float (*metric_g_v)(float, float);
    uv (*accel)(uv, uv);

    void compile(Expr*);
    void exp_code(Expr*);
    void profile();
};

struct Environment
{
    std::vector<Expr*> expressions;

    ~Environment();

    void clear();
    void print(Expr*, NT);
    Expr *copy(Expr*);
    Expr *simplify(Expr*);
    Expr *derivative(Expr*, TK_Var);
    Expr *surface(Expr*, Expr*, Expr*, Expr*, Expr*);
    Expr *vector(Expr*, Expr*, Expr*);
    Expr *plus(Expr*, Expr*);
    Expr *minus(Expr*, Expr*);
    Expr *times(Expr*, Expr*);
    Expr *divide(Expr*, Expr*);
    Expr *power(Expr*, Expr*);
    Expr *number(float);
    Expr *constant(TK_Const);
    Expr *variable(TK_Var);
    Expr *function(TK_Func, Expr*);
};

struct Parser
{
    Lexer lex;
    Environment env;

    void init(const char *);

    Expr *surface();
    Expr *vector();
    Expr *additive();
    Expr *multiplicative();
    Expr *unary();
    Expr *exponential();
    Expr *factor();
};

#endif