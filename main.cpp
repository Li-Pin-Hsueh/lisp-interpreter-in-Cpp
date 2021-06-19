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

// 只有ATOM_NODE是LEAF NODE
enum NodeType
{
  INIT_NODE, ATOM_NODE, QUOTE_ROOT_NODE
};

enum TokenType
{
  INIT_TYPE, LP, RP, INT, FLOAT, STRING, DOT, NIL, T, QUOTE, SYMBOL
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
  Token()
  {
    mRow = -1 ;
    mCol = -1 ;
    mText = "" ;
    mType = INIT_TYPE ;
  } // Token()
  
  Token( int r, int c, string v )
  {

    TokenType t ;
    if ( v == "(" ) t = LP ;
    else if ( v == ")" ) t = RP ;
    else if ( v == "." ) t = DOT ;
    else if ( v == "'" ) t = QUOTE ;
    else if ( v == "nil" || v == "#f" )  t = NIL ;
    else if ( v == "t" || v == "#t" )  t = T ;
    else if ( IsInt( v ) ) t = INT ;
    else if ( IsFloat( v ) ) t = FLOAT ;
    else if ( v == "'" ) t = QUOTE ;
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
    string typeArray[] = {"init", "LP", "RP", "INT", "FLOAT",
                          "STRING", "DOT", "NIL", "T", "QUOTE", "SYMBOL"} ;
    stringstream s1, s2 ;
    s1 << mRow;
    s2 << mCol;
    string strRow = s1.str();
    string strCol = s2.str();
    string s = "[Line]" + strRow + " [Column]" + strCol + " [Type]" + typeArray[mType] + " [Text]" + mText ;
    return s;
  } // ToString()

  TokenType GetType()
  {
    return mType ;
  } // GetType()

  string GetPosInfoAsString()
  {
    stringstream s1, s2 ;
    s1 << mRow ;
    s2 << mCol ;
    string result = "Line " + s1.str() + " Column " + s2.str() ;
    return result ;
  } // GetPosInfoAsString()

  string GetText()
  {
    return mText ;
  } // GetText()

};  // class Token
// ============================================== 
// TreeNode class
class TreeNode
{
private:
  string mContent ;
  NodeType mNodeType;
  TokenType mTokenType ;

public:
  TreeNode *mLeft ;
  TreeNode *mRight ;
  /*
    node type = init, token type = init,
    mLeft = new TreeNode(), mRight = new TreeNode()
   */
  TreeNode()
  {
    mContent = "" ;
    mNodeType = INIT_NODE ;
    mTokenType = INIT_TYPE ;
    mLeft = NULL ;
    mRight = NULL ;
  } // TreeNode()

  void Init()
  {
    mContent = "" ;
    mNodeType = INIT_NODE ;
    mTokenType = INIT_TYPE ;
    mLeft = new TreeNode() ;
    mRight = new TreeNode() ;
  } // Init()

  string GetNodeContent()
  {
    return mContent ;
  } // GetNodeContent()

  NodeType GetNodeType()
  {
    return mNodeType ;
  } // GetNodeType()

  // Only way to Set a TreeNode
  // Automatically sets leaf's left and right nodes to NULL.
  void Set( string content, NodeType n, TokenType t )
  {
    mContent = content ;
    mNodeType = n ;
    mTokenType = t ;
    if ( mNodeType == ATOM_NODE ) {
      mLeft = NULL ;
      mRight = NULL ;
    } // if()
    else if ( mNodeType == QUOTE_ROOT_NODE ) {
      mLeft = NULL ;
    } // else()
    

  } // Set()

  string ToString()
  {
    stringstream s1, s2 ;
    s1 << mNodeType ;
    s2 << mTokenType ;
    string result = "[Content]: " + mContent ;
    result += " [Node Type]: " + s1.str() ;
    result += " [Token Type]: " + s2.str() ;
    return result ;
  } // ToString()

} ; // class TreeNode

