#include <iostream>
#include <string>
#include "scanner.h"

using namespace std;

int main()
{
  Lexer *lx = new Lexer() ;
  Token *r  ;
  while( ( r = lx->GetToken()) != NULL )
  {
    cout << r->ToString() << endl ;
  } //while()
} // main()
