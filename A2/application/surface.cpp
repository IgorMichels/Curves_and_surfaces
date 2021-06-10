#include "surface.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <dlfcn.h>

#define ERROR (printf("Error at %s : %d\n", __FILE__, __LINE__))

void Lexer::init(const char *text)
{
    this->lexeme = text;
    this->lexemeLength = 0;
    nextToken();
}

void Lexer::nextToken()
{
    lexeme += lexemeLength;

    try_again:
    while(*lexeme == ' ' || *lexeme == '\t' || *lexeme == '\n') lexeme++;

    switch(*lexeme)
    {
        case '(':
            lexemeLength = 1;
            token.type = TK_LEFTP;
            return;
        case ')':
            lexemeLength = 1;
            token.type = TK_RIGHTP;
            return;
        case '[':
            lexemeLength = 1;
            token.type = TK_LEFTS;
            return;
        case ']':
            lexemeLength = 1;
            token.type = TK_RIGHTS;
            return;
        case ',':
            lexemeLength = 1;
            token.type = TK_COMMA;
            return;
        case '+':
            lexemeLength = 1;
            token.type = TK_PLUS;
            return;
        case '-':
            lexemeLength = 1;
            token.type = TK_MINUS;
            return;
        case '*':
            lexemeLength = 1;
            token.type = TK_TIMES;
            return;
        case '/':
            lexemeLength = 1;
            token.type = TK_DIVIDE;
            return;
        case '^':
            lexemeLength = 1;
            token.type = TK_POW;
            return;
        case 0:
            lexemeLength = 0;
            token.type = TK_END;
            return;
        default:
            if(isdigit(*lexeme))
            {
                int k = sscanf(lexeme, "%f%n", &token.number, &lexemeLength);
                token.type = TK_NUM;
                return;
            }

            if(isalpha(*lexeme))
            {
                static char buffer[16];
                for(lexemeLength = 0; isalpha(lexeme[lexemeLength]); lexemeLength++)
                    buffer[lexemeLength] = lexeme[lexemeLength];
                buffer[lexemeLength] = 0;

                if(strcmp(buffer, "pi") == 0)
                {
                    token.type = TK_CONST;
                    token.constant = TK_Const::PI;
                    return;
                }

                if(strcmp(buffer, "e") == 0)
                {
                    token.type = TK_CONST;
                    token.constant = TK_Const::E;
                    return;
                }

                if(strcmp(buffer, "u") == 0)
                {
                    token.type = TK_VAR;
                    token.variable = TK_Var::U;
                    return;
                }

                if(strcmp(buffer, "v") == 0)
                {
                    token.type = TK_VAR;
                    token.variable = TK_Var::V;
                    return;
                }

                if(strcmp(buffer, "sin") == 0)
                {
                    token.type = TK_FUNC;
                    token.function = TK_Func::SIN;
                    return;
                }

                if(strcmp(buffer, "cos") == 0)
                {
                    token.type = TK_FUNC;
                    token.function = TK_Func::COS;
                    return;
                }

                if(strcmp(buffer, "tan") == 0)
                {
                    token.type = TK_FUNC;
                    token.function = TK_Func::TAN;
                    return;
                }

                if(strcmp(buffer, "arcsin") == 0)
                {
                    token.type = TK_FUNC;
                    token.function = TK_Func::ARCSIN;
                    return;
                }

                if(strcmp(buffer, "arccos") == 0)
                {
                    token.type = TK_FUNC;
                    token.function = TK_Func::ARCCOS;
                    return;
                }

                if(strcmp(buffer, "arctan") == 0)
                {
                    token.type = TK_FUNC;
                    token.function = TK_Func::ARCTAN;
                    return;
                }

                if(strcmp(buffer, "exp") == 0)
                {
                    token.type = TK_FUNC;
                    token.function = TK_Func::EXP;
                    return;
                }

                if(strcmp(buffer, "log") == 0)
                {
                    token.type = TK_FUNC;
                    token.function = TK_Func::LOG;
                    return;
                }

                if(strcmp(buffer, "sqrt") == 0)
                {
                    token.type = TK_FUNC;
                    token.function = TK_Func::SQRT;
                    return;
                }
            }
    }

    //panic mode
    lexeme++;
    goto try_again;
}

void Parser::init(const char *text)
{
    lex.init(text);
}