// ============================================== 
// Lexer class
// Deal with : [ String not closed Error, EOF encounterred ]
class Lexer
{
private:
  int mColCounter ;
  int mRowCounter ;
  Token *mCurrentToken ;
  // Return true if the argument char is a Separator.
  bool IsSep( char c )
  {
    if ( c == '(' || c == ')' || c == '\"' || c == '\'' ||
         c == ';' || isspace( c ) )
      return true ;
    else
      return false ;
  } // IsSep()
  // Use cin.get() to read input. Do counter increment.
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
    // cout << "console: " << (char) result << endl ;
    return result ;
  } // GetChar()
  // Convert a Char to string type
  string ToString( char c )
  {
    stringstream s ;
    s << c ;
    return s.str() ;
  } // ToString()
  // Return a token string(as string type). Assign error message(String-Not-Closed-Error).
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
        if ( c == '\"' ) end = true ;
        stringstream ss ;
        ss << c ;
        result += ss.str() ;
      } // else

    } // while
    
    // Set String Error here.
    if ( result.length() == 1 || result.at( result.length()-1 ) != '\"' ) {
      // 注意 換行還沒讀 留給parser讀
      stringstream s1, s2 ;
      s1 << mRowCounter, s2 << mColCounter + 1 ;
      gStringNotColsedError = true ;
      gErrorMessage = "ERROR (no closing quote) : END-OF-LINE encountered at Line " + s1.str() +
                      ", Column " + s2.str() ;
      
      ResetCounter() ;

    } // if()
    
    return result ;
  } // StringHelper()
  // Return a Token or NULL(EOF)
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
  // Reset line and column counters 1 and 0.
  void ResetCounter()
  {
    mColCounter = 0 ;
    mRowCounter = 1 ;
  } // ResetCounter()
  // Return current Token or next Token()
  Token *NextToken()
  {
    if ( mCurrentToken == NULL )
      mCurrentToken = GetToken() ;

    Token *t = mCurrentToken ;
    mCurrentToken = NULL ;
    if ( t == NULL )  gEndOfFile = true ;
    return t ;
  } // NextToken()
  // Return current Token
  Token *PeekToken()
  {
    if ( mCurrentToken == NULL ) mCurrentToken = GetToken() ;
    if ( mCurrentToken == NULL ) gEndOfFile = true ;
    return mCurrentToken ;
  } // PeekToken()
  // 重新調整Counter的位置
  void AdjustCounter()
  {
    // 當Parser印出SEXPR後 如果該行只有space或tab 就要全部跳過
    // 從下一行才開始算第一行
    while ( cin.peek() != '\n' && cin.peek() != -1 ) {
      // 如果有valid input 不能讀掉
      if ( !isspace( cin.peek() ) ) return ;
      // 遇到comment 讀掉整行
      if ( cin.peek() == ';' )
        while ( cin.peek() != '\n' ) GetChar() ;
      else
        GetChar() ;
        
    } // while()

    // 讀掉換行並重設RowCounter
    GetChar() ;
    mRowCounter = 1 ;
    return ;

  } // AdjustCounter()

}; // class Lexer

// ============================================== 
// Parser class
// Deal with: [ Syntax Error ]
class Parser
{
private:
  Lexer *mLexer ;
  vector<Token> mTokens ;
  Token *mCurrentToken ;
  TreeNode *mHeadPtr ;
  // Get A Token and Push into Vector.
  void Eat()
  {
    Token *t = mLexer->NextToken() ;
    mTokens.push_back( *t ) ;
    mCurrentToken = t ;
  } // Eat()

  // Parse a Atom 
  void Parse_ATOM()
  {
    Token *pToken = mLexer->PeekToken() ;
    if ( pToken == NULL ) return ;

    TokenType t = pToken->GetType() ;
    if ( t == SYMBOL || t == INT || t == FLOAT ||
         t == STRING || t == NIL || t == T )
      Eat() ;
    else {
      gSyntaxError = true ;
      gErrorMessage = "ERROR (unexpected token) :" ;
      gErrorMessage += " atom or '(' expected when token at " ;
      gErrorMessage += pToken->GetPosInfoAsString() ;
      gErrorMessage += " is >>" + pToken->GetText() + "<<";
      Eat() ;
    } // else()

    return ;
  } // Parse_ATOM()

