// ===標頭檔區===
# include <iostream>
# include <string>
# include <sstream>
# include <ctype.h>

using namespace std ;

// ===列舉、結構區===
string gTokenTypeMap[] = { "LP", "RP", "INT", "FLOAT", "STRING", "DOT",
                           "NIL", "T", "QUOTE", "SYMBOL" } ;
enum TokenType
{
  LP, RP, INT, FLOAT, STRING, DOT, NIL, T, QUOTE, SYMBOL
};

// ===全域變數區===
bool gSyntaxErrorFlag = false ;
bool gEndOfFileFlag = false ;
string gErrorMessage = "" ;

// =====Token Class=====
class Token {

private:
 int mRow ;
 int mCol ;
 string mText ;
 TokenType mType ;

public:
  // 建構子：初始化
  Token() {
    mRow = -1, mCol = -1 ;
    mText = "" ;
  } // Token()
  // 建構子：直接帶入properties
  Token( int r, int c, string str, TokenType t ) {
    mRow = r, mCol = c ;
    mText = str ;
    mType = t ;
  } // Token()
  // 將資料轉成string 方便測試
  string ToString() {
    stringstream s1, s2 ;
    s1 << mRow ;
    s2 << mCol ;
    string result = "[Row]" + s1.str() + " [Col]" + s2.str() +
                    " [Token]" + mText + " [Type]" + gTokenTypeMap[ mType ] ;

    return result ;
  } // ToString()
  // 回傳Token Type
  TokenType GetType() {
    return mType ;
  } // GetType()
  // 回傳Token String
  string GetText() {
    return mText ;
  } // GetText()
  // 以string回傳行列資訊
  string GetPosInfoAsString() {
    stringstream s1, s2 ;
    s1 << mRow ;
    s2 << mCol ;
    string result = "Line " + s1.str() + " Column " + s2.str() ;
    return result ;
  } // GetPosInfoAsString()

}; // class Token
// =====TreeNode Class=====
class TreeNode {

}; // class TreeNode

// =====Lexer Class=====
class Lexer {

private:
  int mColCounter ;
  int mRowcounter ;
  // Return true while given char is a SEPARATOR.
  bool IsSep( char c ) {
    if ( c == '(' || c == ')' || c == '\"' || c == '\'' ||
         c == ';' || isspace( c ) )
      return true ;
    else
      return false ;
  } // IsSep()
  // Input reader and counter-incrementer.
  char GetChar() {
    int c = cin.get() ;
    // EOF的狀況視為一個錯誤
    if ( c == -1 )
      cout << "[Error]GetChar(): Read a EOF..." << endl ;

    char result = ( char ) c ;
    // Deal with Counter
    if ( result == '\n' ) {
      mColCounter = 0 ;
      mRowcounter += 1 ;
    } // if()
    else 
      mColCounter += 1 ;

    return result ;
  } // GetChar()
  // Deal with String Token
  string StringHelper() {
    
    string tokenString = "" ;
    // 1. 讀入前雙引號
    char c = GetChar() ;
    tokenString += c ;
    if ( c != '\"' )
      cout << "[Error]StringHelper(): First character should be a double-quote..." << endl ;
    // 2. 讀入字串內容 並處理escape字元
    while ( cin.peek() != '\"' && cin.peek() != -1 )
    {
      c = GetChar() ;

      // string not closed error
      if ( c == '\n' ) {
        gSyntaxErrorFlag = true ;
        return "" ;
      } // if()

      // 讀掉並重組escape字元
      if ( c == '\\' && cin.peek() == '\\' ) {
        GetChar() ;
        c = '\\' ;
      } // if()
      else if ( c == '\\' && cin.peek() == 't' ) {
        GetChar() ;
        c = '\t' ;
      } // else if()
      else if ( c == '\\' && cin.peek() == 'n' ) {
        GetChar() ;
        c = '\n' ;
      } // else if()
      else if ( c == '\\' && cin.peek() == '\"' ) {
        GetChar() ;
        c = '\"' ;
      } // else if()
      else ;

      tokenString += c ;
    } // while()
    
    // 3. 讀入後雙引號
    if ( cin.peek() == -1 ) {
      cout << "[Error]StringHelper(): EOF encounterred..." << endl ;
      gSyntaxErrorFlag = true ;
      return "" ;
    } // if()

    c = GetChar() ;
    if ( c != '\"' ) cout << "[Error]StringHelper(): String without end-qoutoe..." << endl ;
    tokenString += c ;
    return tokenString ;
  } // StringHelper()
  // Check type of given string.
  TokenType CheckType( string str ) {
    // LP
    if ( str == "(" ) return LP ;
    // RP
    else if ( str == ")" ) return RP ;
    // DOT
    else if ( str == "." ) return DOT ;
    // Quote
    else if ( str == "\'" ) return QUOTE ;
    // String
    else if ( str[ 0 ] == '\"' && str[ str.length() ] == '\"' ) return STRING ;
    // T
    else if ( str == "t" || str == "#t" ) return T ;
    // NIL
    else if ( str == "nil" || str == "#f" ) return NIL ;
    // INT
    else if ( IsInt( str ) ) return INT ;
    // FLOAT
    else if ( IsFloat( str ) ) return FLOAT ;
    // SYNBOL
    else return SYMBOL ;
    
  } // CheckType()
  // Check if a string is INT type.
  bool IsInt( string str ) {

    if ( str.length() > 1 && ( str[ 0 ] == '+' || str[ 0 ] == '-' ) ) {
      for ( int i = 1 ; i < str.length() ; i++ ) {
        if ( isdigit( str.at( i ) ) == 0 )
          return false ;
      } // for()
      return true ;
    } // if()

    else {
      for ( int i = 0 ; i < str.length() ; i++ ) {
        if ( isdigit( str.at( i ) ) == 0 )
          return false ;
      } // for()
      return true ;
    } // else
  } // IsInt()

