/*	LEXICAL ANALYSER FOR MODULA-2	*/

#include "input.h"
#include <alloc.h>
#include "f_info.h"
#include "Lpars.h"
#include "class.h"
#include "param.h"
#include "idf.h"
#include "LLlex.h"

long str2long();
char *GetString();

struct token dot, aside;

static char *RcsId = "$Header$";

int
LLlex()
{
	/*	LLlex() plays the role of Lexical Analyzer for the parser.
		The putting aside of tokens is taken into account.
	*/
	if (ASIDE)	{	/* a token is put aside		*/
		dot = aside;
		ASIDE = 0;
	}
	else	{
		GetToken(&dot);
		if (DOT == EOI) DOT = -1;
	}

	return DOT;
}

int
GetToken(tk)
	register struct token *tk;
{
	char buf[(IDFSIZE > NUMSIZE ? IDFSIZE : NUMSIZE) + 1];
	register int ch, nch;

again:
	LoadChar(ch);
	if ((ch & 0200) && ch != EOI) {
		fatal("non-ascii '\\%03o' read", ch & 0377);
	}
	
	switch (class(ch))	{

	case STSKIP:
		goto again;

	case STNL:
		LineNumber++;
		goto again;

	case STGARB:
		if (040 < ch && ch < 0177)	{
			lexerror("garbage char %c", ch);
		}
		else	{
			lexerror("garbage char \\%03o", ch);
		}
		goto again;

	case STSIMP:
		if (ch == '(')	{
			LoadChar(nch);
			if (nch == '*')	{
				SkipComment();
				goto again;
			}
			else	{
				PushBack(nch);
			}
		}
		return tk->tk_symb = ch;

	case STCOMP:
		LoadChar(nch);
		switch (ch)	{

		case '.':
			if (nch == '.')	{
				return tk->tk_symb = UPTO;
			}
			PushBack(nch);
			return tk->tk_symb = ch;

		case ':':
			if (nch == '=')	{
				return tk->tk_symb = BECOMES;
			}
			PushBack(nch);
			return tk->tk_symb = ch;

		case '<':
			if (nch == '=')	{
				return tk->tk_symb = LESSEQUAL;
			}
			else
			if (nch == '>') {
				return tk->tk_symb = UNEQUAL;
			}
			PushBack(nch);
			return tk->tk_symb = ch;

		case '>':
			if (nch == '=')	{
				return tk->tk_symb = GREATEREQUAL;
			}
			PushBack(nch);
			return tk->tk_symb = ch;

		default :
			crash("bad STCOMP");
		}

	case STIDF:
	{
		register char *tg = &buf[0];
		register struct idf *id;

		do	{
			if (tg - buf < IDFSIZE) *tg++ = ch;
			LoadChar(ch);
		} while(in_idf(ch));

		if (ch != EOI)
			PushBack(ch);
		*tg++ = '\0';

		id = tk->TOK_IDF = str2idf(buf, 1);
		if (!id) fatal("Out of memory");
		return tk->tk_symb = id->id_reserved ? id->id_reserved : IDENT;
	}

	case STSTR:
		tk->TOK_STR = GetString(ch);
		return tk->tk_symb = STRING;

	case STNUM:
	{
		/*	The problem arising with the "parsing" of a number
			is that we don't know the base in advance so we
			have to read the number with the help of a rather
			complex finite automaton.
			Excuses for the very ugly code!
		*/
		register char *np = &buf[1];
					/* allow a '-' to be added	*/

		*np++ = ch;
		
		LoadChar(ch);
		while (is_oct(ch))	{
			if (np < &buf[NUMSIZE]) {
				*np++ = ch;
			}
			LoadChar(ch);
		}
		switch (ch) {
		case 'H':
Shex:			*np++ = '\0';
			/* Type is integer */
			tk->TOK_INT = str2long(&buf[1], 16);
			return tk->tk_symb = INTEGER;

		case '8':
		case '9':
			do {
				if (np < &buf[NUMSIZE]) {
					*np++ = ch;
				}
				LoadChar(ch);
			} while (is_dig(ch));

			if (is_hex(ch))
				goto S2;
			if (ch == 'H')
				goto Shex;
			if (ch == '.')
				goto Sreal;
			PushBack(ch);
			goto Sdec;

		case 'B':
		case 'C':
			if (np < &buf[NUMSIZE]) {
				*np++ = ch;
			}
			LoadChar(ch);
			if (ch == 'H')
				goto Shex;
			if (is_hex(ch))
				goto S2;
			PushBack(ch);
			ch = *--np;
			*np++ = '\0';
			/*
			 * If (ch == 'C') type is a CHAR
			 * else type is an INTEGER
			 */
			tk->TOK_INT = str2long(&buf[1], 8);
			return tk->tk_symb = INTEGER;

		case 'A':
		case 'D':
		case 'E':
		case 'F':
S2:
			do {
				if (np < &buf[NUMSIZE]) {
					*np++ = ch;
				}
				LoadChar(ch);
			} while (is_hex(ch));
			if (ch != 'H') {
				lexerror("H expected after hex number");
				PushBack(ch);
			}
			goto Shex;

		case '.':
Sreal:
			/*	This '.' could be the first of the '..'
				token. At this point, we need a look-ahead
				of two characters.
			*/
			LoadChar(ch);
			if (ch == '.') {
				/*	Indeed the '..' token
				*/
				PushBack(ch);
				PushBack(ch);
				goto Sdec;
			}

			/* a real constant */
			if (np < &buf[NUMSIZE]) {
				*np++ = '.';
			}

			if (is_dig(ch)) {
				/* 	Fractional part
				*/
				do {
					if (np < &buf[NUMSIZE]) {
						*np++ = ch;
					}
					LoadChar(ch);
				} while (is_dig(ch));
			}
			
			if (ch == 'E') {
				/*	Scale factor
				*/
				if (np < &buf[NUMSIZE]) {
					*np++ = 'E';
				}
				LoadChar(ch);
				if (ch == '+' || ch == '-') {
					/*	Signed scalefactor
					*/
					if (np < &buf[NUMSIZE]) {
						*np++ = ch;
					}
					LoadChar(ch);
				}
				if (is_dig(ch)) {
					do {
						if (np < &buf[NUMSIZE]) {
							*np++ = ch;
						}
						LoadChar(ch);
					} while (is_dig(ch));
				}
				else {
					lexerror("bad scale factor");
				}
			}

			PushBack(ch);

			if (np == &buf[NUMSIZE + 1]) {
				lexerror("floating constant too long");
				tk->TOK_REL = Salloc("0.0", 5);
			}
			else {
				tk->TOK_REL = Salloc(buf, np - buf) + 1;
			}
			return tk->tk_symb = REAL;

		default:
			PushBack(ch);
Sdec:
			*np++ = '\0';
			/* Type is an integer */
			tk->TOK_INT = str2long(&buf[1], 10);
			return tk->tk_symb = INTEGER;
		}
		/*NOTREACHED*/
	}

	case STEOI:
		return tk->tk_symb = EOI;

	case STCHAR:
	default:
		crash("bad character class %d", class(ch));
	}
}

char *
GetString(upto)
{
	register int ch;
	int str_size;
	char *str = Malloc(str_size = 32);
	register int pos = 0;
	
	LoadChar(ch);
	while (ch != upto)	{
		if (class(ch) == STNL)	{
			lexerror("newline in string");
			LineNumber++;
			break;
		}
		if (ch == EOI) {
			lexerror("end-of-file in string");
			break;
		}
		str[pos++] = ch;
		if (pos == str_size)	{
			str = Srealloc(str, str_size += 8);
		}
		LoadChar(ch);
	}
	str[pos] = '\0';
	return str;
}

SkipComment()
{
	/*	Skip Modula-2 like comment (* ... *).
		Note that comment may be nested.
	*/

	register int ch;
	register int NestLevel = 0;

	LoadChar(ch);
	for (;;) {
		if (class(ch) == STNL) {
			LineNumber++;
		}
		else
		if (ch == '(') {
			LoadChar(ch);
			if (ch == '*') {
				++NestLevel;
			}
			else {
				continue;
			}
		}
		else
		if (ch == '*') {
			LoadChar(ch);
			if (ch == ')') {
				if (NestLevel-- == 0) {
					return;
				}
			}
			else {
				continue;
			}
		}
		LoadChar(ch);
	}
}