Expr *Parser::surface()
{
    Expr *s = vector();

    if(lex.token.type != TK_LEFTS)
    {
        ERROR;
        return nullptr;
    }

    lex.nextToken();

    Expr *w = additive();

    if(lex.token.type != TK_COMMA)
    {
        ERROR;
        return nullptr;
    }

    lex.nextToken();

    Expr *x = additive();

    if(lex.token.type != TK_RIGHTS)
    {
        ERROR;
        return nullptr;
    }

    lex.nextToken();

    if(lex.token.type != TK_LEFTS)
    {
        ERROR;
        return nullptr;
    }

    lex.nextToken();

    Expr *y = additive();

    if(lex.token.type != TK_COMMA)
    {
        ERROR;
        return nullptr;
    }

    lex.nextToken();

    Expr *z = additive();

    if(lex.token.type != TK_RIGHTS)
    {
        ERROR;
        return nullptr;
    }

    lex.nextToken();
    return env.surface(s, w, x, y, z);
}

Expr *Parser::vector()
{
    if(lex.token.type != TK_LEFTP)
    {
        ERROR;
        return nullptr;
    }

    lex.nextToken();

    Expr *x = additive();

    if(lex.token.type != TK_COMMA)
    {
        ERROR;
        return nullptr;
    }

    lex.nextToken();

    Expr *y = additive();

    if(lex.token.type != TK_COMMA)
    {
        ERROR;
        return nullptr;
    }

    lex.nextToken();

    Expr *z = additive();

    if(lex.token.type != TK_RIGHTP)
    {
        ERROR;
        return nullptr;
    }

    lex.nextToken();
    return env.vector(x, y, z);
}

Expr *Parser::additive()
{
    Expr *left = multiplicative();

    while(true)
    {
        if(lex.token.type == TK_PLUS)
        {
            lex.nextToken();
            left = env.plus(left, multiplicative());
        }
        else if(lex.token.type == TK_MINUS)
        {
            lex.nextToken();
            left = env.minus(left, multiplicative());
        }
        else break;
    }

    return left;
}

Expr *Parser::multiplicative()
{
    Expr *left = unary();

    while(true)
    {
        if(lex.token.type == TK_TIMES)
        {
            lex.nextToken();
            left = env.times(left, unary());
        }
        else if(lex.token.type == TK_DIVIDE)
        {
            lex.nextToken();
            left = env.divide(left, unary());
        }
        else
        {
            switch(lex.token.type)
            {
                case TK_NUM:
                case TK_CONST:
                case TK_LEFTP:
                case TK_VAR:
                case TK_FUNC:
                    left = env.times(left, exponential());
                    break;
                default:
                    goto loop_end;
            }
        }
    }

    loop_end:
    return left;
}

Expr *Parser::unary()
{
    if(lex.token.type == TK_PLUS)
    {
        lex.nextToken();
        return unary();
    }
    else if(lex.token.type == TK_MINUS)
    {
        lex.nextToken();
        return env.times(env.number(-1), unary());
    }
    else return exponential();
}

Expr *Parser::exponential()
{
    Expr *left = factor();

    if(lex.token.type == TK_POW)
    {
        lex.nextToken();
        left = env.power(left, exponential());
    }

    return left;
}

Expr *Parser::factor()
{
    if(lex.token.type == TK_NUM)
    {
        Expr *n = env.number(lex.token.number);
        lex.nextToken();
        return n;
    }

    if(lex.token.type == TK_CONST)
    {
        Expr *c = env.constant(lex.token.constant);
        lex.nextToken();
        return c;
    }

    if(lex.token.type == TK_VAR)
    {
        Expr *v = env.variable(lex.token.variable);
        lex.nextToken();
        return v;
    }

    if(lex.token.type == TK_LEFTP)
    {
        lex.nextToken();
        Expr *e = additive();

        if(lex.token.type != TK_RIGHTP)
        {
            ERROR;
            return nullptr;
        }

        lex.nextToken();
        return e;
    }

    if(lex.token.type == TK_FUNC)
    {
        TK_Func f = lex.token.function;
        lex.nextToken();
        return env.function(f, factor());
    }

    ERROR;
    return nullptr;
}

Environment::~Environment()
{
    for(int i = 0; i < expressions.size(); i++)
        if(expressions[i]) 
            delete expressions[i];

    printf("Freed %ld expressions\n", expressions.size());

    expressions.clear();
}

void Environment::clear()
{
    for(int i = 0; i < expressions.size(); i++)
    {
        if(expressions[i]) delete expressions[i];
    }

    expressions.clear();
}

