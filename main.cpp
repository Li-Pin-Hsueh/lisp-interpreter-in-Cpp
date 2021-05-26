# include <iostream>
# include <string>
# include "scanner.h"

using namespace std;

int main()
{
  Scanner *sc = new Scanner() ;
  Token* x = sc->getToken() ;
  cout << x->value << endl ;
} // main()