  // Parse a S-Expression
  void Parse_SEXPR()
  {
    // Always peek token first until syntax valid
    // Always check EOF when peek a token
    Token *pToken = mLexer->PeekToken() ;
    if ( gEndOfFile || gStringNotColsedError || gSyntaxError ) return ;
    // cout << pToken->ToString() << endl ;
    // Grammar
    // <S-exp> ::= LEFT-PAREN <S-exp> { <S-exp> } [ DOT <S-exp> ] RIGHT-PAREN
    if ( pToken->GetType() == LP ) {
      Eat() ;
      pToken = mLexer->PeekToken() ;
      if ( gEndOfFile || gStringNotColsedError || gSyntaxError ) return ;
      // 這裡比較特殊 多判斷一次看是不是()這個ATOM
      // Grammar
      // <ATOM>  ::= LEFT-PAREN RIGHT-PAREN
      if ( pToken->GetType() == RP ) {
        Eat() ;
        return ;
      } // if
    } // if()

    // Grammar
    // <S-exp> ::= QUOTE <S-exp>
    else if ( pToken->GetType() == QUOTE ) {
      Eat() ;
      Parse_SEXPR() ;
      return ;
    } // else if()

    // Grammar
    // <S-exp> ::= <ATOM>
    // <ATOM>  ::= SYMBOL | INT | FLOAT | STRING 
    //             | NIL | T | LEFT-PAREN RIGHT-PAREN
    else {
      Parse_ATOM() ;
      return ;
    } // else()

    // Continue withe the first Grammar
    // <S-exp> ::= LEFT-PAREN <S-exp> { <S-exp> } [ DOT <S-exp> ] RIGHT-PAREN
    //                        ^ Start from here
    Parse_SEXPR() ;
    if ( gSyntaxError || gStringNotColsedError || gEndOfFile ) return ; // Check error after parse function()

    // Continue
    pToken = mLexer->PeekToken() ;
    if ( gEndOfFile || gStringNotColsedError || gSyntaxError ) return ;
    
    while ( pToken->GetType() != DOT && pToken->GetType() != RP && 
            ! ( gSyntaxError || gStringNotColsedError || gEndOfFile ) ) {
              // Grammar
              // <S-exp> ::= LEFT-PAREN <S-exp> { <S-exp> } [ DOT <S-exp> ] RIGHT-PAREN
              //                                ^ Start from here ( repeat 1 or N times )
      Parse_SEXPR() ;
      pToken = mLexer->PeekToken() ;
      if ( gEndOfFile || gStringNotColsedError || gSyntaxError ) return ;
    } // while()

    if ( gSyntaxError || gStringNotColsedError || gEndOfFile ) return ; // Check error after parse function()
    

    // Grammar
    // <S-exp> ::= LEFT-PAREN <S-exp> { <S-exp> } [ DOT <S-exp> ] RIGHT-PAREN
    //                                              ^ Start from here ( 0 or 1 time )
    if ( pToken->GetType() == DOT ) {
      Eat() ;
      Parse_SEXPR() ;
      // Check error after parse function()
      if ( gSyntaxError || gStringNotColsedError || gEndOfFile ) return ;
    } // if()

    pToken = mLexer->PeekToken() ;
    if ( gEndOfFile || gStringNotColsedError || gSyntaxError ) return ;

    // Grammar
    // <S-exp> ::= LEFT-PAREN <S-exp> { <S-exp> } [ DOT <S-exp> ] RIGHT-PAREN
    //                                                            ^ Start from here
    if ( pToken->GetType() == RP ) {
      Eat() ;
      return ;
    } // if()
    else {
      // Eat() ;
      gSyntaxError = true ;
      gErrorMessage = "ERROR (unexpected token) : ')' expected " ;
      gErrorMessage += "when token at " ;
      gErrorMessage += pToken->GetPosInfoAsString() ;
      gErrorMessage += " is >>" ;
      gErrorMessage += pToken->GetText() ;
      gErrorMessage += "<<" ;
      return ;
    } // else

    
  } // Parse_SEXPR()

