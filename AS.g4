grammar AS;

file: stat* EOF;

stat: exp '=' exp #assignstat
    | exp '.' ID '(' explist ')' #membercallstat
    | ID '(' explist ')' #funccallstat
    | 'if' exp stat ('else if' exp stat)* ('else' els=stat)? #condstat
    | '{' stat* '}' #blockstat
    | 'while' exp stat #whilestat
    | 'return' exp? #returnstat
    ;

exp
    // literals
    : 'true'                            #trueexp
    | 'false'                           #falseexp
    | INT                               #intexp
    | FLOAT                             #floatexp
    | STRING                            #stringexp
    | ID                                #idexp
    // object creation
    | '{' (ID '=' exp)* '}'            #mapdef
    | 'function' '(' idlist? ')' stat #functiondef
    // functions
    | ID '(' explist? ')'     #funccallexp
    | exp '.' ID '(' explist? ')'     #membercallexp
    // access
    | exp '.' ID                        #memberexp
    // unop
    | op=('-' | 'not') exp              #unaryexp
    // binop
    | exp op=('*' | '/' | '%') exp      #multiplicativeexp
    | exp op=('+' | '-') exp            #additiveexp
    | exp op=('<=' | '<' | '>' | '>=') exp #relationexp
    | exp op=('==' | '!=' ) exp            #comparisonexp
    | exp op='and' exp                     #andexp
    | exp op='or' exp                      #orexp
    // ternary
    | exp 'if' exp 'else' exp           #ternaryexp
    // parenthesis
    | '(' exp ')'                       #parenexp
    ;

explist: exp (',' exp)*;
idlist : ID (',' ID)*;

ID
    : [a-zA-Z_] [a-zA-Z_0-9]*
    ;

INT
    : [0-9]+
    ;

FLOAT
    : [0-9]+ '.' [0-9]* 
    | '.' [0-9]+ 
    ;

STRING
    : '"' (~('"'))* '"'
    | '\'' (~('\''))* '\''
    ;

COMMENT
    : '//' ~[\r\n]* -> skip
    ;

SPACE
    : [ \t\r\n] -> skip
    ;

OTHER
    : . 
    ;