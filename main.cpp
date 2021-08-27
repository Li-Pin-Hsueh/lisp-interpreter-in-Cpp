// ===標頭檔區===
# include <stdio.h>
# include <stdlib.h>
# include <iostream>
# include <string>
# include <sstream>
# include <ctype.h>
# include <vector>
# include <map>
using namespace std ;

# define CMDNUM 41
// ===列舉、結構區===
string gTokenTypeMap[] = { "INIT_TYPE", "LP", "RP", "INT", "FLOAT", "STRING", "DOT",
                           "NIL", "T", "QUOTE", "SYMBOL" } ;
enum TokenType
{
  INIT_TYPE, LP, RP, INT, FLOAT, STRING, DOT, NIL, T, QUOTE, SYMBOL
};

string gNodeTypeMap[] = { "CONS-NODE", "ATOM-NODE" } ;
enum NodeType
{
  CONS_NODE, ATOM_NODE
};

string gSystemCmd[ CMDNUM ] = 
{
  "cons", "list", "quote", "define", "car", "cdr","atom?", "pair?", "list?", "null?",
  "integer?", "real?", "number?", "string?", "boolean?", "symbol?", "+", "-", "*", "/",
  "not", "and", "or", ">", ">=", "<", "<=", "=", "string-append", "string>?",
  "string<?", "string=?", "eqv?", "equal?", "begin", "if", "cond", "clean-environment", "exit",
  "let", "lambda"
} ;

enum ErrorType {
  EMPTY, UNBND_SYM, NON_LIST, APLY_NON_FUN, LEVL, NUM_OF_ARGS, WRONG_ARG_TYPE,
  DIV_ZERO, NO_RETURN_VAL, FORMAT_ERR
};
// ===全域變數區===
bool gSyntaxErrorFlag = false ;
bool gEndOfFileFlag = false ;
bool gEndProgramFlag = false ;
string gErrorMessage = "" ;

class Token ;
class Lexer ;
class TreeNode ;
class Parser ;
class EnvFunction ;
class Environment ;
class OurSchemeException ;

void PrettyPrinter( TreeNode* current, bool expr, int spaces ) ;
void Printer( TreeNode* expr ) ;

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
// =====Lexer Class=====
class Lexer {

private:
  int mColCounter ;
  int mRowcounter ;
  Token* mCurrentToken ;
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

      // string not closed error
      if ( cin.peek() == '\n' ) {
        //  先取得正確line, col 再把換行讀掉
        stringstream s1, s2 ;
        s1 << mRowcounter, s2 << mColCounter+1 ;
        // 已經讀掉 不需要clear
        GetChar() ;
        // 設定Erroe Message
        gSyntaxErrorFlag = true ;
        
        gErrorMessage = "ERROR (no closing quote) : END-OF-LINE encountered at Line " + s1.str() ;
        gErrorMessage +=  " Column " + s2.str() ;
        
        return "" ;
      } // if()

      c = GetChar() ;
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
      gEndOfFileFlag = true ;
      return "" ;
    } // if()

    c = GetChar() ;
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
    } // else

    if ( dotCounter != 1 ) return false ;
    else return true ;

  } // IsFloat()
  // 讀進下一個Token
  // 將currentToken設為NULL, 當讀到EOF或StringNotClosedError
  void ReadToken() {
    
    string tokenString = "" ;
    
    // 1. 跳過所有空格 直到碰到“非空格”或“EOF”
    while ( isspace( cin.peek() ) ) {
      GetChar() ;
    } // while()

    // 2.1 如果該非spaces是EOF則currentToken設為NULL，並設定EOF旗標
    if ( cin.peek() == -1 ) { 
      gEndOfFileFlag = true ;
      mCurrentToken = NULL ; 
      return ;
    } // if()

    // 2.2 如果該非spaces是SEPARATOR
    if ( IsSep( cin.peek() ) ) {
      // LP, RP, Quote 本身就是Token
      if ( cin.peek() == '(' || cin.peek() == ')' || cin.peek() == '\'' ) {
        char c  = GetChar() ;
        tokenString += c ;
        if ( tokenString == "(" )
          mCurrentToken = new Token( mRowcounter, mColCounter, tokenString, LP ) ;
        else if ( tokenString == ")" )
          mCurrentToken = new Token( mRowcounter, mColCounter, tokenString, RP ) ;
        else if ( tokenString == "\'" ) 
          mCurrentToken = new Token( mRowcounter, mColCounter, tokenString, QUOTE ) ;
        else
          cout << "[Error]NextToken(): This case shouldn't encounterred..." << endl ; 
        
        return ;           
      } // if()
      // 雙引號
      else if ( cin.peek() == '\"' ) {
        int col = mColCounter + 1 ;
        int line = mRowcounter ;
        tokenString = StringHelper() ;
        // if string not closed, set NULL.
        if ( gSyntaxErrorFlag )
          mCurrentToken = NULL ;
        else {
          mCurrentToken = new Token( line, col, tokenString, STRING ) ;
        } // else

        return ;
      } // else if()
      // semicolon ( starts a line-comment )
      else if ( cin.peek() == ';' ) {
        while ( cin.peek() != '\n' && cin.peek() != -1 )
          GetChar() ;
        // 如果遇到EOF或換行 不要讀掉
        ReadToken() ;
        return ;
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
      if ( t == NIL )
        tokenString = "nil" ;
      else if ( t == T )
        tokenString = "#t" ;

      mCurrentToken = new Token( line, col, tokenString, t ) ;
      return ;
    } // else

    cout << "[Error]NextToken(): Return a init token..." << endl ;
    return ;
  } // ReadToken()

public:
  // 建構子：初始化
  Lexer() {
    mColCounter = 0 ;
    mRowcounter = 1 ;
    mCurrentToken = NULL ;
  } // Lexer()

  Token* PeekToken() {
    if ( mCurrentToken == NULL && ! ( gSyntaxErrorFlag || gEndOfFileFlag ) )
      ReadToken() ;

    return mCurrentToken ;
  } // PeekToken()

  Token* GetToken() {
    Token* next = mCurrentToken ;
    mCurrentToken = NULL ;

    return next ;
  } // GetToken()
  // 清除syntax error後整行
  void ClearLine() {
    while ( cin.peek() != '\n' && cin.peek() != -1 ) {
      cin.get() ;
    } // while()

    if ( cin.peek() == '\n' )
      cin.get() ;
    else
      ;

    return ;

  } // ClearLine()
  // 清除Token該行的spaces
  void ClearSpaces() {
    while ( isspace( cin.peek() ) && ! ( cin.peek() == '\n' ) )
    {
      GetChar() ;
    } // while
    
    // 只有註解
    if ( cin.peek() == ';' ) {
      while ( cin.peek() != '\n' && cin.peek() != -1 ) {
        GetChar() ;
      } // while

    } // if()

    // 只有空格
    if ( cin.peek() == '\n' ) {
      GetChar() ;
      mColCounter = 0 ;
      mRowcounter = 1 ;
      
    } // if()

    return ;
  } // ClearSpaces()

}; // class Lexer
// =====TreeNode Class=====
// TODO
class TreeNode {
private:
  NodeType mNodeType ;
  TokenType mTokenType ;
  string mContent ;
  int mIntValue ;
  float mFloatValue ;
  bool mEvalued ;

public:
  TreeNode* mLeft ;
  TreeNode* mRight ;

  TreeNode() {
    mNodeType = CONS_NODE ;
    mTokenType = INIT_TYPE ;
    mEvalued = false ;
    mLeft = NULL ;
    mRight = NULL ;
  } // TreeNode()

  void InitCons() {
    mNodeType = CONS_NODE ;
    mContent = "" ;
    mIntValue = -1 ;
    mFloatValue = -1.0 ;
    mLeft = new TreeNode() ;
    mRight = new TreeNode() ;

    // Set Nil Node
    mRight->InitAtom( "nil", NIL ) ;
  } // InitCons()

  void InitAtom( string s, TokenType type ) {
    // TODO
    mNodeType = ATOM_NODE ;
    mTokenType = type ;
    mContent = s ;
    mLeft = NULL ;
    mRight = NULL ;
    // TODO : set value
    if ( type == INT ) {
      mIntValue = atoi( mContent.c_str() ) ;
    } // if()
    else if ( type == FLOAT ) {
      mFloatValue = atof( mContent.c_str() ) ;
    } // else if()

  } // InitAtom()

  NodeType NodeType() {
    return mNodeType ;
  } // NodeType()

  TokenType TokenType() {
    return mTokenType ;
  } // TokenType()

  string Content() {
    return mContent ;
  } // Content()

  int IntValue() {
    return mIntValue ;
  } // IntValue()

  float FloatValue() {
    return mFloatValue ;
  } // FloatValue()

  void SetEvaled() {
    mEvalued = true ;
  } // SetEvaled()

  bool GetEvalFlag() {
    return mEvalued ;
  } // GetEvalFlag()

}; // class TreeNode
// =====Parser Class=====
class Parser {
private:
  Lexer* mLexer ;
  vector<Token> mTokens ;
  TreeNode* mHeadPtr ;

  // Get Token from Lexer and push into vector
  void Eat() {
    Token* t = mLexer->GetToken() ;
    mTokens.push_back( *t ) ;
    return ;
  } // Eat()
  // Print vector
  void PrintVector()
  {
    int j = mTokens.size() ;
    if ( j == 0 ) return ;

    cout << "Left element..." << endl ;
    for ( int i = 0 ; i < j ; i ++ )
    {
      cout << mTokens.at( i ).ToString() << endl ;
    } // for()
  } // PrintVector()
  // Parse function
  void ParseEXPR( TreeNode* current ) {
    // 設定此node為cons-node
    current->InitCons() ;
    if ( mTokens.size() == 0 ) {
      cout << "ERROR[ParseEXPR]: vector is empty..." << endl ;
      return ;
    } // if()

    // 取得第一個token
    Token cToken = mTokens.at( 0 ) ;
    Token nextToken ;
    // LP RP
    if ( cToken.GetType() == LP && mTokens.at( 1 ).GetType() == RP ) {
      // erase LP, RP
      mTokens.erase( mTokens.begin(), mTokens.begin()+2 ) ;
      current->InitAtom( "nil", NIL ) ;
      return ;
    } // if()

    // LP ... RP
    else if ( cToken.GetType() == LP ) {
      // erase LP
      mTokens.erase( mTokens.begin() ) ;
      ParseEXPR( current->mLeft ) ;
      // try dot
      nextToken = mTokens.at( 0 ) ;
      if ( nextToken.GetType() == DOT ) {
        // erase DOT
        mTokens.erase( mTokens.begin() ) ;
        ParseEXPR( current->mRight ) ;
      } // if()
      else
        ParseCONS( current->mRight ) ;
      // try RP
      nextToken = mTokens.at( 0 ) ;
      if ( nextToken.GetType() != RP )
        cout << "ERROR[ParseEXPR]: RP is missing..." << endl ;
      else
        mTokens.erase( mTokens.begin() ) ;

      return ;
    } // else if()

    // Quote <expr>
    else if ( cToken.GetType() == QUOTE ) {
      // erase quote
      mTokens.erase( mTokens.begin() ) ;
      // Set left a ATOM
      current->mLeft->InitAtom( "quote", QUOTE ) ;
      // Set Right a Cons-node and ParseEXPR to its left.
      current->mRight->InitCons() ;
      ParseEXPR( current->mRight->mLeft ) ;
      return ;
    } // else if()

    // ATOM
    else {
      if ( cToken.GetType() == RP )
        cout << "ERROR[ParseEXPR]: should not be a RP..." << endl ;

      mTokens.erase( mTokens.begin() ) ;
      current->InitAtom( cToken.GetText(), cToken.GetType() ) ;
      return ;
    } // else

  } // ParseEXPR()