  // Recursivly build tree
  /*
    用單一的函式一併處理左右邊的遞迴
    一個新的NODE一律先長出兩邊的"骨架"
    共有三種NODE TYPE
    INIT_NODE, ATOM_NODE, QUOTE_NODE
   */
  void Build_SEXPR( TreeNode *current, bool settable )
  {
    // 檢查current是否為NULL
    if ( current == NULL ) {
      cout << "DEBUG ERROR: Current node is NULL..." << endl ;
      return ;
    } // if()
    else {
      current->Init() ;
    } // else
    
    if ( mTokens.empty() ) {
      cout << "DEBUG ERROR: Vector is empty..." << endl ;
      return ;
    } // if()

    // 取得第一個TOKEN
    Token currentToken = mTokens.at( 0 ) ;

    /*
      如果settable是false 代表此node一定是骨架
      但若current token為RP 要設nil atom
     */
    if ( ! settable && currentToken.GetType() != RP ) {
      // 0. 不嘗試取得LP 直接進left 左邊一律是settable
      Build_SEXPR( current->mLeft, true ) ;
      // 1. 嘗試取得DOT
      bool dot = false ;
      if ( ! mTokens.empty() && mTokens.at( 0 ).GetType() == DOT ) {
        dot = true ;
        mTokens.erase( mTokens.begin(), mTokens.begin() + 1 ) ;
      } // if()
      // 2. 進right 
      Build_SEXPR( current->mRight, dot ) ;
      return ;
    } // if()

    // not settable但是RP 要設定nil
    if ( ! settable && currentToken.GetType() == RP )
    {
      current->Set( "nil", ATOM_NODE, NIL ) ;
      return ;
    }

    /*
      Settable的node 要看能不能設定成atom或quote
     */

    // 特殊case: () ===> NIL
    if ( currentToken.GetType() == LP && mTokens.at( 1 ).GetType() == RP ) {
      // 一次erase掉LP和RP
      mTokens.erase( mTokens.begin(), mTokens.begin() + 2 ) ;
      // 設定成ATOM
      current->Set( "nil", ATOM_NODE, NIL ) ;
      return ;
    } // if

    // Regular SEXP
    else if ( currentToken.GetType() == LP ) {
      // 0. erase掉LP current設為SEXPR_NODE
      mTokens.erase( mTokens.begin(), mTokens.begin() + 1 ) ;
      // 1. 往左邊走
      Build_SEXPR( current->mLeft, true ) ;
      // 2. 取得DOT
      bool dot = false ;
      if ( ! mTokens.empty() && mTokens.at( 0 ).GetType() == DOT ) {
        mTokens.erase( mTokens.begin(), mTokens.begin() + 1 ) ;
        dot = true ;
      } // if()
      // 3. 往右走
      Build_SEXPR( current->mRight, dot ) ;
      // 4.取得RP
      if ( ! mTokens.empty() && mTokens.at( 0 ).GetType() == RP ) {
        mTokens.erase( mTokens.begin(), mTokens.begin() + 1 ) ;
      } // if()
      else  cout << "Can't get RP..." << endl ;
      return ;

    } // else if()

    // settable QUOTE
    else if ( currentToken.GetType() == QUOTE ) {
      // 0. erase QUOTE token
      mTokens.erase( mTokens.begin(), mTokens.begin() + 1 ) ;
      // 1. current設定成quote root 左邊會是NULL
      current->Set( "QUOTE ROOT", QUOTE_ROOT_NODE, QUOTE ) ;
      // 2.直接進右邊 always settable
      bool quoteFlag = true ;
      Build_SEXPR( current->mRight, quoteFlag ) ;
      return ;

    } // else if()

    // settable ATOM
    else {
      // 0. erase Token
      mTokens.erase( mTokens.begin(), mTokens.begin() + 1 ) ;
      // 1. current設定為ATOM
      current->Set( currentToken.GetText(), ATOM_NODE, currentToken.GetType() ) ;
      return ;

    } // else

    cout << "DEBUG ERROR: Unknow case..." << endl ;
    cout << currentToken.ToString() << endl ;
    return ; 

  } // Build_SEXPR

  void Recursive_Printer( TreeNode *current, bool printable, int nSpaces )
  {
    // TODO
    string str_spaces = "" ;
    for ( int i = 0 ; i < nSpaces ; i ++ ) {
      str_spaces += " " ;
    } // for()

    bool printDOT = false ;
    if ( current->GetNodeType() == INIT_NODE &&
         current->mRight->GetNodeType() == ATOM_NODE ) {
      if ( current->mRight->GetNodeContent() != "nil" )
        printDOT = true ;
    } // if()

    // Printable決定nil要不要印
    if ( ! printable && current->GetNodeType() == ATOM_NODE )
      return ;
    if ( printable && current->GetNodeType() == ATOM_NODE ) {
      cout << current->GetNodeContent() << endl ;
      // 後續要處理ATOM的format

    } // if()

    else if ( current->GetNodeType() == QUOTE_ROOT_NODE ) {
      // TODO
      cout << "( quote" << endl ;
      cout << str_spaces << "  " ;
      Recursive_Printer( current->mRight, true, nSpaces+2 ) ;
      cout << str_spaces << ")" << endl ;
    } // else if()

    else if ( printable && current->GetNodeType() == INIT_NODE ) {
      
      cout << "( " ;
      Recursive_Printer( current->mLeft, true, nSpaces+2 ) ;
      if ( printDOT ) {
        cout << str_spaces << "  " ;
        cout << "." << endl ;
        cout << str_spaces << "  " ;
        Recursive_Printer( current->mRight, true, nSpaces+2 ) ;
      } // if()
      else
        Recursive_Printer( current->mRight, false, nSpaces ) ;
      
      cout << str_spaces << ")" << endl ;

    } // else if()

    else {
      cout << str_spaces << "  " ;
      Recursive_Printer( current->mLeft, true, nSpaces+2 ) ;
      if ( printDOT ) {
        cout << str_spaces << "  ";
        cout << "." << endl ;
        cout << str_spaces << "  ";
        Recursive_Printer( current->mRight, true, nSpaces+2 ) ;
      } // if()
      else {
        Recursive_Printer( current->mRight, false, nSpaces ) ;
      } // else()
    } // else

    return ;

  } // Recursive_Printer()

