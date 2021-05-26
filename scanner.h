# include <iostream>
# include <string>
# include "token.h"
using namespace std ;

// Scanner負責回傳Token串, 紀錄column, row
class Scanner {
private:
    int colCounter ;
    int rowCounter ;

public:
    void printColCounter() ;
    Token* getToken() ;
}; // class Scanner

void Scanner::printColCounter() {
    cout << this->colCounter << "end" << endl ;
}

Token* Scanner::getToken() {
    Token *x = new Token(1, 1, "a string", STRING);
    return x ;
}