  void ParseCONS( TreeNode* current ) {
    current->InitCons() ;
    Token nextToken = mTokens.at( 0 ) ;
    if ( nextToken.GetType() == RP ) {
      current->InitAtom( "nil", NIL ) ;
      return ;
    } // if()
    
    ParseEXPR( current->mLeft ) ;
    // try dot
    nextToken = mTokens.at( 0 ) ;
    if ( nextToken.GetType() == DOT ) {
      // erase DOT
      mTokens.erase( mTokens.begin() ) ;
      ParseEXPR( current->mRight ) ;
    } // if()
    else
      ParseCONS( current->mRight ) ;

    return ;
  } // ParseCONS()
  // 遞迴方式印出樹形
  void SimplePrinter( TreeNode* current ) {
    // TODO
    if ( current->NodeType() == ATOM_NODE ) {
      cout << current->Content() << endl ;
      return ;
    } // if()

    SimplePrinter( current->mLeft ) ;
    SimplePrinter( current->mRight ) ;
  } // SimplePrinter()

public:
  Parser() {
    mLexer = new Lexer() ;
    mTokens.clear() ;
    mHeadPtr = NULL ;
  } // Parser()
  //   PrettyPrinter( mHeadPtr, true, 0 ) ;
  // } // Printer()
  // 根據文法讀入Token，若有誤則設定SyntaxErrorFlag
  void ReadSExp() {
    // TODO
    Token* nextToken = mLexer->PeekToken() ;
    if ( gSyntaxErrorFlag || gEndOfFileFlag ) return ;

      /*
      Grammar
      <S-exp>::= <ATOM> 
              | LEFT-PAREN <S-exp> { <S-exp> } [ DOT <S-exp> ] RIGHT-PAREN
              | QUOTE <S-exp>
      */

    // <ATOM>  ::= SYMBOL | INT | FLOAT | STRING | NIL | T
    TokenType t = nextToken->GetType() ;
    if ( t == SYMBOL || t == INT || t == FLOAT || t == STRING ||
         t == NIL || t == T ) {
      Eat() ;
      return ;
    } // if()
    // QUOTE <S-exp>
    else if ( t == QUOTE ) {
      Eat() ;
      ReadSExp() ;
      return ;
    } // else if()
    // LP <S-exp> { <S-exp> } [ DOT <S-exp> ] RP
    // LP RP
    else {
      // Syntax Error
      if ( t != LP ) {
        gSyntaxErrorFlag = true ;
        mLexer->ClearLine() ;
        // ERROR (unexpected token) : 
        // atom or '(' expected when token at 
        // Line X Column Y is >>...<<
        gErrorMessage = "ERROR (unexpected token) : " ;
        gErrorMessage += "atom or '(' expected when token at " + nextToken->GetPosInfoAsString() ;
        gErrorMessage +=  " is >>" + nextToken->GetText() + "<<" ;
        return ;
      } // if()

      // eat '('
      Eat() ;
      nextToken = mLexer->PeekToken() ;
      if ( nextToken == NULL )  return ;
      // Special case : ()
      if ( nextToken->GetType() == RP ) {
        Eat() ;
        return ;
      } // if()
      // LP <S-exp> { <S-exp> } [ DOT <S-exp> ] RP
      //    ＾
      ReadSExp() ;
      if ( gSyntaxErrorFlag || gEndOfFileFlag ) return ;

      nextToken = mLexer->PeekToken() ;
      if ( nextToken == NULL ) return ;

      while ( nextToken->GetType() != DOT && nextToken->GetType() != RP )
      {
        ReadSExp() ;
        if ( gSyntaxErrorFlag || gEndOfFileFlag ) return ;

        nextToken = mLexer->PeekToken() ;
        if ( nextToken == NULL ) return ;

      } // while()

      if ( nextToken->GetType() == DOT ) {
        Eat() ;
        ReadSExp() ;
        if ( gSyntaxErrorFlag || gEndOfFileFlag ) return ;
      } // if()

      nextToken = mLexer->PeekToken() ;
      if ( nextToken == NULL ) return ;

      if ( nextToken->GetType() == RP ) {
        Eat() ;
        return ;
      } // if()
      else {
        Eat() ;
        gSyntaxErrorFlag = true ;
        mLexer->ClearLine() ;
        gErrorMessage = "ERROR (unexpected token) : ')' expected " ;
        gErrorMessage += "when token at " ;
        gErrorMessage += nextToken->GetPosInfoAsString() ;
        gErrorMessage += " is >>" ;
        gErrorMessage += nextToken->GetText() ;
        gErrorMessage += "<<" ;
        return ;
      } // else

    } // else


  } // ReadSExp()
  // 建樹的啟動點
  TreeNode* Build() {
    if ( mHeadPtr == NULL ) {
      mHeadPtr = new TreeNode() ;
      ParseEXPR( mHeadPtr ) ;
      return mHeadPtr ;
    } // if()
    else {
      cout << "Head pointer is not NULL..." << endl ;
    } // else

    return NULL ;

  } // Build()
  // 檢查EXIT
  bool CheckExit() {
    if ( mTokens.size() == 3 ) {
      if ( mTokens.at( 0 ).GetType() == LP &&
           mTokens.at( 1 ).GetText() == "exit" &&
           mTokens.at( 2 ).GetType() == RP )
        return true ;
    } // if()

    else if ( mTokens.size() == 5 ) {
      if ( mTokens.at( 0 ).GetType() == LP &&
           mTokens.at( 1 ).GetText() == "exit" &&
           mTokens.at( 2 ).GetType() == DOT &&
           mTokens.at( 3 ).GetType() == NIL &&
           mTokens.at( 4 ).GetType() == RP )
        return true ;
    } // else if()

    return false ;

  } // CheckExit()
  // 重置Parser 與 Lexer
  void Reset() {
    delete mLexer ;
    mLexer = new Lexer() ;
    mLexer->ClearSpaces() ;
    mTokens.clear() ;
    mHeadPtr = NULL ;
  } // Reset()

}; // class Parser
// =====OurSchemeException=====
class OurSchemeException {
public:
  ErrorType mErrorType ;
  string mErrorMessage ;
  TreeNode* mExprToPrint ;

  OurSchemeException() {
    mErrorMessage = "shouldn't encounterred..." ;
    mErrorType = EMPTY ;
    mExprToPrint = NULL ;
  } // OurSchemeException()

  OurSchemeException( ErrorType eT, string str ) {
    mErrorType = eT ;
    mExprToPrint = NULL ;

    if ( eT == LEVL ) {
      if ( str == "clean-environment" ) str = "CLEAN-ENVIRONMENT" ;
      else if ( str == "exit" ) str = "EXIT" ;
      else if ( str == "define" ) str = "DEFINE" ;
      else cout << "ERROR[]: line 808...." << endl ;

      mErrorMessage = "ERROR (level of " + str + ")" ;
    } // if()
    else if ( eT == NUM_OF_ARGS )
      mErrorMessage = "ERROR (incorrect number of arguments) : " + str ;
      

  } // OurSchemeException()

  OurSchemeException( ErrorType eT, TreeNode* expr ) {
    mErrorType = eT ;
    mExprToPrint = expr ;
    
    if ( eT == NON_LIST )
      mErrorMessage = "ERROR (non-list) : " ;
    if ( eT == UNBND_SYM )
      mErrorMessage = "ERROR (unbound symbol) : " ;
    else if ( eT == APLY_NON_FUN )
      mErrorMessage = "ERROR (attempt to apply non-function) : " ;
    // else if ( eT == NUM_OF_ARGS )
    //   mErrorMessage = "ERROR (incorrect number of arguments) : " ;
    else if ( eT == DIV_ZERO )
      mErrorMessage = "ERROR (division by zero) : " ;
    else if ( eT == NO_RETURN_VAL )
      mErrorMessage = "ERROR (no return value) : " ;
    
  } // OurSchemeException()

  OurSchemeException( ErrorType eT, TreeNode* expr, TreeNode* cmd ) {
    mErrorType = eT ;
    mExprToPrint = expr ;
    string commandStr = cmd->Content() ;

    if ( eT == WRONG_ARG_TYPE )
      mErrorMessage = "ERROR (" + commandStr + " with incorrect argument type) : " ;
    else if ( eT == FORMAT_ERR && commandStr == "cond" )
      mErrorMessage = "ERROR (COND format) : " ;
    else if ( eT == FORMAT_ERR && commandStr == "define" )
      mErrorMessage = "ERROR (DEFINE format) : " ;
    else if ( eT == FORMAT_ERR && commandStr == "let" )
      mErrorMessage = "ERROR (LET format) : " ;
    else if ( eT == FORMAT_ERR && commandStr == "lambda" )
      mErrorMessage = "ERROR (LAMBDA format) : " ;
    // if ( eT == WRONG_ARG_TYPE && cmd->Content() == "car" ) {
    //   mErrorMessage = "ERROR (car" ;
    //   mErrorMessage += " with incorrect argument type) : " ;
    // } // if()
    // else if ( eT == WRONG_ARG_TYPE &&  cmd->Content() == "cdr" ) {
    //   mErrorMessage = "ERROR (cdr" ;
    //   mErrorMessage += " with incorrect argument type) : " ;
    // } // else if()
    // else if ( eT == WRONG_ARG_TYPE &&  cmd->Content() == "+" ) {
    //   mErrorMessage = "ERROR (+" ;
    //   mErrorMessage += " with incorrect argument type) : " ;
    // } // else if()

    // else if ( eT == WRONG_ARG_TYPE &&  cmd->Content() == "-" ) {
    //   mErrorMessage = "ERROR (-" ;
    //   mErrorMessage += " with incorrect argument type) : " ;
    // } // else if()

    // else if ( eT == WRONG_ARG_TYPE &&  cmd->Content() == "*" ) {
    //   mErrorMessage = "ERROR (*" ;
    //   mErrorMessage += " with incorrect argument type) : " ;
    // } // else if()

    // else if ( eT == WRONG_ARG_TYPE &&  cmd->Content() == "/" ) {
    //   mErrorMessage = "ERROR (/" ;
    //   mErrorMessage += " with incorrect argument type) : " ;
    // } // else if()

    // else if ( eT == WRONG_ARG_TYPE &&  cmd->Content() == ">" ) {
    //   mErrorMessage = "ERROR (>" ;
    //   mErrorMessage += " with incorrect argument type) : " ;
    // } // else if()

    // else if ( eT == WRONG_ARG_TYPE &&  cmd->Content() == "<" ) {
    //   mErrorMessage = "ERROR (<" ;
    //   mErrorMessage += " with incorrect argument type) : " ;
    // } // else if()

    // else if ( eT == WRONG_ARG_TYPE &&  cmd->Content() == "=" ) {
    //   mErrorMessage = "ERROR (=" ;
    //   mErrorMessage += " with incorrect argument type) : " ;
    // } // else if()

    // else if ( eT == WRONG_ARG_TYPE &&  cmd->Content() == ">=" ) {
    //   mErrorMessage = "ERROR (>=" ;
    //   mErrorMessage += " with incorrect argument type) : " ;
    // } // else if()

    // else if ( eT == WRONG_ARG_TYPE &&  cmd->Content() == "<=" ) {
    //   mErrorMessage = "ERROR (<=" ;
    //   mErrorMessage += " with incorrect argument type) : " ;
    // } // else if()

    // else if ( eT == WRONG_ARG_TYPE &&  cmd->Content() == "string-append" ) {
    //   mErrorMessage = "ERROR (string-append" ;
    //   mErrorMessage += " with incorrect argument type) : " ;
    // } // else if()

    // else
    //   cout << "Something went wrong..." << endl ;

  } // OurSchemeException()

}; // class OuSchemeException
// =====EnvFunction Class=====
class EnvFunction {
private:
  string mName ;
  vector<TreeNode*> mBodyStatments ;
  vector<TreeNode*> mParams ;


public:
  EnvFunction() {
    mName = "lambda" ;
    mBodyStatments.clear() ;
    mParams.clear() ;
  } // EnvFunction()

