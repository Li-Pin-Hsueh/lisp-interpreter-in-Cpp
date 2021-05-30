# include <iostream>
# include <string>
# include <sstream>
# include <ctype.h>

using namespace std;

// ============================================== 
// Global Variables
bool gProgramEnd = false ;
bool gProgramReset = false ;
string gErrorMessage = "" ;

enum TokenType
{
  LP, RP, INT, FLOAT, STRING, DOT, NIL, T, QUOTE, SYMBOL
};
// ============================================== 
// Token class
class Token
{
private:
  int mRow ;
  int mCol ;
  string mText ;
  TokenType mType ;
  bool IsInt( string s )
  {
    int i = 0 ;
    int k = s.length() ;
    while ( i < k )
    {
      char c = s.at( i ) ;
      if ( i == 0 && ( c != '+' && c != '-' && !isdigit( c ) ) ) return false ;
      // if ( i == 0 && ( c == '0' ) ) return false ;
      if ( i != 0 && ( !isdigit( c ) || c == '.' ) ) return false ; 
      i ++ ;
    } // while()

    return true ;
  } // IsInt()
  
  bool IsFloat( string s )
  {
    bool dot = false ;
    int i = 0 ;
    int k = s.length() ;
    while ( i < k )
    {
      char c = s.at( i ) ;
      if ( i == 0 && ( c != '+' && c != '-' && ( c != '.' ) && ( !isdigit( c ) ) ) ) return false ;
      if ( i != 0 && ( ( !isdigit( c ) && ( c != '.' ) ) || ( c == '.' && dot ) ) ) {
        return false ; 
      } // if()

      if ( c == '.' ) dot = true ;
      i ++ ;
    } // while()

    return true ;
  } // IsFloat()

public:
  Token( int r, int c, string v )
  {

    TokenType t ;
    if ( v == "(" ) t = LP ;
    else if ( v == ")" ) t = RP ;
    else if ( v == "." ) t = DOT ;
    else if ( v == "'" ) t = QUOTE ;
    else if ( v == "nil" || v == "#f" ) v = "nil", t = NIL ;
    else if ( v == "t" || v == "#t" ) v = "#t", t = T ;
    else if ( IsInt( v ) ) t = INT ;
    else if ( IsFloat( v ) ) t = FLOAT ;
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
    string s = "[Line]" + strRow + " [Column]" + strCol + " [Type]" + typeArray[mType] + " [Text]" + mText ;
    return s;
  } // ToString()
};  // class Token

// ============================================== 
// Lexer class
class Lexer
{
private:
  int mColCounter ;
  int mRowCounter ;
  bool IsSep( char c )
  {
    if ( c == '(' || c == ')' || c == '\"' || c == '\'' ||
         c == ';' || isspace( c ) )
      return true ;
    else
      return false ;
  } // IsSep()

  char GetChar()
  {
    int c = cin.get() ;
    // EOF encounterred.
    if ( c == -1 )
      return '\0' ;
    
    char result = ( char ) c ;
    // Increment counter.
    if ( result == '\n' )
    {
      mColCounter = 0 ;
      mRowCounter += 1 ;
    } // if
    else
      mColCounter += 1 ;

    return result ;
  } // GetChar()

  string ToString( char c )
  {
    stringstream s ;
    s << c ;
    return s.str() ;
  } // ToString()