Expr *Environment::copy(Expr *exp)
{
    if(!exp) return nullptr;

    switch(*exp)
    {
        case EXPR_VECTOR:
        {
            Expr_Ternary *ter = new Expr_Ternary;
            ter->type = *exp;
            ter->x = copy(((Expr_Ternary*)exp)->x);
            ter->y = copy(((Expr_Ternary*)exp)->y);
            ter->z = copy(((Expr_Ternary*)exp)->z);
            expressions.push_back(&ter->type);
            return &ter->type;
        }
        case EXPR_PLUS:
        case EXPR_MINUS:
        case EXPR_TIMES:
        case EXPR_DIVIDE:
        case EXPR_POW:
        {
            Expr_Binary *bin = new Expr_Binary;
            bin->type = *exp;
            bin->x = copy(((Expr_Ternary*)exp)->x);
            bin->y = copy(((Expr_Ternary*)exp)->y);
            expressions.push_back(&bin->type);
            return &bin->type;
        }
        case EXPR_NUM:
        {
            Expr_Num *n = new Expr_Num;
            n->type = *exp;
            n->x = ((Expr_Num*)exp)->x;
            expressions.push_back(&n->type);
            return &n->type;
        }
        case EXPR_CONST:
        {
            Expr_Const *c = new Expr_Const;
            c->type = *exp;
            c->x = ((Expr_Const*)exp)->x;
            expressions.push_back(&c->type);
            return &c->type;
        }
        case EXPR_VAR:
        {
            Expr_Var *v = new Expr_Var;
            v->type = *exp;
            v->x = ((Expr_Var*)exp)->x;
            expressions.push_back(&v->type);
            return &v->type;
        }
        case EXPR_FUNC:
        {
            Expr_Func *f = new Expr_Func;
            f->type = *exp;
            f->x = ((Expr_Func*)exp)->x;
            f->y = copy(((Expr_Func*)exp)->y);
            expressions.push_back(&f->type);
            return &f->type;
        }
    }
    return nullptr;
}

Expr *Environment::simplify(Expr *exp)
{

}

void Environment::print(Expr *exp, NT nt)
{
    if(!exp) return;

    Expr_Ternary *ter = (Expr_Ternary*)exp;
    Expr_Binary *bin = (Expr_Binary*)exp;
    Expr_Num *num = (Expr_Num*)exp;
    Expr_Const *cst = (Expr_Const*)exp;
    Expr_Var *var = (Expr_Var*)exp;
    Expr_Func *func = (Expr_Func*)exp;

    switch(*exp)
    {
        case EXPR_VECTOR:
            if(nt != NT_VECTOR) return;
            printf("( ");
            print(ter->x, NT_ADDITIVE);
            printf(", ");
            print(ter->y, NT_ADDITIVE);
            printf(", ");
            print(ter->z, NT_ADDITIVE);
            printf(") ");
            break;
        case EXPR_PLUS:
            if(nt == NT_VECTOR) return;
            if(nt != NT_ADDITIVE) printf("(");
            print(bin->x, NT_ADDITIVE);
            printf(" + ");
            print(bin->y, NT_ADDITIVE);
            if(nt != NT_ADDITIVE) printf(")");
            break;
        case EXPR_MINUS:
            if(nt == NT_VECTOR) return;
            if(nt != NT_ADDITIVE) printf("(");
            print(bin->x, NT_ADDITIVE);
            printf(" - ");
            print(bin->y, NT_MULTIPLICATIVE);
            if(nt != NT_ADDITIVE) printf(")");
            break;
        case EXPR_TIMES:
            if(nt == NT_VECTOR) return;
            if(nt != NT_ADDITIVE && nt != NT_MULTIPLICATIVE) printf("(");
            print(bin->x, NT_MULTIPLICATIVE);
            printf("*");
            print(bin->y, NT_MULTIPLICATIVE);
            if(nt != NT_ADDITIVE && nt != NT_MULTIPLICATIVE) printf(")");
            break;
        case EXPR_DIVIDE:
            if(nt == NT_VECTOR) return;
            if(nt != NT_ADDITIVE && nt != NT_MULTIPLICATIVE) printf("(");
            print(bin->x, NT_MULTIPLICATIVE);
            printf("/");
            print(bin->y, NT_UNARY);
            if(nt != NT_ADDITIVE && nt != NT_MULTIPLICATIVE) printf(")");
            break;
        case EXPR_POW:
            if(nt == NT_VECTOR) return;
            if(nt == NT_FACTOR) printf("(");
            print(bin->x, NT_FACTOR);
            printf("^");
            print(bin->y, NT_EXPONENTIAL);
            if(nt == NT_FACTOR) printf(")");
            break;
        case EXPR_NUM:
            if(nt == NT_VECTOR) return;
            printf("%g", num->x);
            break;
        case EXPR_CONST:
            if(nt == NT_VECTOR) return;
            switch(cst->x)
            {
                case PI:
                    printf("pi");
                    break;
                case E:
                    printf("exp");
                    break;
            }
            break;
        case EXPR_VAR:
            if(nt == NT_VECTOR) return;
            switch(var->x)
            {
                case U:
                    printf("u");
                    break;
                case V:
                    printf("v");
                    break;
            }
            break;

        case EXPR_FUNC:
            if(nt == NT_VECTOR) return;
            switch(func->x)
            {
                case SIN:
                    printf("sin ");
                    break;
                case COS:
                    printf("cos ");
                    break;
                case TAN:
                    printf("tan ");
                    break;
                case ARCSIN:
                    printf("arcsin ");
                    break;
                case ARCCOS:
                    printf("arccos ");
                    break;
                case ARCTAN:
                    printf("arctan ");
                    break;
                case EXP:
                    printf("exp ");
                    break;
                case LOG:
                    printf("log ");
                    break;
                case SQRT:
                    printf("sqrt ");
                    break;
            }
            
            print(func->y, NT_FACTOR);

            break;
    }
}