  EnvFunction( string name, vector<TreeNode*> body, vector<TreeNode*> params ) {
    mName = name ;
    mBodyStatments = body ;
    mParams = params ;
  } // EnvFunction()

  vector<TreeNode*> GetBody() {
    return mBodyStatments ;
  } // GetBody()

  vector<TreeNode*> GetParams() {
    return mParams ;
  } // GetParams()

  void SetBody( vector<TreeNode*> body ) {
    mBodyStatments = body ;
  } // SetBody()

  void SetParams( vector<TreeNode*> paramList ) {
    mParams = paramList ;
  } // SetParams()

}; // class EnvFunction
class BoundSet {
public:
  string mName ;
  TreeNode* mBinding ;

  BoundSet() {
    mName = "" ;
    mBinding = NULL ;
  } // BoundSet()

  BoundSet( string s, TreeNode* t ) {
    mName = s ;
    mBinding = t ;
  } // BoundSet()

}; // class BoundSet
// =====Environment Class=====
class Environment {

private:
  TreeNode* mResultPrt ;
  map<string, int> mCommandMap ;
  map<string, TreeNode*> mSymBindingMap ;
  // localVar應該改用stack
  // map<string, TreeNode*> mLocalVarMap ;
  vector<BoundSet*> mLocalVarSet ;
  map<TreeNode*, EnvFunction*> mFunctionMap ;
  map<string, int> mPredictionCmdMap ;

  // Return true if given string is a Internal Command
  bool HasCmd( string cmdStr ) {
    map<string, int>::iterator iter ;
    iter = mCommandMap.find( cmdStr ) ;

    if ( iter == mCommandMap.end() )
      return false ;
    else
      return true ;

  } // HasCmd()  
  // Return true if Symbol is bound
  bool HasBinding( string symStr ) {
    map<string, TreeNode*>::iterator iter1 ;
    iter1 = mSymBindingMap.find( symStr ) ;

    // 從localVarSet找
    for ( int i = 0 ; i < mLocalVarSet.size() ; i++ ) {
      if ( mLocalVarSet.at( i )->mName == symStr )
        return true ;
    } // for()

    if ( iter1 != mSymBindingMap.end() ) return true ;

    else return false ;
  } // HasBinding()
  // Return true is Function is exit
  bool HasFunction( TreeNode* t ) {
    map<TreeNode*, EnvFunction*>::iterator it ;
    it = mFunctionMap.find( t ) ;
    if ( it == mFunctionMap.end() )
      return false ;
    else
      return true ;

  } // HasFunction()
  // Return Bound TreeNode
  TreeNode* GetBinding( string symStr ) {
    if ( ! HasBinding( symStr ) ) {
      cout << "Error! Symbol not bound..." << endl ;
      return NULL ;
    } // if()
    else {
      // local variable優先
      // map<string, TreeNode*>::iterator iter ;
      // iter = mLocalVarMap.find( symStr ) ;

      // 從後面開始找 先進後出
      for ( int i = mLocalVarSet.size() - 1 ; i >= 0 ; i-- ) {
        if ( mLocalVarSet.at( i )->mName == symStr )
          return mLocalVarSet.at( i )->mBinding ;
        
      } // for()


      return mSymBindingMap[ symStr ] ;
      
    } // else
  } // GetBinding()
  // Return number of args
  int GetNumOfArgs( TreeNode* expr ) {
    TreeNode* peek ;
    int count = 0 ;
    bool end ;
    for ( peek = expr->mRight ; peek != NULL ; peek = peek->mRight ) {
      if ( peek->NodeType() == CONS_NODE )
        count ++ ;
      else if ( peek->NodeType() == ATOM_NODE && peek->TokenType() != NIL )
        cout << "ERROR[GetNnumOfArgs]: ATOM-NODE and NOT NIL..." << endl ;
      else ;

    } // for()

    return count ;

  } // GetNumOfArgs()
  // Return evaluated args
  vector<TreeNode*> GetEvaluatedArgs( TreeNode* expr, int depth ) {
    vector<TreeNode*> args ;
    for ( TreeNode* next = expr->mRight ; next != NULL  ; next = next->mRight ) {
      if ( next->NodeType() == CONS_NODE ) 
        args.push_back( Evaluate( next->mLeft, depth+1 ) ) ;
      
    } // for()

    return args ;
  } // GetEvaluatedArgs()

  // Return args for quote
  vector<TreeNode*> GetUnevaledArgs( TreeNode* expr ) {
    vector<TreeNode*> args ;
    for ( TreeNode* next = expr->mRight ; next != NULL  ; next = next->mRight ) {
      if ( next->NodeType() == CONS_NODE )
        args.push_back( next->mLeft ) ;

    } // for()

    return args ;
  } // GetUnevaledArgs()

  // compare two node
  bool CheckEqual( TreeNode* arg1, TreeNode* arg2 ) {
    string s1 = "" ;
    string s2 = "" ;

    if ( arg1->NodeType() == ATOM_NODE && arg2->NodeType() == ATOM_NODE ) {
      s1 = arg1->Content() ;
      s2 = arg2->Content() ;
    } // if()
    else if ( arg1->NodeType() == CONS_NODE && arg2->NodeType() == CONS_NODE )
      ;
    // 二者node type不一致
    else
      return false ;

    // 開始比對
    if ( s1 != s2 ) return false ;
    
    // arg1, arg2 往左走
    if ( arg1->mLeft != NULL && arg2->mLeft != NULL ) {
      // 底層傳回false就往上傳false
      if ( ! CheckEqual( arg1->mLeft, arg2->mLeft ) )
        return false ;
    } // if()
    // 兩邊都到底
    else if ( arg1->mLeft == NULL && arg2->mLeft == NULL )
      ;
    // 兩邊不一致
    else
      return false ;

    // arg1, arg2 往右走
    if ( arg1->mRight != NULL && arg2->mRight != NULL ) {
      // 底層傳回false就往上傳false
      if ( ! CheckEqual( arg1->mRight, arg2->mRight ) )
        return false ;
    } // if()
    // 兩邊都到底
    else if ( arg1->mRight == NULL && arg2->mRight == NULL )
      ;
    // 兩邊不一致
    else
      return false ;

    return true ;

  } // CheckEqual()

  bool NumOfArgs_correct( TreeNode* cmd, int arg_num ) {

    string sym = cmd->Content() ;

    if ( sym == "cons" && arg_num == 2 ) return true ;
    else if ( sym == "list" ) return true ;
    else if ( arg_num == 1 && ( sym == "car" || sym == "cdr" ) ) return true ;
    else if ( mPredictionCmdMap[ sym ] > 0 && arg_num == 1 ) return true ; 
    else if ( sym == "not" && arg_num == 1 ) return true ;
    else if ( sym == "+" && arg_num >= 2 ) return true ;
    else if ( sym == "-" && arg_num >= 2 ) return true  ;
    else if ( sym == "*" && arg_num >= 2 ) return true  ;
    else if ( sym == "/" && arg_num >= 2 ) return true ;
    else if ( sym == ">" && arg_num >= 2 ) return true ;
    else if ( sym == "<" && arg_num >= 2 ) return true ;
    else if ( sym == "=" && arg_num >= 2 ) return true  ;
    else if ( sym == ">=" && arg_num >= 2 ) return true  ;
    else if ( sym == "<=" && arg_num >= 2 ) return true  ;
    else if ( sym == "string-append" && arg_num >= 2 ) return true  ;
    else if ( sym == "string>?" && arg_num >= 2 ) return true  ;
    else if ( sym == "string<?" && arg_num >= 2 ) return true  ;
    else if ( sym == "string=?" && arg_num >= 2 ) return true  ;
    else if ( sym == "eqv?" && arg_num == 2 ) return true ;
    else if ( sym == "equal?" && arg_num == 2 ) return true  ;
    else if ( sym == "clean-environment" && arg_num == 0 ) return true  ;
    else if ( sym == "exit" && arg_num == 0 ) return true  ;
    else return false ;

  } // NumOfArgs_correct()

public:
  friend class OurSchemeException ;
  // Initializer
  Environment() {
    for ( int i = 0 ; i < CMDNUM ; i ++ ) {
      mCommandMap[ gSystemCmd[ i ] ] = i+1 ;
    } // for()

    mPredictionCmdMap[ "atom?" ] = 1 ;
    mPredictionCmdMap[ "pair?" ] = 2  ;
    mPredictionCmdMap[ "list?" ] = 3  ;
    mPredictionCmdMap[ "null?" ] = 4  ;
    mPredictionCmdMap[ "integer?" ] = 5 ;
    mPredictionCmdMap[ "real?" ] = 6  ;
    mPredictionCmdMap[ "number?" ] = 7 ;
    mPredictionCmdMap[ "string?" ] = 8 ;
    mPredictionCmdMap[ "boolean?" ] = 9 ;
    mPredictionCmdMap[ "symbol?" ] = 10 ;

  } // Environment()

