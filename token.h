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
  int mRow;
  int mCol;
  string mText;
  TokenType mType;

public:
  Token( int r, int c, string v, TokenType t )
  {
    mRow = r ;
    mCol = c ;
    mText = v ;
    mType = t ;
  } // Token()

  string ToString()
  {
    stringstream s1, s2;
    s1 << mRow;
    s2 << mCol;
    string strRow = s1.str();
    string strCol = s2.str();
    string s = "[Row]" + strRow + " [Column]" + strCol + " " + mText + "\n";
    return s;
  } // toString()
};  // class Token