  string StringHelper()
  {
    // String not close should be a ERROR.
    GetChar() ;  // first double-quote.
    string result = "\"" ;
    bool end = false ;
    while ( !end && cin.peek() != '\n' ) {
      char c = GetChar() ;
      // Escape character case.
      if ( c == '\\' && ( cin.peek() == 'n' || cin.peek() == 't' ||
                          cin.peek() == '\"' || cin.peek() == '\\' ) ) {
        if ( c == '\\' ) result += "\\" ;
        else if ( c == '\"' ) result += "\"" ;
        else if ( c == 'n' ) result += "\n" ;
        else if ( c == 't' ) result += "\t" ;
        else cout << "StringHelper ERROR: Else case..." << endl ;
        c = GetChar() ;  // Read \, ", n or t
      } // if()
      // Normal Case.
      else {
        if ( c == '\n' ) end = true ;
        if ( c == '\"' ) end = true ;
        stringstream ss ;
        ss << c ;
        result += ss.str() ;
      } // else

    } // while
    
    // Set String Error here.
    if ( result.at( result.length()-1 ) != '\"' ) {
      // 注意: 換行字元還沒讀掉
      // Important: new line has not been read.
      stringstream s1, s2 ;
      s1 << mRowCounter, s2 << mColCounter + 1 ;
      gProgramReset = true ;
      gErrorMessage = "ERROR (no closing quote) : END-OF-LINE encountered at line " + s1.str() +
                      ", column " + s2.str() ;
      // 讀掉換行字元, 重設Counter.
      GetChar() ;
      ResetCounter() ;

    } // if()
    
    return result ;
  } // StringHelper()

public:
  Lexer()
  {
    mColCounter = 0 ;
    mRowCounter = 1 ;
  } // Lexer()

  Token *GetToken()
  {
    string tokenString = "" ;
    int startCol = -1 ;

    // 1. Skipwhitespaces
    while ( isspace( cin.peek() ) || cin.peek() == -1 ) {
      char c = GetChar() ;
      if ( c == '\0' ) {
        cout << "EOF..." << endl ;
        return NULL ;
      } // if()
    } // while()

    // 2. Read until Separator.
    startCol = mColCounter+1 ;
    while ( !IsSep( cin.peek() ) ) {
      // Contact with tokenString.
      char c = GetChar() ;
      stringstream ss ;
      if ( c == '\0' ) {
        cout << "GetToken() ERROR : This shouldn't encouterred." << endl ;
        return NULL ;
      } // if()
      // Convert to string and contact.
      ss << c ;
      tokenString += ss.str() ;
    } // while()

    // 3. Return Token.
    // 3.1 Return tokenString if it's not empty.
    if ( !tokenString.empty() )
    {
      return new Token( mRowCounter, startCol, tokenString ) ;
    } // if()
    // 3.2 Return this SEP if it's a Token (, ), '
    else
    {
      if ( cin.peek() == '(' || cin.peek() == ')' || cin.peek() == '\'' )
      {
        stringstream ss ;
        ss << GetChar() ;
        startCol = mColCounter ;
        return new Token( mRowCounter, startCol, ss.str() ) ;
      } // if()
      else if ( cin.peek() == '"' )
      {
        startCol = mColCounter + 1 ;
        tokenString = StringHelper() ;
        return new Token( mRowCounter, startCol, tokenString, STRING ) ;
      } // else if()
      else if ( cin.peek() == ';' )
      {
        // Read all the comment line.
        while ( cin.peek() != '\n' ) GetChar() ;
        GetChar() ; // Read new line character.
        // Return next Token.
        return GetToken() ;
      } // else if()
      else
        cout << "Error... This should not encounterred..." << endl ;
    } // else()
    
    cout << "Executed to the end..." << endl ;
    return NULL ;
  } // GetToken()

  void ResetCounter()
  {
    mColCounter = 0 ;
    mRowCounter = 1 ;
  } // ResetCounter()

}; // class Lexer

// ============================================== 
// Parser class

// ============================================== 
// Main function

int main()
{
  gProgramEnd = false ;
  gProgramReset = false ;
  gErrorMessage = "" ;
  Lexer *lx = new Lexer() ;
  Token *r  ;

  while ( !gProgramEnd )
  {
    r = lx->GetToken() ;
    if ( r == NULL ) gProgramEnd = true ;
    else if ( gProgramReset ) {
      // Reset Lexer and flag
      gProgramReset = false ;
      lx = new Lexer() ;
      cout << gErrorMessage << endl ;
    } // else if()
    else
      cout << r->ToString() << endl ;
  } // while()
} // main()