  TreeNode* Evaluate( TreeNode* expr, int depth ) {
    
    // Evaluating an atom but NOT a symbol
    if ( expr->NodeType() == ATOM_NODE && expr->TokenType() != SYMBOL && expr->TokenType() != QUOTE ) {
      // cout << "Evaluated..." << endl ;
      return expr ;
    } // if()

    // Evaluating a symbol
    else if ( expr->NodeType() == ATOM_NODE && expr->TokenType() == SYMBOL ) {
      // 檢查symbol是否在IntnalCmdMap 或 SymBindingMap
      string sym = expr->Content() ;

      if ( ! ( HasCmd( sym ) || HasBinding( sym ) ) )
        throw OurSchemeException( UNBND_SYM, expr ) ;
      else if ( HasCmd( sym ) ) {
        expr->SetEvaled() ;
        return expr ;
      } // else if()
      else {
        TreeNode* result = GetBinding( sym ) ;
        if ( HasFunction( result ) )
          result->SetEvaled() ;
        
        return result ;
      } // else

    } // else if()

    // Evaluating a (...)
    else {

      // 檢查non-list error
      TreeNode* peek ;
      for ( peek = expr->mRight ; peek != NULL ; peek = peek->mRight ) {
        if ( peek->NodeType() == ATOM_NODE && peek->TokenType() != NIL )
          throw OurSchemeException( NON_LIST, expr ) ;

      } // for()

      // 檢查apply-non-function error (first arg is a atom but not symbol)
      TreeNode* firstArg = expr->mLeft ;
      if ( firstArg->NodeType() == ATOM_NODE && firstArg->TokenType() != SYMBOL &&
           firstArg->TokenType() != QUOTE ) {
        throw OurSchemeException( APLY_NON_FUN, firstArg ) ;
      } // if()

      // first arg是一個symbol
      else if ( firstArg->NodeType() == ATOM_NODE && 
                ( firstArg->TokenType() == SYMBOL || firstArg->TokenType() == QUOTE ) ) {
        string sym = firstArg->Content() ;
        
        // 檢查是不是user function
        if ( HasBinding( sym ) ) {
          TreeNode* binding = GetBinding( sym ) ;
          if ( HasFunction( binding ) )
            return Eval_func( expr, binding, depth ) ;

        } // if()

        bool boundToFunction = false ;
        if ( HasBinding( sym ) && HasCmd( GetBinding( sym )->Content() ) ) // check SYM is SYMBOL
          boundToFunction = true ;

        // 若SYM是Function
        if ( HasCmd( sym ) || boundToFunction ) {
          // SO MUCH TO DO...

          // 將SYM設定成綁定的CMD
          if ( boundToFunction ) {
            firstArg = GetBinding( sym ) ;
            sym =  firstArg->Content() ;
          } // if()

          // Level Error ( clean-environment or exit or define )
          if ( depth != 0 && ( sym == "clean-environment" || sym == "exit" || sym == "define" ) )
            throw OurSchemeException( LEVL, sym ) ;

          // SYM is 'define', 'cond'  ||    (proj.3) 'lambda' || 'set!', 'let',
          else if ( sym == "define" || sym == "cond" || sym == "let" || sym == "lambda" ) {
            if ( sym == "define" ) return Eval_define( expr, firstArg, depth ) ;
            else if ( sym == "cond" ) return Eval_cond( expr, firstArg, depth ) ;
            // let的format error需要main-expr
            else if ( sym == "let" ) return Eval_let( expr, firstArg, depth ) ;
            else if ( sym == "lambda" ) return Eval_lambda( expr, firstArg ) ;


          } // else if()

          // SYM is 'if', 'and', 'or'
          // these three can get unvaluated argument
          else if ( sym == "if" || sym == "and" || sym == "or" || sym == "quote" || sym == "begin" ) {
            vector<TreeNode*> args = GetUnevaledArgs( expr ) ;

            if ( sym == "and" ) return Eval_and( firstArg, args, depth ) ;
            else if ( sym == "or" ) return Eval_or( firstArg, args, depth ) ;
            else if ( sym == "quote" )  return Eval_quote( firstArg, args ) ;
            else if ( sym == "if" )  return Eval_if( expr, firstArg, args, depth ) ;
            else if ( sym == "begin" )  return Eval_begin( expr, firstArg, args, depth ) ;
            
            // return result

          } // else if()

          // other functions
          // "atom?", "pair?", "list?", "null?","integer?", "real?",
          //  "number?", "string?", "boolean?", "symbol?",
          //  "+", "-", "*", "/",
          // "not", ">", ">=", "<", "<=", "=", "string-append", "string>?",
          // "string<?", "string=?", "eqv?", "equal?", "begin"
          else {
            // get args ( unevaluated )
            vector<TreeNode*> args ;
            args = GetUnevaledArgs( expr ) ;

            if ( ! NumOfArgs_correct( firstArg, args.size() ) )
              throw OurSchemeException( NUM_OF_ARGS, firstArg->Content() ) ; // ( first arg應該eval嗎)
            else
              args = GetEvaluatedArgs( expr, depth ) ;

            if ( sym == "cons" ) return Eval_cons( firstArg, args ) ;
            else if ( sym == "list" ) return Eval_list( firstArg, args ) ;
            else if ( sym == "car" || sym == "cdr" )
              return Eval_partAccessor( firstArg, args ) ;
            else if ( mPredictionCmdMap[ sym ] > 0 ) return Eval_pmtPredicate( firstArg, args ) ;
            else if ( sym == "not" ) return Eval_not( firstArg, args ) ;
            else if ( sym == "+" ) return Eval_plus( firstArg, args ) ;
            else if ( sym == "-" ) return Eval_minus( firstArg, args ) ;
            else if ( sym == "*" ) return Eval_multi( firstArg, args ) ;
            else if ( sym == "/" ) return Eval_devide( firstArg, args ) ;
            else if ( sym == ">" ) return Eval_greatThan( firstArg, args ) ;
            else if ( sym == "<" ) return Eval_lessThan( firstArg, args ) ;
            else if ( sym == "=" ) return Eval_compareEqual( firstArg, args ) ;
            else if ( sym == ">=" ) return Eval_greatAndEqual( firstArg, args ) ;
            else if ( sym == "<=" ) return Eval_lessAndEqual( firstArg, args ) ;
            else if ( sym == "string-append" ) return Eval_stringAppend( firstArg, args ) ;
            else if ( sym == "string>?" ) return Eval_stringCompare( firstArg, args ) ;
            else if ( sym == "string<?" ) return Eval_stringCompare( firstArg, args ) ;
            else if ( sym == "string=?" ) return Eval_stringCompare( firstArg, args ) ;
            else if ( sym == "eqv?" ) return Eval_eqv( firstArg, args ) ;
            else if ( sym == "equal?" ) return Eval_equal( firstArg, args ) ;
            else if ( sym == "clean-environment" ) return Eval_cleanEnv( firstArg, args ) ;
            else if ( sym == "exit" ) return Eval_exit( firstArg, args ) ;
            
          } // else

        } // if()

        // SYM is NOT the name of known function
        else {
          if ( ! HasBinding( sym ) )
            throw OurSchemeException( UNBND_SYM, firstArg ) ;
          else {
            TreeNode* binding = GetBinding( sym ) ;
            throw OurSchemeException( APLY_NON_FUN, binding ) ;
          } // else

        } // else

      } // if()

      // first arg是一個( ... )
      else {

        TreeNode* leftArg = Evaluate( firstArg, depth+1 ) ;

        // 以下是沒有binding的nameless lambda expression
        if ( leftArg->Content() == "lambda" && HasFunction( leftArg ) ) {
          TreeNode* result = Eval_func( expr, leftArg, depth ) ;
          map<TreeNode*, EnvFunction*>::iterator it ;
          it = mFunctionMap.find( leftArg ) ;
          if ( it != mFunctionMap.end() )
            mFunctionMap.erase( it ) ;

          return result ;
        } // if()


        if ( leftArg->NodeType() == ATOM_NODE && leftArg->TokenType() == SYMBOL )
          expr->mLeft = leftArg ;
        else
          throw OurSchemeException( APLY_NON_FUN, leftArg ) ;

        return Evaluate( expr, depth ) ;
      } // else

    } // else



    return NULL ;

  } // Evaluate()

  TreeNode* Eval_cons( TreeNode* cmd, vector<TreeNode*> args ) {
    
    // if ( args.size() != 2 )
    //   throw OurSchemeException( NUM_OF_ARGS, cmd ) ;

    TreeNode* result = new TreeNode() ;
    result->InitCons() ;
    result->mLeft = args.at( 0 ) ;
    result->mRight = args.at( 1 ) ;
    return result ;

  } // Eval_cons()

  TreeNode* Eval_quote( TreeNode* cmd, vector<TreeNode*> args ) {
    // if ( args.size() != 1 )
    //   throw OurSchemeException( NUM_OF_ARGS, cmd ) ;

    TreeNode* result =  args.at( 0 ) ;
    return result ;

  } // Eval_quote()

  TreeNode* Eval_list( TreeNode* cmd, vector<TreeNode*> args ) {

    TreeNode* result = new TreeNode() ;
    TreeNode* next ;
  
    next = result ;

    if ( args.size() == 0 )
      result->InitAtom( "()", NIL ) ;
    else {
      for ( int i = 0 ; i < args.size() ; i++ ) {
        next->InitCons() ;
        next->mLeft = args.at( i ) ;
        next = next->mRight ;
      } // for()

    } // else

    return result ;
  } // Eval_list()

  TreeNode* Eval_partAccessor( TreeNode* cmd, vector<TreeNode*> args ) {
    // if ( args.size() != 1 )
    //   throw OurSchemeException( NUM_OF_ARGS, cmd ) ;

    if ( args.at( 0 )->NodeType() == ATOM_NODE )
      throw OurSchemeException( WRONG_ARG_TYPE, args.at( 0 ), cmd ) ;

    TreeNode* result ;
    


    if ( cmd->Content() == "car" )
      result = args.at( 0 )->mLeft ;
    else if ( cmd->Content() == "cdr" )
      result = args.at( 0 )->mRight ;
    else
      cout << "Not car or cdr at line 1105..." << endl ;

    return result ;

  } // Eval_partAccessor()

