%{

//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <IceStorm/Parser.h>
#include <IceStorm/Grammar.h>

#if defined(_MSC_VER)
// '<' : signed/unsigned mismatch
#   pragma warning(disable:4018)
// 'initializing' : conversion from '__int64' to 'int', possible loss of data
#   pragma warning(disable:4244)

#   if defined(ICE_64)
//
// '=' : conversion from 'size_t' to 'int', possible loss of data
// The result of fread() is a size_t and gets inserted into an int
//
#       pragma warning(disable:4267)
#   endif
#endif

#if defined(__GNUC__)
#   pragma GCC diagnostic ignored "-Wsign-compare"
#endif

using namespace std;
using namespace Ice;
using namespace IceStorm;

#ifdef _MSC_VER
#   ifdef yywrap
#      undef yywrap
#      define yywrap() 1
#   endif
#   define YY_NO_UNISTD_H
#endif

#define YY_INPUT(buf, result, maxSize) parser->getInput(buf, result, maxSize)

namespace IceStorm
{

typedef std::map<std::string, int> StringTokenMap;
static StringTokenMap keywordMap;

void initScanner();

}
#define         YY_USER_INIT initScanner();

%}

WS      [ \t\v\f\r]
NL      [\n]
keyword [[:alpha:]]*

%option noyywrap
%option always-interactive

%%

"//" {
    // C++-style comment
    int c;
    do
    {
        c = yyinput();
    }
    while(c != '\n' && c != EOF);
}

"/*" {
    // C-style comment
    while(true)
    {
        int c = yyinput();
        if(c == '*')
        {
            int next = yyinput();
            if(next == '/')
            {
                break;
            }
            else
            {
                unput(next);
            }
        }
        else if(c == EOF)
        {
            parser->warning("EOF in comment");
            break;
        }
    }
}

{WS}*(\\{WS}*{NL})? {
    size_t len = strlen(yytext);
    for(size_t i = 0; i < len; ++i)
    {
        if(yytext[i] == '\\')
        {
            parser->continueLine();
        }
    }
}

{NL}|; {
    return ';';
}

\" {
    // "..."-type strings
    string s;
    while(true)
    {
        int c = yyinput();
        if(c == '"')
        {
            break;
        }
        else if(c == EOF)
        {
            parser->warning("EOF in string");
            break;
        }
        else if(c == '\\')
        {
            int next = yyinput();
            switch(next)
            {
                case '\\':
                case '"':
                {
                    s += static_cast<char>(next);
                    break;
                }

                default:
                {
                    s += static_cast<char>(c);
                    unput(next);
                }
            }
        }
        else
        {
            s += static_cast<char>(c);
        }
    }
    yylvalp->clear();
    yylvalp->push_back(s);
    return ICE_STORM_STRING;
}

\' {
    // '...'-type strings
    string s;
    while(true)
    {
        int c = yyinput();
        if(c == '\'')
        {
            break;
        }
        else if(c == EOF)
        {
            parser->warning("EOF in string");
            break;
        }
        else
        {
            s += c;
        }
    }
    yylvalp->clear();
    yylvalp->push_back(s);
    return ICE_STORM_STRING;
}

. {
    // Simple strings
    string s;
    s += yytext[0];
    while(true)
    {
        int c = yyinput();
        if(c == EOF)
        {
            break;
        }
        else if(isspace(c) || c == ';')
        {
            unput(c);
            break;
        }

        s += static_cast<char>(c);
    }

    yylvalp->clear();
    yylvalp->push_back(s);

    StringTokenMap::const_iterator pos = keywordMap.find(s);
    return pos != keywordMap.end() ? pos->second : ICE_STORM_STRING;
}

%%

namespace IceStorm {

//
// initScanner() fills the keyword map with all keyword-token pairs.
//

void
initScanner()
{
    keywordMap["help"] = ICE_STORM_HELP;
    keywordMap["quit"] = ICE_STORM_EXIT;
    keywordMap["exit"] = ICE_STORM_EXIT;
    keywordMap["current"] = ICE_STORM_CURRENT;
    keywordMap["create"] = ICE_STORM_CREATE;
    keywordMap["destroy"] = ICE_STORM_DESTROY;
    keywordMap["link"] = ICE_STORM_LINK;
    keywordMap["unlink"] = ICE_STORM_UNLINK;
    keywordMap["links"] = ICE_STORM_LINKS;
    keywordMap["topics"] = ICE_STORM_TOPICS;
    keywordMap["replica"] = ICE_STORM_REPLICA;
    keywordMap["subscribers"] = ICE_STORM_SUBSCRIBERS;
}

}