Expr *Environment::derivative(Expr *exp, TK_Var dx)
{
    if(!exp) return nullptr;

    Expr_Ternary *ter = (Expr_Ternary*)exp;
    Expr_Binary *bin = (Expr_Binary*)exp;
    Expr_Num *num = (Expr_Num*)exp;
    Expr_Const *cst = (Expr_Const*)exp;
    Expr_Var *var = (Expr_Var*)exp;
    Expr_Func *func = (Expr_Func*)exp;

    switch(*exp)
    {
        case EXPR_VECTOR: return vector(derivative(ter->x, dx), derivative(ter->y, dx), derivative(ter->z, dx));
        case EXPR_PLUS: return plus(derivative(bin->x, dx), derivative(bin->y, dx));
        case EXPR_MINUS: return minus(derivative(bin->x, dx), derivative(bin->y, dx));
        case EXPR_TIMES: return plus(times(bin->x, derivative(bin->y, dx)), times(bin->y, derivative(bin->x, dx)));
        case EXPR_DIVIDE: return divide(minus(times(derivative(bin->x, dx), (bin->y)), times(derivative(bin->y, dx), (bin->x))), power(bin->y, number(2)));
        case EXPR_POW: return times(power(bin->x, (bin->y)), plus(times(derivative(bin->y, dx), function(LOG, (bin->x))), times(bin->y, divide(derivative(bin->x, dx), (bin->x)))));
        case EXPR_NUM: return number(0);
        case EXPR_CONST: return number(0);
        case EXPR_VAR: return number((var->x == dx) ? 1 : 0);
        case EXPR_FUNC:
            switch(func->x)
            {
                case SIN: return times(function(COS, (func->y)), derivative(func->y, dx));
                case COS: return times(times(number(-1), function(SIN, (func->y))), derivative(func->y, dx));
                case TAN: return times(divide(number(1), power(function(COS, (func->y)), number(2))), derivative(func->y, dx));
                case ARCSIN: return divide(derivative(func->y, dx), function(SQRT, minus(number(1), power(func->y, number(2)))));
                case ARCCOS: return divide(times(number(-1), derivative(func->y, dx)), function(SQRT, minus(number(1), power(func->y, number(2)))));
                case ARCTAN: return divide(derivative(func->y, dx), plus(number(1), power(func->y, number(2))));
                case EXP: return times(function(EXP, (func->y)), derivative(func->y, dx));
                case LOG: return divide(derivative(func->y, dx), (func->y));
                case SQRT: return divide(derivative(func->y, dx), times(number(2), function(SQRT, (func->y))));
            }
    }
}

Expr *Environment::surface(Expr *s, Expr *w, Expr *x, Expr *y, Expr *z)
{
    if(!s || !w || !x || !y || !z) return nullptr;
    Expr_Quinary *qui = new Expr_Quinary;
    qui->type = EXPR_SURFACE;
    qui->s = s;
    qui->w = w;
    qui->x = x;
    qui->y = y;
    qui->z = z;
    expressions.push_back(&qui->type);
    return &qui->type;
}