  TreeNode* Eval_pmtPredicate( TreeNode* cmd, vector<TreeNode*> args ) {
    // if ( args.size() != 1 )
    //   throw OurSchemeException( NUM_OF_ARGS, cmd ) ;

    bool result = false ;
    TreeNode *arg = args.at( 0 ) ;

    // Evaluate each function
    if ( cmd->Content() == "atom?" && arg->NodeType() == ATOM_NODE )
      result = true ;
    else if ( cmd->Content() == "pair?" && arg->NodeType() == CONS_NODE ) {
      // if ( arg->NodeType() == CONS_NODE && arg->mRight->TokenType() != NIL )
      result = true ;
    } // else if()
    else if ( cmd->Content() == "list?" && arg->NodeType() == CONS_NODE ) {
      result = true ;
      for ( TreeNode* next = arg->mRight ; next != NULL ; next = next->mRight ) {
        if ( next->NodeType() == ATOM_NODE && next->TokenType() != NIL )
          result = false ;
      } // for()
    } // else if()
    else if ( cmd->Content() == "null?" && arg->NodeType() == ATOM_NODE && arg->TokenType() == NIL )
      result = true ;

    else if ( cmd->Content() == "integer?" && arg->NodeType() == ATOM_NODE && arg->TokenType() == INT )
      result = true ;

    else if ( cmd->Content() == "number?" || cmd->Content() == "real?" ) {
      if ( cmd->NodeType() == ATOM_NODE &&
           ( arg->TokenType() == INT || arg->TokenType() == FLOAT ) )
        result = true ;
    } // else if()

    else if ( cmd->Content() == "string?" && 
              arg->NodeType() == ATOM_NODE && arg->TokenType() == STRING )
      result = true ;

    else if ( cmd->Content() == "boolean?" && arg->NodeType() == ATOM_NODE &&
              ( arg->TokenType() == T || arg->TokenType() == NIL ) )
      result = true ;
    else if ( cmd->Content() == "symbol?" && arg->NodeType() == ATOM_NODE &&
              arg->TokenType() == SYMBOL )
      result = true ;


    // Generating Result...
    TreeNode* t = new TreeNode() ;
    if ( result )
      t->InitAtom( "#t", T ) ;
    else
      t->InitAtom( "()", NIL ) ;

    return t ;

  } // Eval_pmtPredicate() ;

  TreeNode* Eval_not( TreeNode* cmd, vector<TreeNode*> args ) {
    // if ( args.size() != 1 )
    //   throw OurSchemeException( NUM_OF_ARGS, cmd ) ;

    TreeNode* result = new TreeNode() ;
    // arg不是f result就是false
    if ( args.at( 0 )->NodeType() == CONS_NODE || args.at( 0 )->TokenType() != NIL )
      result->InitAtom( "nil", NIL ) ;
    else
      result->InitAtom( "#t", T ) ;

    return result ;

  } // Eval_not()

  TreeNode* Eval_plus( TreeNode* cmd, vector<TreeNode*> args ) {
    // if ( args.size() < 2 )
    //   throw OurSchemeException( NUM_OF_ARGS, cmd ) ;

    int intResult = 0 ;
    float floatResult = 0 ;
    bool useFloat = false ;
    TreeNode* ans = new TreeNode() ;

    // 尋遍Args進行累加
    for ( int i = 0 ; i < args.size() ; i ++ ) {
      TreeNode* current = args.at( i ) ;
      if ( current->NodeType() == CONS_NODE || 
           ! ( current->TokenType() == INT || current->TokenType() == FLOAT ) )
        throw OurSchemeException( WRONG_ARG_TYPE, current, cmd ) ;

      if ( ! useFloat && current->TokenType() == FLOAT ) {
        useFloat = true ;
        floatResult = ( float ) intResult ;
      } // if()


      if ( ! useFloat ) {
        intResult += current->IntValue() ;
      } // if()
      else {
        if ( current->TokenType() == FLOAT )
          floatResult += current->FloatValue() ;
        else
          floatResult += ( float ) current->IntValue() ;

      } // else

    } // for()

    // 針對不同型別轉換成字串
    if ( useFloat ) {
      stringstream ss ;
      ss << floatResult ;
      ans->InitAtom( ss.str(), FLOAT ) ;
    } // if()
    else {
      stringstream ss ;
      ss << intResult ;
      ans->InitAtom( ss.str(), INT ) ;
    } // else()

    return ans ;

  } // Eval_plus()

  TreeNode* Eval_minus( TreeNode* cmd, vector<TreeNode*> args ) {
    // if ( args.size() < 2 )
    //   throw OurSchemeException( NUM_OF_ARGS, cmd ) ;

    bool useFloat = false ;
    int intResult = 0 ;
    float floatResult = 0 ;
    TreeNode* ans = new TreeNode() ;

    // 尋遍Args進行累減
    for ( int i = 0 ; i < args.size() ; i ++ ) {
      TreeNode* current = args.at( i ) ;
      if ( current->NodeType() == CONS_NODE || 
           ! ( current->TokenType() == INT || current->TokenType() == FLOAT ) )
        throw OurSchemeException( WRONG_ARG_TYPE, current, cmd ) ;

      if ( ! useFloat && current->TokenType() == FLOAT ) {
        useFloat = true ;
        floatResult = ( float ) intResult ;
      } // if()

      // set init value
      if ( i == 0 && current->TokenType() == FLOAT )
        floatResult = current->FloatValue() ;
      else if ( i == 0 && current->TokenType() == INT )
        intResult = current->IntValue() ;
      else if ( i != 0 && current->TokenType() == FLOAT ) {
        floatResult -= current->FloatValue() ;
      } // else if()
      else if ( useFloat && i != 0 && current->TokenType() == INT ) {
        floatResult -= ( float ) current->IntValue() ;
      } // else if()
      else if ( ! useFloat )
        intResult -= current->IntValue() ;
        

    } // for()

    if ( useFloat ) {
      stringstream ss ;
      ss << floatResult ;
      ans->InitAtom( ss.str(), FLOAT ) ;
    } // if()
    else {
      stringstream ss ;
      ss << intResult ;
      ans->InitAtom( ss.str(), INT ) ;
    } // else

    return ans ;
  } // Eval_minus()

  TreeNode* Eval_multi( TreeNode* cmd, vector<TreeNode*> args ) {
    // if ( args.size() < 2 )
    //   throw OurSchemeException( NUM_OF_ARGS, cmd ) ;

    bool useFloat = false ;
    int intResult = 0 ;
    float floatResult = 0 ;
    TreeNode* ans = new TreeNode() ;

    // 尋遍Args進行累乘法
    for ( int i = 0 ; i < args.size() ; i ++ ) {
      TreeNode* current = args.at( i ) ;
      if ( current->NodeType() == CONS_NODE || 
           ! ( current->TokenType() == INT || current->TokenType() == FLOAT ) )
        throw OurSchemeException( WRONG_ARG_TYPE, current, cmd ) ;

      // 第一次遇到float, 將現有result轉成float
      if ( ! useFloat && current->TokenType() == FLOAT ) {
        useFloat = true ;
        floatResult = ( float ) intResult ;
      } // if()

      // set init value
      if ( i == 0 && current->TokenType() == FLOAT )
        floatResult = current->FloatValue() ;
      else if ( i == 0 && current->TokenType() == INT )
        intResult = current->IntValue() ;
      // 累乘
      else if ( i != 0 && current->TokenType() == FLOAT ) {
        floatResult *= current->FloatValue() ;
      } // else if()
      else if ( useFloat && i != 0 && current->TokenType() == INT ) {
        floatResult *= ( float ) current->IntValue() ;
      } // else if()
      else if ( ! useFloat )
        intResult *= current->IntValue() ;



    } // for()

    if ( useFloat ) {
      stringstream ss ;
      ss << floatResult ;
      ans->InitAtom( ss.str(), FLOAT ) ;
    } // if()
    else {
      stringstream ss ;
      ss << intResult ;
      ans->InitAtom( ss.str(), INT ) ;
    } // else

    return ans ;

  } // Eval_multi()

  TreeNode* Eval_devide( TreeNode* cmd, vector<TreeNode*> args ) {
    // if ( args.size() < 2 )
    //   throw OurSchemeException( NUM_OF_ARGS, cmd ) ;

    bool useFloat = false ;
    int intResult = 0 ;
    float floatResult = 0 ;
    TreeNode* ans = new TreeNode() ;

    // 尋遍Args進行累除法
    for ( int i = 0 ; i < args.size() ; i ++ ) {
      TreeNode* current = args.at( i ) ;
      if ( current->NodeType() == CONS_NODE || 
           ! ( current->TokenType() == INT || current->TokenType() == FLOAT ) )
        throw OurSchemeException( WRONG_ARG_TYPE, current, cmd ) ;

      if ( i != 0 && current->NodeType() == ATOM_NODE &&
           ( current->IntValue() == 0 || current->FloatValue() == 0 ) )
        throw OurSchemeException( DIV_ZERO, cmd ) ;

      // 第一次遇到float, 將現有result轉成float
      if ( ! useFloat && current->TokenType() == FLOAT ) {
        useFloat = true ;
        floatResult = ( float ) intResult ;
      } // if()

      // set init value
      
      if ( i == 0 && current->TokenType() == FLOAT )
        floatResult = current->FloatValue() ;
      else if ( i == 0 && current->TokenType() == INT )
        intResult = current->IntValue() ;
      // 累乘
      else if ( i != 0 && current->TokenType() == FLOAT ) {
        floatResult /= current->FloatValue() ;
      } // else if()
      else if ( useFloat && i != 0 && current->TokenType() == INT ) {
        floatResult /= ( float ) current->IntValue() ;
      } // else if()
      else if ( ! useFloat )
        intResult /= current->IntValue() ;



    } // for()

    if ( useFloat ) {
      stringstream ss ;
      ss << floatResult ;
      ans->InitAtom( ss.str(), FLOAT ) ;
    } // if()
    else {
      stringstream ss ;
      ss << intResult ;
      ans->InitAtom( ss.str(), INT ) ;
    } // else

    return ans ;
  } // Eval_devide()

  TreeNode* Eval_greatThan( TreeNode* cmd, vector<TreeNode*> args ) {
    // if ( args.size() != 2 )
    //   throw OurSchemeException( NUM_OF_ARGS, cmd ) ;

    float num1, num2 ;
    TreeNode* result = new TreeNode() ;
    // 檢查type error
    for ( int i = 0 ; i < args.size() ; i++ ) {
      TreeNode* current = args.at( i ) ;
      if ( current->NodeType() != ATOM_NODE ||
           ! ( current->TokenType() == INT || current->TokenType() == FLOAT ) )
        throw OurSchemeException( WRONG_ARG_TYPE, current, cmd ) ;
    } // for()

    for ( int i = 0 ; i+1 < args.size() ; i++ ) {
      // 設定num1 num2
      if ( args.at( i )->TokenType() == FLOAT )
        num1 = args.at( i )->FloatValue() ;
      else
        num1 = args.at( i )->IntValue() ;

      if ( args.at( i+1 )->TokenType() == FLOAT )
        num2 = args.at( i+1 )->FloatValue() ;
      else
        num2 = args.at( i+1 )->IntValue() ;

      // 設定result
      if ( num1 > num2 )
        ;
      else {
        result->InitAtom( "nil", NIL ) ;
        return result ;
      } // else
    } // for
    
    result->InitAtom( "#t", T ) ;
    return result ;

  } // Eval_greatThan()