  bool CheckExit()
  {
    if ( mTokens.size() != 3 )
      return false ;
    
    if ( mTokens.at( 0 ).GetType() == LP &&
         mTokens.at( 1 ).GetText() == "exit" &&
         mTokens.at( 2 ).GetType() == RP )
      return true ;

    return false ;
  } // CheckExit()


public:
  
  Parser()
  {
    mLexer = new Lexer() ;
    mHeadPtr = new TreeNode() ;
    mHeadPtr->Init() ;
  } // Parser()

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

  // Return the Vectoe which store tokens.
  vector<Token> GetTokensCopy()
  {
    vector<Token> copy ;
    for ( int i = 0 ; i < mTokens.size() ; i++ )
    {
      copy.push_back( mTokens.at( i ) ) ;
    } // for

    return copy ;
  } // GetTokensCopy()
  

  // Return true if no Syntax-error.
  bool ReadSExp()
  {
    if ( mLexer->PeekToken() == NULL ) return false ;
    else Parse_SEXPR() ;

    if ( gSyntaxError || gStringNotColsedError || gEndOfFile ) {
      while ( cin.peek() != '\n' ) cin.get() ;
      cin.get() ; 
      return false ;
    } // if()

    gExitFlag = CheckExit() ;
    if ( gExitFlag )  return true ;

    Build_SEXPR( mHeadPtr, true ) ;
    // gExitFlag = CheckExit( mHeadPtr ) ;
    // SimplePrinter( mHeadPtr ) ;
    // cout << mHeadPtr->ToString() << endl ;
    return true ;

  } // ReadSExp()
  
  // Reset Vector 
  void ResetTokenVector()
  {
    mTokens.clear() ;
    return ;
  } // ResetTokenVector()

  // Reset mLexer 
  void ResetLexer()
  {
    delete mLexer ;
    mLexer = new Lexer() ;
    mLexer->AdjustCounter() ;
    return ;
  } // ResetLexer()

  void SimplePrinter( TreeNode *currentPtr )
  {
    if ( currentPtr != NULL )
      cout << currentPtr->ToString() << endl;

    else
      return ;

    SimplePrinter( currentPtr->mLeft ) ;
    SimplePrinter( currentPtr->mRight ) ;
  } // SimplePrinter()

  void PrettyPrint()
  {
    Recursive_Printer( mHeadPtr, true, 0 ) ;
    return ;
  } // PrettyPrint()

}; // class Parser

// ============================================== 
// Main function

int main()
{

  Parser *p = new Parser() ;
  cout << "Welcome to OurScheme!\n" ;
  cout << "> " ;
  while ( !gExitFlag && !gEndOfFile )
  {
    bool noSyntaxError = p->ReadSExp() ;

    if ( noSyntaxError && !gExitFlag ) { // S-Exp成立
      // p->PrintVector() ;
      p->PrettyPrint() ;
      cout << "> " ;
      p->ResetTokenVector() ;
      p->ResetLexer() ;
    } // if()
    else if ( gExitFlag ) {
      ;
    } // else if()
    else {
      // Print error message.
      cout << gErrorMessage << endl ;
      // Reset error flag.
      gSyntaxError = false, gStringNotColsedError = false ;
      // Reset Parser
      delete p ;
      p = new Parser() ;
      cout << "> " ;
    } // else

  } // while()

  if ( gEndOfFile )
    cout << "Error (no more input) : END-OF-FILE encountered" << endl ;

  cout << endl << "Thanks for using OurScheme!" << endl ;
} // main()
