#include <iostream>
#include <string>
#include "scanner.h"

using namespace std;

int main()
{
  
  Token *x = new Token( 1, 1, "Fuck", STRING ) ;
  cout << x->ToString();
} // main()