  TreeNode* Eval_lessThan( TreeNode* cmd, vector<TreeNode*> args ) {
    // if ( args.size() != 2 )
    //   throw OurSchemeException( NUM_OF_ARGS, cmd ) ;

    float num1, num2 ;
    TreeNode* result = new TreeNode() ;
    // 檢查type error
    for ( int i = 0 ; i < args.size() ; i++ ) {
      TreeNode* current = args.at( i ) ;
      if ( current->NodeType() != ATOM_NODE ||
           ! ( current->TokenType() == INT || current->TokenType() == FLOAT ) )
        throw OurSchemeException( WRONG_ARG_TYPE, current, cmd ) ;
    } // for()

    for ( int i = 0 ; i+1 < args.size() ; i++ ) {
      // 設定num1 num2
      if ( args.at( i )->TokenType() == FLOAT )
        num1 = args.at( i )->FloatValue() ;
      else
        num1 = args.at( i )->IntValue() ;

      if ( args.at( i+1 )->TokenType() == FLOAT )
        num2 = args.at( i+1 )->FloatValue() ;
      else
        num2 = args.at( i+1 )->IntValue() ;

      // 設定result
      if ( num1 < num2 )
        ;
      else {
        result->InitAtom( "nil", NIL ) ;
        return result ;
      } // else
    } // for()

    result->InitAtom( "#t", T ) ;
    return result ;
  } // Eval_lessThan()

  TreeNode* Eval_compareEqual( TreeNode* cmd, vector<TreeNode*> args ) {
    // if ( args.size() != 2 )
    //   throw OurSchemeException( NUM_OF_ARGS, cmd ) ;

    float num1, num2 ;
    TreeNode* result = new TreeNode() ;
    // 檢查type error
    for ( int i = 0 ; i < args.size() ; i++ ) {
      TreeNode* current = args.at( i ) ;
      if ( current->NodeType() != ATOM_NODE ||
           ! ( current->TokenType() == INT || current->TokenType() == FLOAT ) )
        throw OurSchemeException( WRONG_ARG_TYPE, current, cmd ) ;
    } // for()

    for ( int i = 0 ; i+1 < args.size() ; i++ ) {
      // 設定num1 num2
      if ( args.at( i )->TokenType() == FLOAT )
        num1 = args.at( i )->FloatValue() ;
      else
        num1 = args.at( i )->IntValue() ;

      if ( args.at( i+1 )->TokenType() == FLOAT )
        num2 = args.at( i+1 )->FloatValue() ;
      else
        num2 = args.at( i+1 )->IntValue() ;

      // 設定result
      if ( num1 == num2 )
        ;
      else {
        result->InitAtom( "nil", NIL ) ;
        return result ;
      } // else()

    } // for()

    result->InitAtom( "#t", T ) ;
    return result ;
  } // Eval_compareEqual()

  TreeNode* Eval_greatAndEqual( TreeNode* cmd, vector<TreeNode*> args ) {
    // if ( args.size() != 2 )
    //   throw OurSchemeException( NUM_OF_ARGS, cmd ) ;

    float num1, num2 ;
    TreeNode* result = new TreeNode() ;
    // 檢查type error
    for ( int i = 0 ; i < args.size() ; i++ ) {
      TreeNode* current = args.at( i ) ;
      if ( current->NodeType() != ATOM_NODE ||
           ! ( current->TokenType() == INT || current->TokenType() == FLOAT ) )
        throw OurSchemeException( WRONG_ARG_TYPE, current, cmd ) ;
    } // for()

    for ( int i = 0 ; i+1 < args.size() ; i++ ) {
      // 設定num1 num2
      if ( args.at( i )->TokenType() == FLOAT )
        num1 = args.at( i )->FloatValue() ;
      else
        num1 = args.at( i )->IntValue() ;

      if ( args.at( i+1 )->TokenType() == FLOAT )
        num2 = args.at( i+1 )->FloatValue() ;
      else
        num2 = args.at( i+1 )->IntValue() ;

      // 設定result
      if ( num1 >= num2 )
        ;
      else {
        result->InitAtom( "nil", NIL ) ;
        return result ;
      } // else()

    } // for()

    result->InitAtom( "#t", T ) ;
    return result ;
  } // Eval_greatAndEqual()

  TreeNode* Eval_lessAndEqual( TreeNode* cmd, vector<TreeNode*> args ) {
    // if ( args.size() != 2 )
    //   throw OurSchemeException( NUM_OF_ARGS, cmd ) ;

    float num1, num2 ;
    TreeNode* result = new TreeNode() ;
    // 檢查type error
    for ( int i = 0 ; i < args.size() ; i++ ) {
      TreeNode* current = args.at( i ) ;
      if ( current->NodeType() != ATOM_NODE ||
           ! ( current->TokenType() == INT || current->TokenType() == FLOAT ) )
        throw OurSchemeException( WRONG_ARG_TYPE, current, cmd ) ;
    } // for()

    for ( int i = 0 ; i+1 < args.size() ; i++ ) {
      // 設定num1 num2
      if ( args.at( i )->TokenType() == FLOAT )
        num1 = args.at( i )->FloatValue() ;
      else
        num1 = args.at( i )->IntValue() ;

      if ( args.at( i+1 )->TokenType() == FLOAT )
        num2 = args.at( i+1 )->FloatValue() ;
      else
        num2 = args.at( i+1 )->IntValue() ;

      // 設定result
      if ( num1 <= num2 )
        ;
      else {
        result->InitAtom( "nil", NIL ) ;
        return result ;
      } // else()

    } // for()

    result->InitAtom( "#t", T ) ;
    return result ;
  } // Eval_lessAndEqual()

  TreeNode* Eval_stringAppend( TreeNode* cmd, vector<TreeNode*> args ) {
    // if ( args.size() < 2 )
    //   throw OurSchemeException( NUM_OF_ARGS, cmd ) ;

    TreeNode* ans = new TreeNode() ;
    string result = "" ;

    // 檢查args的type
    // 一併執行
    for ( int i = 0 ; i < args.size() ; i++ ) {
      TreeNode* current = args.at( i ) ;
      if ( current->NodeType() == CONS_NODE || current->TokenType() != STRING )
        throw OurSchemeException( WRONG_ARG_TYPE, current, cmd ) ;

      string s = current->Content() ;
      s = s.substr( 1, s.size()-2 ) ;
      result += s ;
    } // for()

    result = "\"" +result + "\"" ;
    ans->InitAtom( result, STRING ) ;
    return ans ;

  } // Eval_stringAppend()

  TreeNode* Eval_stringCompare( TreeNode* cmd, vector<TreeNode*> args ) {
    // if ( args.size() < 2 )
    //   throw OurSchemeException( NUM_OF_ARGS, cmd ) ;

    string s1, s2 ;
    string command = cmd->Content() ;
    bool result = false ;
    bool stopEval = false ;
    // Type check
    for ( int i = 0 ; i < args.size() ; i++ ) {
      TreeNode* current = args.at( i ) ;
      if ( current->NodeType() == CONS_NODE || current->TokenType() != STRING )
        throw OurSchemeException( WRONG_ARG_TYPE, current, cmd ) ;

      if ( i+1 < args.size() && ! stopEval ) {
        s1 = current->Content() ;
        s2 = args.at( i + 1 )->Content() ;
        if ( command == "string>?" && ( s1 > s2 ) ) {
          result = true ;
        } // if()
        else if ( command == "string<?" && ( s1 < s2 ) ) {
          result = true ;
        } // else if()
        else if ( command == "string=?" && ( s1 == s2 )  ) {
          result = true ;
        } // else if()
        else {
          result = false ;
          stopEval = true ;
        } // else
      } // if()

    } // for()

    TreeNode* ans = new TreeNode() ;

    if ( result )
      ans->InitAtom( "#t", T ) ;
    else
      ans->InitAtom( "#f", NIL ) ;

    return ans ;
  } // Eval_stringCompare()

  TreeNode* Eval_and( TreeNode* cmd, vector<TreeNode*> args, int depth ) {

    if ( args.size() < 2 )
      throw OurSchemeException( NUM_OF_ARGS, cmd->Content() ) ;

    TreeNode* result = new TreeNode() ;

    for ( int i = 0 ; i < args.size() ; i ++ ) {
      result = Evaluate( args.at( i ), depth+1 ) ;

      if ( result->NodeType() == ATOM_NODE && result->TokenType() == NIL )
        return result ;

    } // for()

    return result ;

  } // Eval_and()

  TreeNode* Eval_or( TreeNode* cmd, vector<TreeNode*> args, int depth ) {
    if ( args.size() < 2 )
      throw OurSchemeException( NUM_OF_ARGS, cmd->Content() ) ;

    TreeNode* result = new TreeNode() ;

    for ( int i = 0 ; i < args.size() ; i ++ ) {
      result = Evaluate( args.at( i ), depth+1 ) ;

      // 若result非nil
      if ( result->TokenType() != NIL )
        return result ;

    } // for()

    return result ;

  } // Eval_or()

  TreeNode* Eval_eqv( TreeNode* cmd, vector<TreeNode*> args ) {
    // if ( args.size() != 2 )
    //   throw OurSchemeException( NUM_OF_ARGS, cmd ) ;
    

    // 兩個atom(not quote and not string)的content(text-string)一樣
    // 或是ar1 == arg2 ( 指向相同位置 ) ;
    bool result = false ;
    TreeNode* arg1 = args.at( 0 ) ;
    TreeNode* arg2 = args.at( 1 ) ;

    if ( arg1 == arg2 )
      result = true ;
    // 都做compare 但只有atom會是#t 且比較text
    else {
      if ( arg1->NodeType() == CONS_NODE || arg1->TokenType() == STRING )
        result = false ;
      // 檢查arg2的wrong type error
      else if ( arg2->NodeType() == CONS_NODE || arg2->TokenType() == STRING )
        result = false ;

      else {
        if ( arg1->Content() == arg2->Content() )
          result = true ;
      } // else()
    } // else()

    TreeNode* ans = new TreeNode() ;

    if ( result ) ans->InitAtom( "#t", T ) ;
    else ans->InitAtom( "nil", NIL ) ;

    return ans ;
  } // Eval_eqv()

  TreeNode* Eval_equal( TreeNode* cmd, vector<TreeNode*> args ) {
    // if ( args.size() != 2 )
    //   throw OurSchemeException( NUM_OF_ARGS, cmd ) ;

    
    TreeNode* arg1 = args.at( 0 ) ;
    TreeNode* arg2 = args.at( 1 ) ;
    bool result = CheckEqual( arg1, arg2 ) ;

    TreeNode* ans = new TreeNode() ;

    if ( result ) ans->InitAtom( "#t", T ) ;
    else ans->InitAtom( "nil", NIL ) ;

    return ans ;
  } // Eval_equal()
  // expr* is only for print out error message(NO_RETURN_VAL)
  TreeNode* Eval_if( TreeNode* expr, TreeNode* cmd, vector<TreeNode*> args, int depth ) {
    if ( args.size() != 2 && args.size() != 3 )
      throw OurSchemeException( NUM_OF_ARGS, cmd->Content() ) ;

    TreeNode* cond = Evaluate( args.at( 0 ), depth+1 ) ;

    // 沒有nil的condition error
    if ( cond->TokenType() == NIL && args.size() == 2 )
      throw OurSchemeException( NO_RETURN_VAL, expr ) ;
    else if ( cond->TokenType() == NIL )
      return Evaluate( args.at( 2 ), depth+1 ) ;
    else
      return Evaluate( args.at( 1 ), depth+1 ) ;

  } // Eval_if()

