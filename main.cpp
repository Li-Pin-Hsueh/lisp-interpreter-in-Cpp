# include <iostream>
# include <string>
# include <sstream>
# include <ctype.h>
# include <vector>
using namespace std;

// ============================================== 
// Global Variables
bool gExitFlag = false ;
bool gStringNotColsedError = false ;
bool gSyntaxError = false ;
bool gEndOfFile = false ;
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
// Deal with : [ String not closed Error, EOF encounterred ]
class Lexer
{
private:
  int mColCounter ;
  int mRowCounter ;
  Token *mCurrentToken ;
  /* Return true if the argument char is a Separator. */
  bool IsSep( char c )
  {
    if ( c == '(' || c == ')' || c == '\"' || c == '\'' ||
         c == ';' || isspace( c ) )
      return true ;
    else
      return false ;
  } // IsSep()
  /* Use cin.get() to read input. Do counter increment. */
  char GetChar()
  {
    int c = cin.get() ;
    // EOF encounterred.
    if ( c == -1 ) {
      // gEndOfFile = true ;
      return '\0' ;
    } // if

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
  /* Convert a Char to string type */
  string ToString( char c )
  {
    stringstream s ;
    s << c ;
    return s.str() ;
  } // ToString()
  /* Return a token string(as string type). Assign error message(String-Not-Closed-Error).*/
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

        c = GetChar() ;  // Read \, ", n or t
        if ( c == '\\' ) result += "\\" ;
        else if ( c == '\"' ) result += "\"" ;
        else if ( c == 'n' ) result += "\n" ;
        else if ( c == 't' ) result += "\t" ;
        else cout << "StringHelper ERROR: Else case..." << endl ;
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
      gSyntaxError = true ;
      gErrorMessage = "ERROR (no closing quote) : END-OF-LINE encountered at line " + s1.str() +
                      ", column " + s2.str() ;
      // 讀掉換行字元, 重設Counter.
      GetChar() ;
      ResetCounter() ;

    } // if()
    
    return result ;
  } // StringHelper()
  /* Return a Token or NULL(EOF) */
  Token *GetToken()
  {
    string tokenString = "" ;
    int startCol = -1 ;

    // 1. Skipwhitespaces
    while ( isspace( cin.peek() ) || cin.peek() == -1 ) {
      char c = GetChar() ;
      // EOF
      if ( c == '\0' ) return NULL ;
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
public:
  Lexer()
  {
    mColCounter = 0 ;
    mRowCounter = 1 ;
    mCurrentToken = NULL ;
  } // Lexer()
  /* Reset line and column counters 1 and 0.*/
  void ResetCounter()
  {
    mColCounter = 0 ;
    mRowCounter = 1 ;
  } // ResetCounter()
  /* Return current Token or next Token() */
  Token *NextToken()
  {
    if ( mCurrentToken == NULL )
      mCurrentToken = GetToken() ;

    Token *t = mCurrentToken ;
    mCurrentToken = NULL ;
    if ( t == NULL )  gEndOfFile = true ;
    return t ;
  } // NextToken()
  /* Return current Token */
  Token *PeekToken()
  {
    if ( mCurrentToken == NULL ) mCurrentToken = GetToken() ;
    return mCurrentToken ;
  }
}; // class Lexer

// ============================================== 
// Parser class
// Deal with: [ Syntax Error ]
class Parser
{
private:
  Lexer *mLexer ;
  vector<Token> mTokens ;

public:
  Parser()
  {
    mLexer = new Lexer() ;
  } // Parser()
  /* Return true if no Syntax-error.*/
  bool ReadSExp()
  {
    // TODO
    return false ;

  } // ReadSExp()

  /* Return the Vectoe which store tokens.*/
  vector<Token> GetTokensCopy()
  {
    vector<Token> copy ;
    for ( int i = 0 ; i < mTokens.size() ; i++ )
    {
      copy.push_back( mTokens.at( i ) ) ;
    } // for

    return copy ;
  } // GetTokensCopy()
  /* Iterate vector and print the content. */
  void PrintTokens()
  {
    for ( int i = 0 ; i < mTokens.size() ; i++ )
    {
      cout << mTokens.at( i ).ToString() << endl ;
    } // for
  } // PrintTokens()

}; // class Parser

// ============================================== 
// Main function

int main()
{

  Parser *p = new Parser() ;
  cout << "Welcome to OurScheme!\n" ;

  while ( !gExitFlag && !gEndOfFile )
  {
    cout << "> " ;
    bool noSyntaxError = p->ReadSExp() ;

    if ( noSyntaxError ) { // S-Exp成立
      p->PrintTokens() ;
    } // if()
    else {
      // Print error message.
      cout << gErrorMessage << endl ;
      // Reset error flag.
      gSyntaxError = false, gStringNotColsedError = false ;
    }

  } // while()

  if ( gEndOfFile )
    cout << "Error (no more input) : END-OF-FILE encountered" << endl ;

  cout << endl << "Thanks for using OurScheme!" << endl ;
} // main()
