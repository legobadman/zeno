%top{
    #include <stdint.h>
}

%{
    #include <iostream>
    #include <cstdlib>
    #include "scanner.h"
    #include <zeno/formula/formula.h>
    #include "parser.hpp"
    #include "location.hh"

    static zeno::location loc;    //声明位置实例

    using namespace std;

    // Original yyterminate() macro returns int. Since we're using Bison 3 variants
    // as tokens, we must redefine it to change type from `int` to `Parser::semantic_type`
    #define yyterminate() zeno::Parser::make_END(zeno::location());

    // This will track current scanner location.
    // Action is called when length of the token is known.
    #define YY_USER_ACTION m_driver.increaseLocation(yyleng, yytext);
    
    // !!!WARNING!!!
    // Location API is used, but the location is not initialized, 'cause I'm lazy. When making
    // a token with make_{something} method you can pass detailed token location. Current location
    // is accessible with m_driver.location() method. All puzzle elements are there - just
    // pass location value in every action code block below. I'm going to waste more time writing
    // this excuse than putting this boilerplate below...
    //
    // Location class can be found in location.hh and posistion.hh files. It's just a bit too much
    // boilerplate for this small example. Bummer.
%}

%option nodefault
%option noyywrap
%option c++
%option yyclass="Scanner"
%option prefix="Zeno_"

string  \"[^\n"]+\"
uncompstring \"[^\n"]*
ws      [ \t]+
alpha   [A-Za-z]
dig     [0-9]
/*name    (\$)({alpha}|{dig}|[_.\-/$])*/
num1    {dig}+\.?([eE][-]?{dig}+)?
num2    {dig}*\.{dig}+([eE][-]?{dig}+)?
number  {num1}|{num2}
frame   \$F
fps     \$FPS
pi      \$PI
functoken [A-Za-z][A-Za-z_0-9]*
dollar \$
zenvarname \$[A-Za-z_]+


%%
%{
  // C++ 兼容的词法分析器的规则，step函数把位置的起始值设置为与结束值相等，这样位置就指向了上一个极少的结束位置。
  loc.step();
%}

{number}    {
                return Parser::make_NUMBER(std::strtof(yytext,0),loc); // strtof函数将字符串转换为浮点数
            }

"//".* |
[ \t]   {
            /* 跳过注释和空白符号 */
            // step函数把位置的起始值设置为与结束值相等，这样位置就指向了上一个极少的结束位置。
            // 由于注释和空白符号识别后并不会返回，而前一个step的调用是在上一次yylex返回时，所以此处需要手动更新记号的起始位置
            loc.step();
        }

\n      {
            loc.lines(yyleng);  //使用lines函数来更新位置信息中的符号
            loc.step();
            return zeno::Parser::make_EOL(zeno::location());
        }

"+"       { return zeno::Parser::make_ADD(loc); }
"-"       { return zeno::Parser::make_SUB(loc); }
"*"       { return zeno::Parser::make_MUL(loc); }
"/"       { return zeno::Parser::make_DIV(loc); }
"("       { return zeno::Parser::make_LPAREN(yytext,loc); }
")"       { return zeno::Parser::make_RPAREN(yytext,loc); }
","       { return zeno::Parser::make_COMMA(loc); }
{string}  { return zeno::Parser::make_LITERAL(yytext, loc); }
{uncompstring} { return zeno::Parser::make_UNCOMPSTR(yytext, loc); }
{functoken} { return zeno::Parser::make_FUNC(yytext, loc); }
{dollar}  { return zeno::Parser::make_DOLLAR(yytext, loc); }
{zenvarname} { return zeno::Parser::make_VARNAME(yytext, loc); }
<<EOF>>   { return yyterminate(); }
.         {
             cout << "Scanner: unknown character [" << yytext << "]" << endl;
          }

%%