  // Check if a string is FLOAT type.
  bool IsFloat( string str ) {
    // +1.123 +1. +.123
    // 1.123
    // .123
    int dotCounter = 0 ;

    if ( str == "+." || str == "-." || str == "." ) return false ;

    if ( str.length() > 1 && ( str[ 0 ] == '+' || str[ 0 ] == '-' ) ) {
      // Iterate digital part ( maybe with one dot ).
      for ( int i = 1 ; i < str.length() ; i++ ) {
        // 遇到非digit
        if ( isdigit( str.at( i ) ) == 0 ) {
          // 非digit只能是dot，且只能有一個
          if ( str.at( i ) == '.' ) {
            dotCounter += 1 ;
            if ( dotCounter > 1 ) return false ;
          } // if()
          else
            return false ;
        } // if()
      } // for()

    } // if()

    // Iterate the whole string ( no sign ).
    else {
      for ( int i = 0 ; i < str.length() ; i++ ) {
        if ( isdigit( str.at( i ) ) == 0 ) {

          if ( str.at( i ) == '.' ) {
            dotCounter += 1 ;
            if ( dotCounter > 1 ) return false ;
          } // if()
          else return false ;
          
        } // if()
      } // for()
    }

    if ( dotCounter != 1 ) return false ;
    else return true ;

  } // IsFloat()

public:
  // 建構子：初始化
  Lexer() {
    mColCounter = 0 ;
    mRowcounter = 1 ;
  } // Lexer()

  // 回傳下一個Token
  // 回傳NULL, 當遇到EOF或StringNotClosed Error.
  Token* NextToken() {
    
    string tokenString = "" ;
    
    // 1. 跳過所有空格 直到碰到“非空格”或“EOF”
    while ( isspace( cin.peek() ) ) {
      GetChar() ;
    } // while()

    // 2.1 如果該非spaces是EOF則回傳NULL
    if ( cin.peek() == -1 ) { 
      gEndOfFileFlag = true ;
      return NULL ;
    } // if()

    // 2.2 如果該非spaces是SEPARATOR
    if ( IsSep( cin.peek() ) ) {
      // '(' or ')' or single-quite
      if ( cin.peek() == '(' || cin.peek() == ')' || cin.peek() == '\'' ) {
        char c  = GetChar() ;
        tokenString += c ;
        if ( tokenString == "(" )
          return new Token( mRowcounter, mColCounter, tokenString, LP ) ;
        else if ( tokenString == ")" )
          return new Token( mRowcounter, mColCounter, tokenString, RP ) ;
        else if ( tokenString == "\'" )
          return new Token( mRowcounter, mColCounter, tokenString, QUOTE ) ;
        else
          cout << "[Error]NextToken(): This case shouldn't encounterred..." << endl ;          
      } // if()
      // double-quote ( starts a string )
      else if ( cin.peek() == '\"' ) {
        int col = mColCounter + 1 ;
        int line = mRowcounter ;
        tokenString = StringHelper() ;
        // if string not closed, do not return a token.
        if ( gSyntaxErrorFlag )
          return NULL ;
        else
          return new Token( line, col, tokenString, STRING ) ;
      } // else if()
      // semicolon ( starts a line-comment )
      else if ( cin.peek() == ';' ) {
        while ( cin.peek() != '\n' && cin.peek() != -1 )
          GetChar() ;
        // 如果遇到EOF 不要讀掉
        return NextToken() ;
      } // else if()
      // spaces ( should not encounterred )
      else
        cout << "[Error]NextToken(): Spaces shouldn't encointerred..." << endl ;
    } // if()

    // 3. 從此非space字元讀到下一個SEPARATOR 確認Type
    else {
      int col = mColCounter + 1 ;
      int line = mRowcounter ;
      // Concat all the input char.
      while ( ! IsSep( cin.peek() ) && cin.peek() != -1 ) {
        char c = GetChar() ;
        tokenString += c ;
      } // while()
      // check type
      TokenType t =  CheckType( tokenString ) ;
      // return
      return new Token( line, col, tokenString, t ) ;
    } //  else

    cout << "[Error]NextToken(): Return a init token..." << endl ;
    return new Token() ;
  } // Token()

}; // class Lexer

// =====Parser Class=====

// =======測試程式-Lexer=======
int TestBench1() {
  Lexer* lx = new Lexer() ;
  Token* token = new Token() ;
  bool end = false ;
  while ( ! end ) {
    token = lx->NextToken() ;
    if ( gSyntaxErrorFlag ) {
      cout << endl << "String not closed..." << endl ;
      gSyntaxErrorFlag = false ;
    } // if()
    else if ( gEndOfFileFlag ) {
      cout << endl << "End of File..." << endl ;
      end = true ;
    } // else if()
    else
      cout << endl << token->ToString() << endl ;

  } // while()

  cout << "End of Test Bench 1 ..." << endl ;
  return 0 ;
} // TestBench1()

// =======主程式=======

int main()
{
  cout << "Welecome!" << endl ;
  // test bench 1
  TestBench1() ;

} // main()

