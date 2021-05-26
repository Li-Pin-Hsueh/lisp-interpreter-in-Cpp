#include <iostream>
#include <string>
#include <sstream>
using namespace std;

enum TokenType
{
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
};

class Token
{
private:
  int row;
  int col;
  string text;
  TokenType type;

public:
  Token(int r, int c, string v, TokenType t);
  string toString();
}; // class Token

Token::Token(int r, int c, string v, TokenType t)
{
  this->col = c;
  this->row = r;
  this->text.assign(v);
  this->type = t;
} // Token()

string Token::toString()
{
  stringstream s1, s2 ;
  s1 << this->row ;
  s2 << this->col ;
  string strRow = s1.str() ;
  string strCol = s2.str() ;
  string s = "[Row]" + strRow + " [Column]" + strCol + " " + this->text + "\n";
  return s ;
} // toString()
