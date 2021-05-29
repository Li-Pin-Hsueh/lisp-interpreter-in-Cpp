#include <iostream>
#include <string>
#include "scanner.h"

using namespace std;

int main()
{
  Lexer *lx = new Lexer() ;
  Token *r  ;
  bool end = false ;
  while( !end )
  {
    r = lx->GetToken() ;
    if ( r == NULL ) end = true ;
    else
      cout << r->ToString() << endl ;
  } //while()
} // main()
