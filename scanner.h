#include <iostream>
#include <string>
#include "token.h"
using namespace std;

// Scanner負責回傳Token串, 紀錄column, row
class Scanner
{
private:
  int mColCounter ;
  int mRowCounter ;
  bool IsSep( char c ) ;
  char GetChar() ;
  string ToString( char c ) ;

public:
  Token *GetToken() ;
}; // class Scanner

Token *Scanner::GetToken()
{
  // TODO
  // 1. Skipwhitespaces
  // 2. Read until Separator.
  // 2.1 Return if the Separator is Token itself.
  // 2.2 Contact Token-String if Not a Token.

  Token *x = new Token( 1, 1, "a", STRING ) ;
  return x;
} // GetToken()

bool Scanner::IsSep( char c )
{
  if ( c == '(' || c == ')' || c == '\"' || c == '\'' || c == ';' || isspace( c ) )
    return true;
  else
    return false;
} // IsSep()

char Scanner::GetChar()
{
  int c = cin.get() ;
  // EOF encounterred.
  if( c == -1 )
    return '\0' ;
  
  char result = (char)c ;
  // Increment counter.
  if( result == '\n' )
  {
    mColCounter = 0 ;
    mRowCounter += 1 ;
  } // if
  else
    mColCounter += 1 ;

  return result ;
} // GetChar()

string Scanner::ToString( char c )
{
  stringstream s ;
  s << c ;
  return s.str() ;
} // ToString()