#include <iostream>
#include <string>
#include <sstream>
using namespace std;

enum TokenType
{
  LP, RP, INT, FLOAT, STRING, DOT, NIL, T, QUOTE, SYMBOL
};

class Token
{
private:
  int mRow;
  int mCol;
  string mText;
  TokenType mType;
  bool IsInt( string s ) ;
  bool IsFloat( string s ) ;

public:
  Token( int r, int c, string v )
  {
    // TODO check type.
    // Type to check:
    // LP, RP, INT, FLOAT, DOT, NIL, T, QUOTE, SYMBOL
    TokenType t ;
    if ( v == "(" ) t = LP ;
    else if ( v == ")" ) t = RP ;
    else if ( v == "." ) t = DOT ;
    else if ( v == "'" ) t = QUOTE ;
    else if ( v == "nil" || v == "#f" ) v = "nil", t = NIL ;
    else if ( v == "t" || v == "#t" ) v = "#t", t = T ;
    else if ( IsInt(v) ) t = INT ;
    else if ( IsFloat(v) ) t = FLOAT ;
    else if ( v == "'" ) v = "QUOTE", t = QUOTE ;
    else if ( v.at( 0 ) == '\"' ) cout << "Token init ERROR: STRING..." << endl ;
    else t = SYMBOL ;

    mRow = r ;
    mCol = c ;
    mText = v ;
    mType = t ;
  } // Token()

  Token( int r, int c, string v, TokenType t )
  {
    mRow = r ;
    mCol = c ;
    mText = v ;
    mType = t ;
  } // Token()

  string ToString()
  {
    string typeArray[] = {"LP", "RP", "INT", "FLOAT",
                          "STRING", "DOT", "NIL", "T", "QUOTE", "SYMBOL"} ;
    stringstream s1, s2 ;
    s1 << mRow;
    s2 << mCol;
    string strRow = s1.str();
    string strCol = s2.str();
    string s = "[Row]" + strRow + " [Column]" + strCol + " [Type]" + typeArray[mType] + " [Text]" + mText ;
    return s;
  } // ToString()
};  // class Token

bool Token::IsInt( string s )
{
  int i = 0 ;
  int k = s.length() ;
  while ( i < k )
  {
    char c = s.at( i ) ;
    if ( i == 0 && ( c != '+' && c != '-' && !isdigit( c ) ) ) return false ;
    if ( i == 0 && ( c == '0' ) ) return false ;
    if ( i != 0 && ( !isdigit( c ) || c == '.' ) ) return false ; 
    i ++ ;
  } // while()

  return true ;
} // IsInt()

bool Token::IsFloat( string s )
{
  bool dot = false ;
  int i = 0 ;
  int k = s.length() ;
  while ( i < k )
  {
    char c = s.at( i ) ;
    if ( i == 0 && ( c != '+' && c != '-' && ( c != '.' ) && ( !isdigit( c ) ) ) ) return false ;
    if ( i != 0 && ( ( !isdigit( c ) && ( c != '.') ) || ( c == '.' && dot ) ) ) {
      return false ; 
    } // if()
    if( c == '.' ) dot = true ;
    i ++ ;
  } // while()

  return true ;
} // IsFloat
