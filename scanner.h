#include <iostream>
#include <string>
#include "token.h"
using namespace std;

// Scanner負責回傳Token串, 紀錄column, row
class Scanner
{
private:
    int colCounter;
    int rowCounter;

public:
    Token *getToken();
}; // class Scanner

Token *Scanner::getToken()
{
    Token *x = new Token(1, 1, "a string", STRING);
    return x;
} // getToken()