  TreeNode* Eval_cond( TreeNode* expr, TreeNode* cmd, int depth ) {
    // Format : ( cond 1-or-more-{list ( >= 2 SEXPR) })
    vector<TreeNode*> args = GetUnevaledArgs( expr ) ;

    // check arg num
    if ( args.size() < 1 )
      throw OurSchemeException( FORMAT_ERR, expr, cmd ) ;

    // 逐一取arg-n的sub-expression
    for ( int i = 0 ; i < args.size() ; i ++ ) {
      TreeNode* current = args.at( i ) ;

      // 先判斷arg-n是否有non-list error
      TreeNode* peek ;
      for ( peek = current->mRight ; peek != NULL ; peek = peek->mRight ) {
        if ( peek->NodeType() == ATOM_NODE && peek->TokenType() != NIL )
          // 這邊不丟non-list error 而是format
          throw OurSchemeException( FORMAT_ERR, expr, cmd ) ;

      } // for()

      // 確定是list後 取得arg-n的sub-expressions
      vector<TreeNode*> subExpressions = GetUnevaledArgs( current ) ;
      // 若current的右側沒有任何sub-exprssion 則錯誤
      if ( subExpressions.size() < 1 )
        throw OurSchemeException( FORMAT_ERR, expr, cmd ) ;

    } // for()

    // 確認無format error 開始eval
    for ( int i = 0 ; i < args.size() ; i ++ ) {
      TreeNode* current = args.at( i ) ;
      vector<TreeNode*> subExpressions = GetUnevaledArgs( current ) ;

      // 若condition成立 依序eval後面的sub-expressions
      TreeNode* lastEval ;
      // else的case
      if ( i == args.size() - 1 && current->mLeft->Content() == "else" ) {
        for ( int j = 0 ; j < subExpressions.size() ; j++ ) {
          lastEval = Evaluate( subExpressions.at( j ), depth+2 ) ;
        } // for()

        return lastEval ;
      } // if()

      // 其他
      TreeNode* condition =  Evaluate( current->mLeft, depth+2 ) ;
      if ( condition->NodeType() == CONS_NODE || condition->TokenType() != NIL ) {
        for ( int j = 0 ; j < subExpressions.size() ; j++ ) {
          lastEval = Evaluate( subExpressions.at( j ), depth+2 ) ;
        } // for()

        return lastEval ;
      } // if()

    } // for()

    // throw no return value
    throw OurSchemeException( NO_RETURN_VAL, expr ) ;
    
    
  } // Eval_cond()

  TreeNode* Eval_define( TreeNode* expr, TreeNode* cmd, int depth ) {
    // num-of-args : 2
    // can't redefine system primitives
    // check format
    // num-of-arg-err 視為format error

    // 檢查format
    vector<TreeNode*> args = GetUnevaledArgs( expr ) ;

    if ( args.size() < 2 )
      throw OurSchemeException( FORMAT_ERR, expr, cmd ) ;

    // 送去function define
    if ( args.size() > 2  || args.at( 0 )->NodeType() == CONS_NODE )
      return Eval_funcDef( expr, cmd, depth ) ;

    TreeNode* arg1 = args.at( 0 ) ;
    TreeNode* arg2 = args.at( 1 ) ;
    
    // arg1只能是symbol
    if ( arg1->NodeType() == CONS_NODE || arg1->TokenType() != SYMBOL )
      throw OurSchemeException( FORMAT_ERR, expr, cmd ) ;
    // arg1是一個system primitive
    if ( HasCmd( arg1->Content() ) )
      throw OurSchemeException( FORMAT_ERR, expr, cmd ) ;

    // 如果arg2的eval有誤 不能redefine
    // 所以要先eval arg2
    arg2 = Evaluate( arg2, depth+1 ) ;
    // 若arg2是一個lambda 會回傳atom(lambda)
    // arg2 eval過後 處理binding
    if ( HasBinding( arg1->Content() ) ) {
      mSymBindingMap.erase( arg1->Content() ) ;
    } // if

    mSymBindingMap[ arg1->Content() ] = arg2  ;

    TreeNode* result = new TreeNode() ;
    // string s = arg1->Content() + " defined" ;
    string s = arg1->Content() ;
    result->InitAtom( s, SYMBOL ) ;
    // result->SetEvaled() ;
    return result ;

  } // Eval_define()

  TreeNode* Eval_funcDef( TreeNode* expr, TreeNode* cmd, int depth ) {
    // arg1 format : ( SYM 0-or-more-syms )
    // arg2~argN (statements of function) : expr
    
    vector<TreeNode*> args = GetUnevaledArgs( expr ) ;
    vector<TreeNode*> params ;
    vector<TreeNode*> statements ;
    // 檢查arg1 並取得params
    TreeNode* arg1 = args.at( 0 ) ;
    string funcName = arg1->mLeft->Content() ;
    int counter = 0 ;
    for ( TreeNode* tmp = arg1 ; tmp != NULL ; tmp = tmp->mRight ) {
      // 檢查non-list
      if ( tmp->NodeType() == ATOM_NODE && tmp->TokenType() != NIL )
        throw OurSchemeException( FORMAT_ERR, expr, cmd ) ;

      // 檢查非symbol
      if ( tmp->NodeType() == CONS_NODE && tmp->mLeft->TokenType() != SYMBOL )
        throw OurSchemeException( FORMAT_ERR, expr, cmd ) ;

      // 蒐集params
      if ( tmp->NodeType() == CONS_NODE ) {
        counter ++ ;
        if ( counter > 1 )
          params.push_back( tmp->mLeft ) ;

      } // if()
    
    } // for()

    for ( int i = 1 ; i < args.size() ; i++ ) {
      statements.push_back( args.at( i ) ) ;
    } // for()

    EnvFunction* newFunc = new EnvFunction( funcName, statements, params ) ;
    // 移除原有的define
    map<string, TreeNode*>::iterator it ;
    it = mSymBindingMap.find( arg1->Content() ) ;
    if ( it != mSymBindingMap.end() )
      mSymBindingMap.erase( it ) ;

    TreeNode* result = new TreeNode() ;
    result->InitAtom( funcName, SYMBOL ) ;
    // result->SetEvaled() ;
    mSymBindingMap[ funcName ] = result ;
    mFunctionMap[ result ] = newFunc ;

    return result ;

  } // Eval_funcDef()

  TreeNode* Eval_begin( TreeNode* expr, TreeNode* cmd, vector<TreeNode*> args, int depth ) {
    if ( args.size() == 0 )
      throw OurSchemeException( NUM_OF_ARGS, cmd->Content() ) ;


    TreeNode* lastEvaluated ;

    for ( int i = 0 ; i < args.size() ; i++ ) {
      lastEvaluated = Evaluate( args.at( i ), depth+1 ) ;
    } // for()


    return lastEvaluated ;

  } // Eval_begin()

  TreeNode* Eval_cleanEnv( TreeNode* cmd, vector<TreeNode*> args ) {
    TreeNode* result = new TreeNode() ;

    mSymBindingMap.clear() ;
    mFunctionMap.clear() ;

    result->InitAtom( "environment cleaned", SYMBOL ) ;
    return result ;

  } // Eval_cleanEnv()

  TreeNode* Eval_exit( TreeNode* cmd, vector<TreeNode*> args ) {
    TreeNode* result = new TreeNode() ;
    result->InitAtom( "exit..", SYMBOL ) ;
    return result ;

  } // Eval_exit()

  TreeNode* Eval_let( TreeNode* expr, TreeNode* cmd, int depth ) {
    // 首先取得args (unevaled)
    vector<TreeNode*> args = GetUnevaledArgs( expr ) ;
    vector<TreeNode*> pairs ;

    // 檢查top level的arg數量
    if ( args.size() < 2 )
      throw OurSchemeException( FORMAT_ERR, expr, cmd ) ;

    TreeNode* arg1 = args.at( 0 ) ;
    // ERROR::arg1是一個非()的atom
    if ( arg1->NodeType() == ATOM_NODE && arg1->TokenType() != NIL )
      throw OurSchemeException( FORMAT_ERR, expr, cmd ) ;

    // 檢查arg1的error
    // 順便收集sub-expressions
    
    for ( TreeNode* peek = arg1 ; peek != NULL ; peek = peek->mRight ) {
      if ( peek->NodeType() == ATOM_NODE && peek->TokenType() != NIL )
        throw OurSchemeException( FORMAT_ERR, expr, cmd ) ;
      
      if ( peek->NodeType() == ATOM_NODE ) // 右下角的nil atom
        ;
      else {
        pairs.push_back( peek->mLeft ) ;

        // 檢查pair的合法
        TreeNode* pair = peek->mLeft ;
        // pair非pair
        if ( pair->mRight == NULL || pair->mRight->NodeType() != CONS_NODE )
          throw OurSchemeException( FORMAT_ERR, expr, cmd ) ;
        // 檢查pair左邊是不是symbol
        else if ( pair->mLeft == NULL || pair->mLeft->NodeType() != ATOM_NODE ||
                  pair->mLeft->TokenType() != SYMBOL )
          throw OurSchemeException( FORMAT_ERR, expr, cmd ) ;
        
        // 檢查pair的non-list 和 數量
        int counterForElement = 0 ;
        for ( TreeNode* nextRight = pair ; nextRight != NULL ; nextRight = nextRight->mRight ) {
          // 檢查pair的non-list
          if ( nextRight->NodeType() == ATOM_NODE && nextRight->TokenType() != NIL )
            throw OurSchemeException( FORMAT_ERR, expr, cmd ) ;

          // 要避免加到nil atom
          if ( nextRight->NodeType() == CONS_NODE )
            counterForElement ++ ;

        } // for()

        // symbol + 1*expr = 2
        if ( counterForElement != 2 )
          throw OurSchemeException( FORMAT_ERR, expr, cmd ) ;
      } // else

    } // for()

    // biding pairs的值
    vector<BoundSet*> evaledSubexpr ;
    for ( int i = 0 ; i < pairs.size() ; i++ ) {
      TreeNode* current = pairs.at( i ) ;
      TreeNode* sym = current->mLeft ;
      TreeNode* subExpr = current->mRight->mLeft ;

      
      string symStr = sym->Content() ;
      BoundSet* bs = new BoundSet() ;
      bs->mName = symStr ;
      bs->mBinding = Evaluate( subExpr, depth+2 ) ;
      // cout << "binding sym: " << symStr << endl ;
      evaledSubexpr.push_back( bs ) ;
    } // for()

    for ( int i = 0 ; i < evaledSubexpr.size() ; i++ ) {
      mLocalVarSet.push_back( evaledSubexpr.at( i ) ) ;
    } // for()

    // eval arg2~argn
    TreeNode* lastEvaled ;
    for ( int i = 1 ; i < args.size() ; i++ ) {
      lastEvaled = Evaluate( args.at( i ), depth+1 ) ;
    } // for()

    for ( int i = 0 ; i < pairs.size() ; i++ ) {
      // pop
      mLocalVarSet.pop_back() ;
    } // for()

    return lastEvaled ;

  } // Eval_let()

