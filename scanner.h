#include <iostream>
#include <string>
#include "token.h"
using namespace std;

// Lexer負責回傳Token串, 紀錄column, row
class Lexer
{
private:
  int mColCounter ;
  int mRowCounter ;
  bool IsSep( char c ) ;
  char GetChar() ;
  string ToString( char c ) ;
  string StringHelper() ;

public:
  Token *GetToken() ;
}; // class Lexer

Token *Lexer::GetToken()
{
  string tokenString = "" ;
  int startCol = -1 ;

  // 1. Skipwhitespaces
  while( isspace( cin.peek() ) || cin.peek() == -1) {
    char c = GetChar() ;
    if( c == '\0' ) {
      cout << "EOF..." << endl ;
      return NULL ;
    }
  } // while()

  // 2. Read until Separator.
  startCol = mColCounter+1 ;
  while( !IsSep( cin.peek() ) ) {
    // Contact with tokenString.
    char c = GetChar() ;
    stringstream ss ;
    if( c == '\0' ){
      cout << "GetToken() ERROR : This shouldn't encouterred." << endl ;
      return NULL ;
    }
    // Convert to string and contact.
    ss << c ;
    tokenString += ss.str() ;
  } // while()

  // 3. Return Token.
  // 3.1 Return tokenString if it's not empty.
  if( !tokenString.empty() )
  {
    return new Token( mRowCounter, startCol, tokenString, STRING ) ;
  } // if()
  // 3.2 Return this SEP if it's a Token (, ), '
  else
  {
    if( cin.peek() == '(' || cin.peek() == ')' || cin.peek() == '\'' )
    {
      stringstream ss ;
      ss << GetChar() ;
      startCol = mColCounter ;
      return new Token( mRowCounter, startCol, ss.str(), LP ) ;
    } // if()
    else if( cin.peek() == '"' )
    {
      startCol = mColCounter + 1 ;
      tokenString = StringHelper() ;
      return new Token( mRowCounter, startCol, tokenString, STRING ) ;
    } // else if()
    else if( cin.peek() == ';')
    {
      // Read all the comment line.
      while( cin.peek() != '\n') GetChar() ;
      GetChar() ; // Read new line character.
      // Return next Token.
      return GetToken() ;
    }
    else
      cout << "Error... This should not encounterred..." << endl ;
  }
  
  cout << "Executed to the end..." << endl ;
  return NULL ;
} // GetToken()

bool Lexer::IsSep( char c )
{
  if ( c == '(' || c == ')' || c == '\"' || c == '\'' || c == ';' || isspace( c ) )
    return true;
  else
    return false;
} // IsSep()

char Lexer::GetChar()
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

string Lexer::ToString( char c )
{
  stringstream s ;
  s << c ;
  return s.str() ;
} // ToString()

string Lexer::StringHelper()
{
  // String not close should be a ERROR.
  GetChar() ;  // first double-quote.
  string result = "\"" ;
  bool end = false ;
  while( !end && cin.peek() != '\n' ) {
    char c = GetChar() ;
    // Escape character case.
    if( c == '\\' && ( cin.peek() == 'n' || cin.peek() == 't' || cin.peek() == '\"' || cin.peek() == '\\' )) {
      c = GetChar() ;
      if( c == '\\' ) result += "\\" ;
      else if( c == '\"' ) result += "\"" ;
      else if( c == 'n' ) result += "\n" ;
      else if( c == 't' ) result += "\t" ;
      else cout << "StringHelper ERROR: Else case..." << endl ;
    } // if()
    // Normal Case.
    else {
      if( c == '\n' ) end = true ;
      if( c == '\"') end = true ;
      stringstream ss ;
      ss << c ;
      result += ss.str() ;
    } // else

  } // while

  if( result.at( result.length()-1 ) == '\n' )
    cout << "StringHelper ERROR: Not Colses..." << endl ;
  return result ;
}