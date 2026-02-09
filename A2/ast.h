#include<iostream>
#include<string>

class AST{
public:
    int numChild;

    Ast();
    ~Ast();
    
}

class assignment_AST{
public:
    AST* lhs;
    AST* rhs;
    assignment_AST(AST* left, AST* right);
    ~assignment_AST();
}

class name_AST{
public:
    string name;

}