Expr *Environment::vector(Expr *x, Expr *y, Expr *z)
{
    if(!x || !y || !z) return nullptr;
    Expr_Ternary *ter = new Expr_Ternary;
    ter->type = EXPR_VECTOR;
    ter->x = x;
    ter->y = y;
    ter->z = z;
    expressions.push_back(&ter->type);
    return &ter->type;
}

Expr *Environment::plus(Expr *x, Expr *y)
{
    if(!x || !y) return nullptr;
    Expr_Binary *bin = new Expr_Binary;
    bin->type = EXPR_PLUS;
    bin->x = x;
    bin->y = y;
    expressions.push_back(&bin->type);
    return &bin->type;
}

Expr *Environment::minus(Expr *x, Expr *y)
{
    if(!x || !y) return nullptr;
    Expr_Binary *bin = new Expr_Binary;
    bin->type = EXPR_MINUS;
    bin->x = x;
    bin->y = y;
    expressions.push_back(&bin->type);
    return &bin->type;
}

Expr *Environment::times(Expr *x, Expr *y)
{
    if(!x || !y) return nullptr;
    Expr_Binary *bin = new Expr_Binary;
    bin->type = EXPR_TIMES;
    bin->x = x;
    bin->y = y;
    expressions.push_back(&bin->type);
    return &bin->type;
}

Expr *Environment::divide(Expr *x, Expr *y)
{
    if(!x || !y) return nullptr;
    Expr_Binary *bin = new Expr_Binary;
    bin->type = EXPR_DIVIDE;
    bin->x = x;
    bin->y = y;
    expressions.push_back(&bin->type);
    return &bin->type;
}

Expr *Environment::power(Expr *x, Expr *y)
{
    if(!x || !y) return nullptr;
    Expr_Binary *bin = new Expr_Binary;
    bin->type = EXPR_POW;
    bin->x = x;
    bin->y = y;
    expressions.push_back(&bin->type);
    return &bin->type;
}

Expr *Environment::number(float x)
{
    Expr_Num *num = new Expr_Num;
    num->type = EXPR_NUM;
    num->x = x;
    expressions.push_back(&num->type);
    return &num->type;
}

Expr *Environment::constant(TK_Const x)
{
    Expr_Const *cst = new Expr_Const;
    cst->type = EXPR_CONST;
    cst->x = x;
    expressions.push_back(&cst->type);
    return &cst->type;
}

Expr *Environment::variable(TK_Var x)
{
    Expr_Var *var = new Expr_Var;
    var->type = EXPR_VAR;
    var->x = x;
    expressions.push_back(&var->type);
    return &var->type;
}

Expr *Environment::function(TK_Func x, Expr *y)
{
    if(!y) return nullptr;
    Expr_Func *func = new Expr_Func;
    func->type = EXPR_FUNC;
    func->x = x;
    func->y = y;
    expressions.push_back(&func->type);
    return &func->type;
}

Compiler::~Compiler()
{
    if(library) dlclose(library);
}

