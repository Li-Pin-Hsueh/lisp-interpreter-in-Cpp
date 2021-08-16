// ===標頭檔區===
# include <iostream>
# include <string>
# include <sstream>
# include <ctype.h>
# include <vector>

using namespace std ;

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
    string result = "Line " + s1.str() + ", Column " + s2.str() ;
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
        GetChar() ;
        // 設定Erroe Message
        gSyntaxErrorFlag = true ;
        gErrorMessage = "ERROR (no closing quote) : END-OF-LINE encountered at line " + s1.str() ;
        gErrorMessage +=  ", column " + s2.str() ;
        
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
        else
          mCurrentToken = new Token( line, col, tokenString, STRING ) ;

        return ;
      } // else if()
      // semicolon ( starts a line-comment )
      else if ( cin.peek() == ';' ) {
        while ( cin.peek() != '\n' && cin.peek() != -1 )
          GetChar() ;
        // 如果遇到EOF 不要讀掉
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

public:
  TreeNode* mLeft ;
  TreeNode* mRight ;

  TreeNode() {
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
  } ;

  void InitAtom( string s, TokenType type ) {
    // TODO
    mNodeType = ATOM_NODE ;
    mTokenType = type ;
    mContent = s ;
    mLeft = NULL ;
    mRight = NULL ;
    // TODO : set value
    if ( type == INT ) {

    } // if()
    else if ( type == FLOAT ) {

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
    Token cToken = mTokens.at(0) ;
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
  
  void PrettyPrinter( TreeNode* current, bool expr, int spaces ) {
    
    string spaceStr = "" ;
    for ( int i = 0 ; i < spaces ; i ++ ) {
      spaceStr += " " ;
    } // for()

    // 判斷是否要印括號
    if ( current->NodeType() == CONS_NODE && expr ) {
      // 假expr
      if ( current->mRight->NodeType() == ATOM_NODE &&
         current->mRight->TokenType() == NIL )
        expr = false ;

    } // if()

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
      if ( current->mRight->NodeType() == CONS_NODE ) {
        cout << spaceStr << "  " ;
        PrettyPrinter( current->mRight, false, spaces ) ;
      } // if()
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
      cout << current->Content() << endl ;
      return ;
    } // else

    return ;

  } // PrettyPrinter()

public:
  Parser() {
    mLexer = new Lexer() ;
    mTokens.clear() ;
    mHeadPtr = NULL ;
  } // Parser()
  
  // 遞迴方式印出樹形
  void Printer() {
    PrettyPrinter( mHeadPtr, true, 0 ) ;
  } // Printer()
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
        gErrorMessage = "ERROR (unexpected token at " + nextToken->GetPosInfoAsString() ;
        gErrorMessage +=  ") : " + nextToken->GetText() ;
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
  void Build() {
    if ( mHeadPtr == NULL ) {
      mHeadPtr = new TreeNode() ;
      ParseEXPR( mHeadPtr ) ;

    } // if()
    else {
      cout << "Head pointer is not NULL..." << endl ;
    } // else

  } // Build()

}; // class Parser

// =======測試程式-Lexer=======
int Testbench_Lexer() {
  Lexer* lx = new Lexer() ;
  Token* token = new Token() ;
  bool end = false ;
  while ( ! end ) {
    token = lx->PeekToken() ;
    if ( token == NULL ) {
      if ( gSyntaxErrorFlag )
        cout << gErrorMessage << endl ;
      else if ( gEndOfFileFlag )
        cout << "ERROR (no more input) : END-OF-FILE encountered" << endl ;
      else
        cout << "ERROR[Testbench_Lexer]: There's no flag have been set..." << endl ;

      end = true ;
    } // if()
    else {
      cout << endl << token->ToString() << endl ;
      lx->GetToken() ;
    } // else()
  } // while()

  cout << "End of Test Bench 1 ..." << endl ;
  return 0 ;
} // Testbench_Lexer()
// =======測試程式-Parser=======
int TestBench_Syntax_Parser() {
  
  Parser* p ;
  while ( ! ( gSyntaxErrorFlag || gEndOfFileFlag ) )
  {
    p = new Parser() ;
    p->ReadSExp() ;
    if ( ! ( gSyntaxErrorFlag || gEndOfFileFlag ) ) {
      p->Build() ;
      p->Printer() ;
    } // if()

    delete p ;
  }
  
  return 0 ;
} // TestBench_Syntax_Parser()
// =======主程式=======

int main()
{
  
  cout << "Welecome!" << endl ;
  // test bench 1
  // Testbench_Lexer() ;
  // test bench 2
  TestBench_Syntax_Parser() ;

  cout << "\nEnd of program..." << endl ;
} // main()

