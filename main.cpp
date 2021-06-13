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

enum NodeType
{
  INIT_NODE = -1, ATOM_NODE, QUOTE_NODE, LP_NODE
};

enum TokenType
{
  INIT_TYPE = -1, LP, RP, INT, FLOAT, STRING, DOT, NIL, T, QUOTE, SYMBOL
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
  } // GetLineAsString()

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
    TreeNode *mLeft = NULL ;
    TreeNode *mRight = NULL ;

    TreeNode()
    {
      mContent = "" ;
      mNodeType = INIT_NODE ;
      mTokenType = INIT_TYPE ;
      mLeft = NULL ;
      mRight = NULL ;
    } // TreeNode()

    TreeNode( NodeType nType, Token token )
    {
      mContent = token.GetText() ;
      mNodeType = nType ;
      mTokenType = token.GetType() ;
      mLeft = NULL ;
      mRight = NULL ;
    } // TreeNode()

    string ToString()
    {
      stringstream s1,s2 ;
      s1 << mNodeType ;
      s2 << mTokenType ;
      string result = "[Content]: " + mContent ;
      result += " [Node Type]: " + s1.str() ;
      result += " [Token Type]: " + s2.str() ;
      return result ;
    } // ToString()

    void ModifyContent( string s, NodeType nT, TokenType tT )
    {
      mContent = s ;
      mNodeType = nT ;
      mTokenType = tT ;
      mLeft = NULL ;
      mRight = NULL ;
    } // ModifyContent()

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
    if ( mCurrentToken == NULL ) gEndOfFile = true ;
    return mCurrentToken ;
  } // PeekToken()

  /* 重新調整Counter的位置 */
  void AdjustCounter()
  {
    /* 當Parser印出SEXPR後 如果該行只有space或tab 就要全部跳過*/
    /* 從下一行才開始算第一行*/
    while ( cin.peek() != '\n' && cin.peek() != -1 ) {
      // 如果有valid input 不能讀掉
      if ( !isspace( cin.peek() ) ) return ;
      // 遇到comment 讀掉整行
      if ( cin.peek() == ';' )
        while ( cin.peek() != '\n') GetChar() ;
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
  /* Get A Token and Push into Vector.*/
  void Eat()
  {
    Token *t = mLexer->NextToken() ;
    mTokens.push_back( *t ) ;
    mCurrentToken = t ;
  }

  /* Parse a Atom */
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

  /* Parse a S-Expression */
  void Parse_SEXPR()
  {
    // Always peek token first until syntax valid
    // Always check EOF when peek a token
    Token *pToken = mLexer->PeekToken() ;
    if ( gEndOfFile || gStringNotColsedError || gSyntaxError ) return ;

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
            !( gSyntaxError || gStringNotColsedError || gEndOfFile ) ) {
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
    if (pToken->GetType() == RP ) {
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

  /* Build a tree-like structur */
  void Start_BuildTree( TreeNode **current )
  {
    Token currentToken ;
    bool dot = false ;
    // Copy and erase first element of vector
    if ( !mTokens.empty() ) {
      currentToken = mTokens.at( 0 ) ;
      mTokens.erase( mTokens.begin(), mTokens.begin()+1 ) ;
    } // if()

    else {
      cout << "Vector is empty..." << endl ;
      return ;
    } // else

    /* 如果Token是左括號 */
    if ( currentToken.GetType() == LP ) {
      // 建立新的LP_NODE
      TreeNode *newNode = new TreeNode( LP_NODE, currentToken ) ;
      *current = newNode ;
      // 檢查是不是特殊case -> ()
      Token nextToken = mTokens.at( 0 ) ;
      if ( nextToken.GetType() == RP ) {
        // 把右括號erase掉
        if ( mTokens.size() <= 0 ) cout << "ERROE Erase()" << endl ;
        mTokens.erase( mTokens.begin(), mTokens.begin()+1 ) ;
        newNode->ModifyContent( "()", ATOM_NODE, NIL ) ;
        return ;   
      } // if()

      // Left-Recursive
      Left_BuildTree( &( mHeadPtr->mLeft ) ) ;
      // Erase DOT token
      if ( mTokens.size() > 0 ) {
        nextToken = mTokens.at( 0 ) ;
        if ( nextToken.GetType() == DOT ) {
          mTokens.erase( mTokens.begin(), mTokens.begin()+1 ) ;
          dot = true ;
        } // if()
      }
      // Right-Recursive
      Right_BuildTree( &( mHeadPtr->mRight ), dot ) ;
      // Erase RP token.
      if ( mTokens.size() > 0 ) {
        nextToken = mTokens.at( 0 ) ;
        if ( nextToken.GetType() == RP )
          mTokens.erase( mTokens.begin(), mTokens.begin()+1 ) ;
        else cout << "ERROR: must be RP..." << endl ;
      }
      else
        cout << "ERROR: Vector is empty..." << endl ;
    } // if()

    /* 如果Token是QUOTE*/
    else if ( currentToken.GetType() == QUOTE ) {
      // 創造一個INIT NODE
      TreeNode *newNode = new TreeNode( ) ;
      *current = newNode ;
      // 左邊放QUOTE 直接進右邊
      newNode->mLeft = new TreeNode( QUOTE_NODE, currentToken ) ;
      // Recursive right
      Right_BuildTree( &( mHeadPtr->mRight ), dot ) ;
    } // else if()

    /* 如果Token是ATOM */
    else {
      TreeNode *newNode = new TreeNode( ATOM_NODE, currentToken ) ;
      *current = newNode ;
    } // else()

    // Debug console
    // cout << "Debug mode: " << mHeadPtr->ToString() << endl ;

  } // BuildTree()

  /* Recursive function for left node */
  void Left_BuildTree( TreeNode **current )
  {
    // TODO
    Token currentToken ;
    bool dot = false ;
    // Copy and erase first element of vector
    if ( !mTokens.empty() ) {
      currentToken = mTokens.at( 0 ) ;
      mTokens.erase( mTokens.begin(), mTokens.begin()+1 ) ;
    } // if()

    else {
      cout << "Vector is empty..." << endl ;
      return ;
    } // else

    /* 如果Token是左括號 */
    if ( currentToken.GetType() == LP ) {
      // 建立新的LP_NODE
      TreeNode *newNode = new TreeNode( LP_NODE, currentToken ) ;
      *current = newNode ;
      // 檢查是不是特殊case -> ()
      Token nextToken = mTokens.at( 0 ) ;
      if ( nextToken.GetType() == RP ) {
        // 把右括號erase掉
        if ( mTokens.size() <= 0 ) cout << "ERROE Erase()" << endl ;
        mTokens.erase( mTokens.begin(), mTokens.begin()+1 ) ;
        newNode->ModifyContent( "()", ATOM_NODE, NIL ) ;
        return ;   
      } // if()

      // Left-Recursive
      Left_BuildTree( &( newNode->mLeft ) ) ;
      // Erase DOT token
      if ( mTokens.size() > 0 ) {
        nextToken = mTokens.at( 0 ) ;
        if ( nextToken.GetType() == DOT ) {
          mTokens.erase( mTokens.begin(), mTokens.begin()+1 ) ;
          dot = true ;
        } // if()
      }
      // Right-Recursive
      Right_BuildTree( &( newNode->mRight ), dot ) ;
      // Erase RP token.
      if ( mTokens.size() > 0 ) {
        nextToken = mTokens.at( 0 ) ;
        if ( nextToken.GetType() == RP )
          mTokens.erase( mTokens.begin(), mTokens.begin()+1 ) ;
        else cout << "ERROR: must be RP..." << endl ;
      }
      else
        cout << "ERROR: Vector is empty..." << endl ;
    } // if()

    /* 如果Token是QUOTE*/
    else if ( currentToken.GetType() == QUOTE ) {
      // 創造一個INIT NODE
      TreeNode *newNode = new TreeNode( ) ;
      *current = newNode ;
      // 左邊放QUOTE 直接進右邊
      newNode->mLeft = new TreeNode( QUOTE_NODE, currentToken ) ;
      // Recursive right
      Right_BuildTree( &( newNode->mRight ), dot ) ;
    } // else if()

    /* 如果Token是ATOM */
    else {
      TreeNode *newNode = new TreeNode( ATOM_NODE, currentToken ) ;
      *current = newNode ;
    } // else()
    
    return ;

  } // Left_BuildTree()

  /* Recursive function for right node */
  void Right_BuildTree( TreeNode **current, bool dot )
  {
    // TODO
    Token currentToken ;
    // Copy and erase first element of vector
    // 注意 這裡要確定不是RP才Erase()
    if ( !mTokens.empty() ) {
      currentToken = mTokens.at( 0 ) ;
    } // if()

    else {
      cout << "Vector is empty..." << endl ;
      return ;
    } // else

    /* 如果Token是RP -> nil*/
    if ( currentToken.GetType() == RP ) {
      TreeNode *newNode = new TreeNode() ;
      newNode->ModifyContent( "nil", ATOM_NODE, NIL ) ;
      *current = newNode ;
    } // if()

    /* 如果Token是LP */
    else if ( currentToken.GetType() == LP ) {
      // 建立新的LP_NODE
      mTokens.erase( mTokens.begin(), mTokens.begin()+1 ) ;
      TreeNode *newNode = new TreeNode( LP_NODE, currentToken ) ;
      *current = newNode ;
      // 檢查是不是特殊case -> ()
      Token nextToken = mTokens.at( 0 ) ;
      if ( nextToken.GetType() == RP ) {
        // 把右括號erase掉
        if ( mTokens.size() <= 0 ) cout << "ERROE Erase()" << endl ;
        mTokens.erase( mTokens.begin(), mTokens.begin()+1 ) ;
        newNode->ModifyContent( "()", ATOM_NODE, NIL ) ;
        return ;   
      } // if()

      // Left-Recursive
      Left_BuildTree( &( newNode->mLeft ) ) ;
      // Erase DOT token
      bool current_dot_flag = false ;
      if ( mTokens.size() > 0 ) {
        nextToken = mTokens.at( 0 ) ;
        if ( nextToken.GetType() == DOT ) {
          mTokens.erase( mTokens.begin(), mTokens.begin()+1 ) ;
          current_dot_flag = true ;
        } // if() 
      }
      // Right-Recursive
      Right_BuildTree( &( newNode->mRight ), current_dot_flag ) ;
      // Erase RP token.
      if ( mTokens.size() > 0 ) {
        nextToken = mTokens.at( 0 ) ;
        if ( nextToken.GetType() == RP )
          mTokens.erase( mTokens.begin(), mTokens.begin()+1 ) ;
        else cout << "ERROR: must be RP..." << endl ;
      }
      else
        cout << "ERROR: Vector is empty..." << endl ;
    } // else if()

    /* 如果Token是QUOTE*/
    else if ( currentToken.GetType() == QUOTE ) {
      // 創造一個INIT NODE
      mTokens.erase( mTokens.begin(), mTokens.begin()+1 ) ;
      TreeNode *newNode = new TreeNode( ) ;
      *current = newNode ;
      // 左邊放QUOTE 直接進右邊
      newNode->mLeft = new TreeNode( QUOTE_NODE, currentToken ) ;
      // Recursive right
      Right_BuildTree( &( newNode->mRight ), false ) ;
    } // else if()

    /* 如果Token是Else */
    else {
      if ( dot == true ) {
        mTokens.erase( mTokens.begin(), mTokens.begin()+1 ) ;
        TreeNode *newNode = new TreeNode( ATOM_NODE, currentToken ) ;
        *current = newNode ;
      } // if()
      else {
        TreeNode *newNode = new TreeNode( ) ;
        *current = newNode ;
        Left_BuildTree( &newNode->mLeft ) ;
        Right_BuildTree( &newNode->mRight, false ) ;
      } // else
    } // else

    return ;

  } // Right_BuildTree()

public:
  
  Parser()
  {
    mLexer = new Lexer() ;
    mHeadPtr = NULL ;
  } // Parser()
  /* Return true if no Syntax-error.*/
  bool ReadSExp()
  {
    if ( mLexer->PeekToken() == NULL ) return false ;
    else Parse_SEXPR() ;

    if ( gSyntaxError || gStringNotColsedError || gEndOfFile ) {
      while ( cin.peek() != '\n' ) cin.get() ;
      cin.get() ; 
      return false ;
    } // if()
    
    else {
      Start_BuildTree( &mHeadPtr ) ;
      SimplePrinter( mHeadPtr ) ;
      // cout << mHeadPtr->ToString() << endl ;
      return true ;
    } // else
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
  
  /* Reset Vector */
  void ResetTokenVector()
  {
    mTokens.clear() ;
    return ;
  } //ResetTokenVector()

  /* Reset mLexer */
  void ResetLexer()
  {
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
    SimplePrinter( currentPtr->mRight) ;
  } // SimplePrinter()

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

    if ( noSyntaxError ) { // S-Exp成立
      p->PrintTokens() ;
      cout << "> " ;
      p->ResetTokenVector() ;
      p->ResetLexer() ;
    } // if()
    else {
      // Print error message.
      cout << gErrorMessage << endl ;
      // Reset error flag.
      gSyntaxError = false, gStringNotColsedError = false ;
      // Reset Parser
      delete p ;
      Parser *p = new Parser() ;
      cout << "> " ;
    }

  } // while()

  if ( gEndOfFile )
    cout << "Error (no more input) : END-OF-FILE encountered" << endl ;

  cout << endl << "Thanks for using OurScheme!" << endl ;
} // main()