void Compiler::exp_code(Expr *exp)
{
    Expr_Ternary *ter = (Expr_Ternary*)exp;
    Expr_Binary *bin = (Expr_Binary*)exp;
    Expr_Num *num = (Expr_Num*)exp;
    Expr_Const *cst = (Expr_Const*)exp;
    Expr_Var *var = (Expr_Var*)exp;
    Expr_Func *func = (Expr_Func*)exp;

    switch(*exp)
    {
        case EXPR_VECTOR:
            {
                exp_code(ter->x);
                int e1 = var_num;
                exp_code(ter->y);
                int e2 = var_num;
                exp_code(ter->z);
                int e3 = var_num;
                var_num++;
                fprintf(file, "vector v%d = (vector){v%d, v%d, v%d};\n", var_num, e1, e2, e3);
            }
            return;
        case EXPR_PLUS:
            {
                exp_code(bin->x);
                int e1 = var_num;
                exp_code(bin->y);
                int e2 = var_num;
                var_num++;
                fprintf(file, "float v%d = v%d + v%d;\n", var_num, e1, e2);
            }
            return;
        case EXPR_MINUS:
            {
                exp_code(bin->x);
                int e1 = var_num;
                exp_code(bin->y);
                int e2 = var_num;
                var_num++;
                fprintf(file, "float v%d = v%d - v%d;\n", var_num, e1, e2);
            }
            return;
        case EXPR_TIMES:
            {
                exp_code(bin->x);
                int e1 = var_num;
                exp_code(bin->y);
                int e2 = var_num;
                var_num++;
                fprintf(file, "float v%d = v%d * v%d;\n", var_num, e1, e2);
            }
            return;
        case EXPR_DIVIDE:
            {
                exp_code(bin->x);
                int e1 = var_num;
                exp_code(bin->y);
                int e2 = var_num;
                var_num++;
                fprintf(file, "float v%d = v%d / v%d;\n", var_num, e1, e2);
            }
            return;
        case EXPR_POW:
            {
                exp_code(bin->x);
                int e1 = var_num;
                exp_code(bin->y);
                int e2 = var_num;
                var_num++;
                fprintf(file, "float v%d = pow(v%d, v%d);\n", var_num, e1, e2);
            }
            return;
        case EXPR_NUM:
            {
                var_num++;
                fprintf(file, "float v%d = %f;\n", var_num, num->x);
            }
            return;
        case EXPR_CONST:
            {
                var_num++;
                fprintf(file, "float v%d = %s;\n", var_num, (cst->x == E) ? "e" : "pi");
            }
            return;
        case EXPR_VAR:
            {
                var_num++;
                fprintf(file, "float v%d = %s;\n", var_num, (var->x == U) ? "u" : "v");
            }
            return;
        case EXPR_FUNC:
            {
                exp_code(func->y);
                int e1 = var_num;
                var_num++;
                const char *f_name;
                switch(func->x)
                {
                    case SIN: f_name = "sin"; break;
                    case COS: f_name = "cos"; break;
                    case TAN: f_name = "tan"; break;
                    case ARCSIN: f_name = "asin"; break;
                    case ARCCOS: f_name = "acos"; break;
                    case ARCTAN: f_name = "atan"; break;
                    case EXP: f_name = "exp"; break;
                    case LOG: f_name = "log"; break;
                    case SQRT: f_name = "sqrt"; break;
                }
                fprintf(file, "float v%d = %s(v%d);\n", var_num, f_name, e1);
            }
            return;
        default:
            fprintf(file, "?");      
    }
}