  TreeNode* Eval_lambda( TreeNode* expr, TreeNode* cmdNode ) {
    TreeNode* result = new TreeNode() ;
    TreeNode* definition ; // lambda expression的部分
    vector<TreeNode*> args ;
    EnvFunction* lambdaFunction = new EnvFunction() ;

    definition = expr ;


    // definition format : ( lambda (0-or-more-SYM) 1-or-more-expr )
    args = GetUnevaledArgs( definition ) ;
    if ( args.size() < 2 )
      throw OurSchemeException( FORMAT_ERR, expr, cmdNode ) ;
    // arg1包含0或多個symbol 這些就是function的params
    TreeNode* arg1 = args.at( 0 ) ;
    vector<TreeNode*> params ;

    // 收集params並檢查error
    for ( TreeNode* peek = arg1 ; peek != NULL ; peek = peek->mRight ) {
      // 檢查non-list
      if ( peek->NodeType() == ATOM_NODE && peek->TokenType() != NIL )
        throw OurSchemeException( FORMAT_ERR, expr, cmdNode ) ;

      // 檢查left要是symbol
      if ( peek->NodeType() == CONS_NODE && peek->mLeft->TokenType() != SYMBOL )
        throw OurSchemeException( FORMAT_ERR, expr, cmdNode ) ;

      // 避免push到最後那個nil
      if ( peek->NodeType() == CONS_NODE )
        params.push_back( peek->mLeft ) ;

    } // for


    // 無錯誤 組成一個function
    args.erase( args.begin() ) ;
    vector<TreeNode*> statements = args ;
    lambdaFunction->SetBody( statements ) ;
    lambdaFunction->SetParams( params ) ;
    result->InitAtom( "lambda", SYMBOL ) ;
    result->SetEvaled() ;
    mFunctionMap[ result ] = lambdaFunction ;

    return result ;

  } // Eval_lambda()

  TreeNode* Eval_func( TreeNode* mainExpr, TreeNode* funcNode, int depth ) {

    // 先取得function node
    EnvFunction* func = mFunctionMap[ funcNode ] ;
    vector<TreeNode*> params = func->GetParams() ;
    vector<TreeNode*> statements = func->GetBody() ;
    int paramsNum = params.size() ;

    vector<TreeNode*> args = GetUnevaledArgs( mainExpr ) ;

    // 檢查args數量
    if ( paramsNum != args.size() )
      throw OurSchemeException( NUM_OF_ARGS, funcNode->Content() ) ;

    // 開始做binding
    vector<BoundSet*> evaledArgs ;
    for ( int i = 0 ; i < paramsNum ; i++ ) {
      string param = params.at( i )->Content() ;
      TreeNode* evaled = Evaluate( args.at( i ), depth+1 ) ;
      BoundSet* bs = new BoundSet( param, evaled ) ;
      evaledArgs.push_back( bs ) ;
    } // for()

    for ( int i = 0 ; i < paramsNum ; i++ ) {
      mLocalVarSet.push_back( evaledArgs.at( i ) ) ;
    } // for()
    
    // 依序eval
    TreeNode* lastEval = NULL ;
    for ( int i = 0 ; i < statements.size() ; i++ ) {
      lastEval = Evaluate( statements.at( i ), depth+1 ) ;
    } // for()

    if ( lastEval == NULL )
      throw OurSchemeException( NO_RETURN_VAL, mainExpr ) ;

    // 消除loval var的binding
    for ( int i = 0 ; i < paramsNum ; i++ ) {
      mLocalVarSet.pop_back() ;
    } // for()


    return lastEval ;

  } // Eval_func()
  
  void Result_Printer( TreeNode* expr ) {
    if ( expr == NULL )
      cout << "expr is NULL..." ;

    Result_PrettyPrinter( expr, true, 0 ) ;
  } // Result_Printer()

  void Result_PrettyPrinter( TreeNode* current, bool expr, int spaces ) {
    
    string spaceStr = "" ;
    for ( int i = 0 ; i < spaces ; i ++ ) {
      spaceStr += " " ;
    } // for()

    // 要印括號
    if ( expr && current->NodeType() == CONS_NODE ) {
      
      cout << "( " ;
      Result_PrettyPrinter( current->mLeft, true, spaces+2 ) ;
      
      // TODO
      if ( current->mRight->NodeType() == ATOM_NODE &&
           current->mRight->TokenType() != NIL ) {
        cout << spaceStr << "  .\n" << spaceStr << "  " ;
        Result_PrettyPrinter( current->mRight, false, spaces ) ;
      } // if()
      else if ( current->mRight->NodeType() == CONS_NODE ) {
        cout << spaceStr << "  " ;
        Result_PrettyPrinter( current->mRight, false, spaces ) ;
      } // else if()
      else {
      ;
      } // else

      cout << spaceStr << ")" << endl ;

      return ;
    } // if()

    // cons-node
    else if ( current->NodeType() == CONS_NODE ) {
      Result_PrettyPrinter( current->mLeft, true, spaces+2 ) ;
      // 不印dot
      if ( current->mRight->NodeType() == CONS_NODE ) {
        cout << spaceStr << "  " ;
        Result_PrettyPrinter( current->mRight, false, spaces ) ;
      } // if()

      // 要印dot 右邊不能是nil
      else if ( current->mRight->TokenType() != NIL ) {
        cout << spaceStr << "  ." << endl << spaceStr << "  " ;
        Result_PrettyPrinter( current->mRight, false, spaces ) ;
      } // else if()
      else {
        ;
      } // else

      return ;

    } // else if()
    // ATOM-NODE
    else {

      if ( current->GetEvalFlag() && ( HasCmd( current->Content() ) ||
                                       HasFunction( GetBinding( current->Content() ) ) ) ) {
        string s = "#<procedure " + current->Content() + ">" ;
        cout << s << endl ;
      } // if()
      else if ( HasBinding( current->Content() ) && current->TokenType() == SYMBOL ) {
        string s = current->Content() + " defined" ;
        cout << s << endl ;
      } // else if()

      else if ( current->TokenType() == INT )
        cout << current->IntValue() << endl ;
      else if ( current->TokenType() == FLOAT )
        printf( "%.3f\n", current->FloatValue() ) ;
      else if ( current->TokenType() == NIL )
        cout << "nil" << endl ;
      else
        cout << current->Content() << endl ;
      return ;
    } // else

    return ;

  } // Result_PrettyPrinter()

}; // class Environment


// =======全域函式=======

void PrettyPrinter( TreeNode* current, bool expr, int spaces ) {
    
  string spaceStr = "" ;
  for ( int i = 0 ; i < spaces ; i ++ ) {
    spaceStr += " " ;
  } // for()

  // 要印括號
  if ( expr && current->NodeType() == CONS_NODE ) {
    
    cout << "( " ;
    PrettyPrinter( current->mLeft, true, spaces+2 ) ;
    
    // TODO
    if ( current->mRight->NodeType() == ATOM_NODE &&
         current->mRight->TokenType() != NIL ) {
      cout << spaceStr << "  .\n" << spaceStr << "  " ;
      PrettyPrinter( current->mRight, false, spaces ) ;
    } // if()
    else if ( current->mRight->NodeType() == CONS_NODE ) {
      cout << spaceStr << "  " ;
      PrettyPrinter( current->mRight, false, spaces ) ;
    } // else if()
    else {
    ;
    } // else

    cout << spaceStr << ")" << endl ;

    return ;
  } // if()

  // cons-node
  else if ( current->NodeType() == CONS_NODE ) {
    PrettyPrinter( current->mLeft, true, spaces+2 ) ;
    // 不印dot
    if ( current->mRight->NodeType() == CONS_NODE ) {
      cout << spaceStr << "  " ;
      PrettyPrinter( current->mRight, false, spaces ) ;
    } // if()

    // 要印dot 右邊不能是nil
    else if ( current->mRight->TokenType() != NIL ) {
      cout << spaceStr << "  ." << endl << spaceStr << "  " ;
      PrettyPrinter( current->mRight, false, spaces ) ;
    } // else if()
    else {
      ;
    } // else

    return ;

  } // else if()
  // ATOM-NODE
  else {
    if ( current->TokenType() == INT )
      cout << current->IntValue() << endl ;
    else if ( current->TokenType() == FLOAT )
      printf( "%.3f\n", current->FloatValue() ) ;
    else if ( current->TokenType() == NIL )
      cout << "nil" << endl ;
    else
      cout << current->Content() << endl ;
    return ;
  } // else

  return ;

} // PrettyPrinter()

void Printer( TreeNode* expr ) {
  if ( expr == NULL )
    cout << "expr is NULL..." ;

  PrettyPrinter( expr, true, 0 ) ;
} // Printer()

// =======主程式=======

int main()
{
  int gNum = 0 ;
  TreeNode* exprPtr = NULL ;
  TreeNode* resultExpr = NULL ;
  Environment* env = new Environment() ;
  Parser* parser = new Parser() ;
  bool end = false ;

  cin >> gNum ;

  cout << "Welcome to OurScheme!\n"  ;
  cout << endl << "> " ;

  while ( ! end ) {
    
    parser->ReadSExp() ;
    gEndProgramFlag = parser->CheckExit() ;
    if ( ! ( gSyntaxErrorFlag || gEndOfFileFlag || gEndProgramFlag ) ) {
      exprPtr = parser->Build() ;
      // Printer( exprPtr ) ;
      try {
        resultExpr = env->Evaluate( exprPtr, 0 ) ;
        env->Result_Printer( resultExpr ) ;
      } catch ( OurSchemeException e ) {
        cout << e.mErrorMessage ;
        if ( e.mExprToPrint != NULL )
          env->Result_Printer( e.mExprToPrint ) ;
        else
          cout << endl ;

      } // catch()

      cout << endl ;
      cout << "> " ;
    } // if()
    
    

    if ( gEndProgramFlag || gEndOfFileFlag )
      end = true ;
   
    if ( gSyntaxErrorFlag ) {
      cout << gErrorMessage << endl  ;
      cout << endl << "> " ;
      // cout << endl ;
    } // if()

    
    // Reset
    parser->Reset() ;
    delete exprPtr ;
    exprPtr = NULL ;
    gSyntaxErrorFlag = false ;

  
  } // while()

  if ( gEndOfFileFlag )
    cout << "ERROR (no more input) : END-OF-FILE encountered"  ;
  
  cout << endl << "Thanks for using OurScheme!"  ;

} // main()

