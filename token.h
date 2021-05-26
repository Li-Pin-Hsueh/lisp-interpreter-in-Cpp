# include <iostream>
# include <string>
using namespace std ;

enum TokenType {
  LP = 1,
  RP = 2,
  INT = 3,
  FLOAT = 4,
  STRING = 5,
  DOT = 6,
  NIL = 7,
  T = 8,
  QUOTE = 9,
  SYMBOL = 10
} ;

struct Token
{
    int row ;
    int col ;
    string value ;
    TokenType type ;
    Token( int r, int c, string v, TokenType t) {
        this->row = r ;
        this->col = c ;
        this->value.assign(v) ;
        this->type = t ;
    };
};