void Compiler::compile(Expr *exp)
{
    limits = nullptr;
    partial_u = nullptr;
    partial_v = nullptr;
    library = nullptr;
    metric_e = nullptr;
    metric_f = nullptr;
    metric_g = nullptr;
    metric_e_u = nullptr;
    metric_e_v = nullptr;
    metric_f_u = nullptr;
    metric_f_v = nullptr;
    metric_g_u = nullptr;
    metric_g_v = nullptr;
    accel = nullptr;

    if(!exp) return;
    if(*exp != EXPR_SURFACE) return;

    FILE *fcmd = fopen("cmd.txt", "rb");
    if(!fcmd) return;

    char cmd[512];
    int cmd_len = fread(cmd, 1, 512, fcmd);
    cmd[cmd_len] = 0;
    fclose(fcmd);

    file = fopen("output.cpp", "w");
    if(!file) return;

    Environment env;
    Expr_Quinary *qui = (Expr_Quinary*)exp;
    Expr_Ternary *ter = (Expr_Ternary*)qui->s;

    Expr *du = env.derivative(&ter->type, U);
    Expr *dv = env.derivative(&ter->type, V);

    Expr_Ternary *ter_du = (Expr_Ternary*)du;
    Expr_Ternary *ter_dv = (Expr_Ternary*)dv;

    Expr *me = env.plus(env.plus(env.times(ter_du->x, ter_du->x), env.times(ter_du->y, ter_du->y)), env.times(ter_du->z, ter_du->z));
    Expr *mf = env.plus(env.plus(env.times(ter_du->x, ter_dv->x), env.times(ter_du->y, ter_dv->y)), env.times(ter_du->z, ter_dv->z));
    Expr *mg = env.plus(env.plus(env.times(ter_dv->x, ter_dv->x), env.times(ter_dv->y, ter_dv->y)), env.times(ter_dv->z, ter_dv->z));
    
    Expr *me_u = env.derivative(me, U);
    Expr *me_v = env.derivative(me, V);
    Expr *mf_u = env.derivative(mf, U);
    Expr *mf_v = env.derivative(mf, V);
    Expr *mg_u = env.derivative(mg, U);
    Expr *mg_v = env.derivative(mg, V);


    fprintf(file, "#include \"header.h\"\n");

    //limits
    {
        var_num = 0;
        fprintf(file, "\nextern \"C\" quad limits(float u, float v)\n{\n");
        exp_code(qui->w);
        int e1 = var_num;
        exp_code(qui->x);
        int e2 = var_num;
        exp_code(qui->y);
        int e3 = var_num;
        exp_code(qui->z);
        int e4 = var_num;
        fprintf(file, "return (quad){v%d, v%d, v%d, v%d};\n}\n", e1, e2, e3, e4);
    }

    //surface
    {
        var_num = 0;
        fprintf(file, "\nextern \"C\" vector surface(float u, float v)\n{\n");
        exp_code(&ter->type);
        int e1 = var_num;
        fprintf(file, "return v%d;\n}\n", e1);
    }

    //partial_u
    {
        var_num = 0;
        fprintf(file, "\nextern \"C\" vector partial_u(float u, float v)\n{\n");
        exp_code(du);
        int e1 = var_num;
        fprintf(file, "return v%d;\n}\n", e1);
    }

    //partial_v
    {
        var_num = 0;
        fprintf(file, "\nextern \"C\" vector partial_v(float u, float v)\n{\n");
        exp_code(dv);
        int e1 = var_num;
        fprintf(file, "return v%d;\n}\n", e1);
    }

    //metric_e
    {
        fprintf(file, "\nextern \"C\" float metric_e(float u, float v)\n{\n");
        fprintf(file, "vector v1 = partial_u(u,v);\n");
        fprintf(file, "vector v2 = partial_u(u,v);\n");
        fprintf(file, "float v3 = v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;\n");
        fprintf(file, "return v3;\n}\n");
    }

    //metric_f
    {
        fprintf(file, "\nextern \"C\" float metric_f(float u, float v)\n{\n");
        fprintf(file, "vector v1 = partial_u(u,v);\n");
        fprintf(file, "vector v2 = partial_v(u,v);\n");
        fprintf(file, "float v3 = v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;\n");
        fprintf(file, "return v3;\n}\n");
    }

    //metric_g
    {
        fprintf(file, "\nextern \"C\" float metric_g(float u, float v)\n{\n");
        fprintf(file, "vector v1 = partial_v(u,v);\n");
        fprintf(file, "vector v2 = partial_v(u,v);\n");
        fprintf(file, "float v3 = v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;\n");
        fprintf(file, "return v3;\n}\n");
    }

    //metric_e_u
    {
        var_num = 0;
        fprintf(file, "\nextern \"C\" float metric_e_u(float u, float v)\n{\n");
        exp_code(me_u);
        int e1 = var_num;
        fprintf(file, "return v%d;\n}\n", e1);
    }

    //metric_e_v
    {
        var_num = 0;
        fprintf(file, "\nextern \"C\" float metric_e_v(float u, float v)\n{\n");
        exp_code(me_v);
        int e1 = var_num;
        fprintf(file, "return v%d;\n}\n", e1);
    }

    //metric_f_u
    {
        var_num = 0;
        fprintf(file, "\nextern \"C\" float metric_f_u(float u, float v)\n{\n");
        exp_code(mf_u);
        int e1 = var_num;
        fprintf(file, "return v%d;\n}\n", e1);
    }

    //metric_f_v
    {
        var_num = 0;
        fprintf(file, "\nextern \"C\" float metric_f_v(float u, float v)\n{\n");
        exp_code(mf_v);
        int e1 = var_num;
        fprintf(file, "return v%d;\n}\n", e1);
    }

    //metric_g_u
    {
        var_num = 0;
        fprintf(file, "\nextern \"C\" float metric_g_u(float u, float v)\n{\n");
        exp_code(mg_u);
        int e1 = var_num;
        fprintf(file, "return v%d;\n}\n", e1);
    }

    //metric_g_v
    {
        var_num = 0;
        fprintf(file, "\nextern \"C\" float metric_g_v(float u, float v)\n{\n");
        exp_code(mg_v);
        int e1 = var_num;
        fprintf(file, "return v%d;\n}\n", e1);
    }

    //accel
    {
        var_num = 0;
        fprintf(file, "\nextern \"C\" uv accel(uv pos, uv vel)\n{\n");
        fprintf(file, "float E2 = 2*metric_e(pos.u, pos.v);\n");
        fprintf(file, "float F2 = 2*metric_f(pos.u, pos.v);\n");
        fprintf(file, "float G2 = 2*metric_g(pos.u, pos.v);\n");
        fprintf(file, "float Eu = metric_e_u(pos.u, pos.v);\n");
        fprintf(file, "float Ev = metric_e_v(pos.u, pos.v);\n");
        fprintf(file, "float Fu = metric_f_u(pos.u, pos.v);\n");
        fprintf(file, "float Fv = metric_f_v(pos.u, pos.v);\n");
        fprintf(file, "float Gu = metric_g_u(pos.u, pos.v);\n");
        fprintf(file, "float Gv = metric_g_v(pos.u, pos.v);\n");
        fprintf(file, "uv x = { vel.v*vel.v*(Gu - 2*Fv) - vel.u*(Eu*vel.u + 2*Ev*vel.v), vel.u*vel.u*(Ev - 2*Fu) - vel.v*(Gv*vel.v + 2*Gu*vel.u)};\n");
        fprintf(file, "float det = E2*G2 - F2*F2;\n");
        fprintf(file, "uv ac = { G2*x.u/det - F2*x.v/det, -F2*x.u/det + E2*x.v/det };\n");
        fprintf(file, "return ac;\n}\n");
    }

    fclose(file);

    int k = system(cmd);

    if(k)
    {
        printf("Couldn't compile!\n");
        return;
    }

    library = dlopen("./output.so", RTLD_NOW);

    if(!library) return;

    limits    = (quad   (*)(float, float))dlsym(library, "limits");
    surface   = (vector (*)(float, float))dlsym(library, "surface");
    partial_u = (vector (*)(float, float))dlsym(library, "partial_u");
    partial_v = (vector (*)(float, float))dlsym(library, "partial_v");
    metric_e = (float (*)(float, float))dlsym(library, "metric_e");
    metric_f = (float (*)(float, float))dlsym(library, "metric_f");
    metric_g = (float (*)(float, float))dlsym(library, "metric_g");
    metric_e_u = (float (*)(float, float))dlsym(library, "metric_e_u");
    metric_e_v = (float (*)(float, float))dlsym(library, "metric_e_v");
    metric_f_u = (float (*)(float, float))dlsym(library, "metric_f_u");
    metric_f_v = (float (*)(float, float))dlsym(library, "metric_f_v");
    metric_g_u = (float (*)(float, float))dlsym(library, "metric_g_u");
    metric_g_v = (float (*)(float, float))dlsym(library, "metric_g_v");
    accel = (uv (*)(uv, uv))dlsym(library, "accel");

    if(!limits || !surface || !partial_u || !partial_v ||
    !metric_e || !metric_f || !metric_g ||
    !metric_e_u || !metric_e_v ||
    !metric_f_u || !metric_f_v ||
    !metric_g_u || !metric_g_v || !accel)
    {
        limits = nullptr;
        partial_u = nullptr;
        partial_v = nullptr;
        library = nullptr;
        metric_e = nullptr;
        metric_f = nullptr;
        metric_g = nullptr;
        metric_e_u = nullptr;
        metric_e_v = nullptr;
        metric_f_u = nullptr;
        metric_f_v = nullptr;
        metric_g_u = nullptr;
        metric_g_v = nullptr;
        accel = nullptr;
    }
}

#define PROFILE_N 10000000
#define PROFILE(x) { clock_t time0 = clock(); for(int i = 0; i < PROFILE_N; i++) volatile auto k = x(rand(), rand()); clock_t time1 = clock(); float time_diff = (float)(time1 - time0) / CLOCKS_PER_SEC; printf("%f", time_diff); }

void Compiler::profile() 
{
    if(!library) return;

    printf("**************** PROFILE ****************\n");

    printf("surface: ");
    PROFILE(surface);
    printf("\n");

    printf("partial_u: ");
    PROFILE(partial_u);
    printf("\n");

    printf("partial_v: ");
    PROFILE(partial_v);
    printf("\n");

    printf("metric_e: ");
    PROFILE(metric_e);
    printf("\n");

    printf("metric_f: ");
    PROFILE(metric_f);
    printf("\n");

    printf("metric_g: ");
    PROFILE(metric_g);
    printf("\n");

    printf("metric_e_u: ");
    PROFILE(metric_e_u);
    printf("\n");

    printf("metric_e_v: ");
    PROFILE(metric_e_v);
    printf("\n");

    printf("metric_f_u: ");
    PROFILE(metric_f_u);
    printf("\n");

    printf("metric_f_v: ");
    PROFILE(metric_f_v);
    printf("\n");

    printf("metric_g_u: ");
    PROFILE(metric_g_u);
    printf("\n");

    printf("metric_g_v: ");
    PROFILE(metric_g_v);
    printf("\n");

    printf("*****************************************\n");

}