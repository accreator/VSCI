/*
 Very Simple C Interpreter (VSCI)
 
 Kai Sun 2015
*/
/*
 if else while break continue return
 float int void
 * / + - %
 > < >= <= == !=
 & ^ ~ | << >>
 && || !
 =
 ( ) [ ] , ; { }
 1 dim array only
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#define MAX_VAR_ID_LEN 32
#define MAX_CODE_LEN 256*1024

void error_log(char *fmt, ...)
{
	char buffer[1024];
	va_list va;
	va_start(va, fmt);
	vsprintf(buffer, fmt, va);
	printf("%s\n", buffer);
	fflush(stdout);
	va_end(va);
	exit(1);
}

enum {var_int, var_float, var_void, var_ints, var_floats, var_null};

enum {
	token_int = 128, token_float, token_void,
	token_if, token_else, token_while, token_break, token_continue, token_return,
	token_geq, token_leq, token_eq, token_neq,
	token_shl, token_shr,
	token_land, token_lor,
	token_id, token_ival, token_fval, token_null
};

struct VAR {
	int type;
	char *id;
	union {
		float fval;
		int ival;
		float *fvals;
		int *ivals;
	};
};

struct TOKEN {
	int type;
	union {
		char *id;

		float fval;
		int ival;
	};
};

struct OPT {
	void **nxt_tok;
	void **nxt_tok_p;
	void **prs_exp;
};

struct ENV {
	struct VAR *var;
	int size;
	int len;

	char *code;
	struct OPT *opt;
};

void env_enlarge(struct ENV *env) {
	if (env->len == env->size) {
		struct VAR *new_var = (struct VAR*)malloc(env->size * 2 * sizeof(struct VAR));
		int i;
		for (i = 0; i < env->len; i++) new_var[i] = env->var[i];
		env->size *= 2;
		free(env->var);
		env->var = new_var;
	}
}

void env_new_null(struct ENV *env) {
	env_enlarge(env);
	env->var[env->len].type = var_null;
	env->var[env->len].id = NULL;
	env->var[env->len].ival = 0;
	env->len++;
}

void env_new_level(struct ENV *env) {
	env_new_null(env);
}

void env_delete_level(struct ENV *env) {
	while (env->var[env->len - 1].type != var_null) {
		if (env->var[env->len - 1].id != NULL)
			free(env->var[env->len - 1].id);
		if (env->var[env->len - 1].type == var_ints) {
			int i, f = 1;
			for (i = env->len - 2; i >= 0; i--) {
				if (env->var[i].type == var_ints && env->var[i].ivals == env->var[env->len - 1].ivals) {
					f = 0;
					break;
				}
			}
			if(f) free(env->var[env->len - 1].ivals);
		}
		else if (env->var[env->len - 1].type == var_floats) {
			int i, f = 1;
			for (i = env->len - 2; i >= 0; i--) {
				if (env->var[i].type == var_floats && env->var[i].fvals == env->var[env->len - 1].fvals) {
					f = 0;
					break;
				}
			}
			if(f) free(env->var[env->len - 1].fvals);
		}
		env->len--;
	}
	env->len--;
}

void env_init(struct ENV *env, char *code) {
	env->len = 0;
	env->size = 128;
	env->var = (struct VAR*)malloc(env->size * sizeof(struct VAR));

	env->code = code;
	env->opt = (struct OPT*)malloc(strlen(code) * sizeof(struct OPT));
	memset(env->opt, 0, strlen(code) * sizeof(struct OPT));
}

void env_free(struct ENV *env) {
	int i;
	free(env->var);

	for (i = strlen(env->code) - 1; i >= 0; i--) {
		if (env->opt[i].nxt_tok != NULL) {
			if (((struct TOKEN *)(env->opt[i].nxt_tok[0]))->type == token_id) {
				if (((struct TOKEN *)(env->opt[i].nxt_tok[0]))->id != NULL)
					free(((struct TOKEN *)(env->opt[i].nxt_tok[0]))->id);
			}
			free(env->opt[i].nxt_tok[0]);
			free(env->opt[i].nxt_tok);
		}
		if (env->opt[i].nxt_tok_p != NULL) free(env->opt[i].nxt_tok_p);
		if (env->opt[i].prs_exp != NULL) free(env->opt[i].prs_exp);
	}
	free(env->opt);
}

int next_token(char *p, struct TOKEN *tok, struct ENV *env) {
	int ret = 0;
	
	int opt_p = p - env->code;
	if (env->opt[opt_p].nxt_tok != NULL) {
		tok->type = ((struct TOKEN *)(env->opt[opt_p].nxt_tok[0]))->type;
		if (tok->type == token_id)
			tok->id = ((struct TOKEN *)(env->opt[opt_p].nxt_tok[0]))->id == NULL ? NULL : strdup(((struct TOKEN *)(env->opt[opt_p].nxt_tok[0]))->id);
		if (tok->type == token_ival)
			tok->ival = ((struct TOKEN *)(env->opt[opt_p].nxt_tok[0]))->ival;
		if (tok->type == token_fval)
			tok->fval = ((struct TOKEN *)(env->opt[opt_p].nxt_tok[0]))->fval;
		ret = (int)(env->opt[opt_p].nxt_tok[1]);
		return ret;
	}

	tok->type = token_null;
	tok->id = NULL;
	while (1) {
		while (isspace(*p)) {
			p++;
			ret++;
		}
		if (*p == '/' && *(p + 1) == '/') {
			p += 2;
			ret += 2;
			while (*p != '\n') {
				p++;
				ret++;
			}
		}
		else if (*p == '/' && *(p + 1) == '*') {
			p += 2;
			ret += 2;
			while (*p != '*' || *(p + 1) != '/') {
				p++;
				ret++;
			}
			p += 2;
			ret += 2;
		}
		else break;
	}
	if (memcmp(p, "&&", 2) == 0) { tok->type = token_land; ret += 2; }
	else if (memcmp(p, "||", 2) == 0) { tok->type = token_lor; ret += 2; }
	else if (memcmp(p, ">=", 2) == 0) { tok->type = token_geq; ret += 2; }
	else if (memcmp(p, "<=", 2) == 0) { tok->type = token_leq; ret += 2; }
	else if (memcmp(p, "==", 2) == 0) { tok->type = token_eq; ret += 2; }
	else if (memcmp(p, "!=", 2) == 0) { tok->type = token_neq; ret += 2; }
	else if (memcmp(p, "<<", 2) == 0) { tok->type = token_shl; ret += 2; }
	else if (memcmp(p, ">>", 2) == 0) { tok->type = token_shr; ret += 2; }
	else if (*p == '*' || *p == '/' || *p == '+' || *p == '-' || *p == '%'
		|| *p == '>' || *p == '<'
		|| *p == '&' || *p == '^' || *p == '~' || *p == '|'
		|| *p == '!'
		|| *p == '='
		|| *p == '(' || *p == ')' || *p == '[' || *p == ']' || *p == ',' || *p == ';' || *p == '{' || *p == '}') {
		tok->type = *p;
		ret += 1;
	}
	else if (memcmp(p, "int", 3) == 0 && !isalnum(*(p + 3)) && *(p + 3) != '_') { tok->type = token_int; ret += 3; }
	else if (memcmp(p, "float", 5) == 0 && !isalnum(*(p + 5)) && *(p + 5) != '_') { tok->type = token_float; ret += 5; }
	else if (memcmp(p, "void", 4) == 0 && !isalnum(*(p + 4)) && *(p + 4) != '_') { tok->type = token_void; ret += 4; }
	else if (memcmp(p, "if", 2) == 0 && !isalnum(*(p + 2))  && *(p + 2) != '_') { tok->type = token_if; ret += 2; }
	else if (memcmp(p, "else", 4) == 0 && !isalnum(*(p + 4)) && *(p + 4) != '_') { tok->type = token_else; ret += 4; }
	else if (memcmp(p, "while", 5) == 0 && !isalnum(*(p + 5)) && *(p + 5) != '_') { tok->type = token_while; ret += 5; }
	else if (memcmp(p, "break", 5) == 0 && !isalnum(*(p + 5)) && *(p + 5) != '_') { tok->type = token_break; ret += 5; }
	else if (memcmp(p, "continue", 8) == 0 && !isalnum(*(p + 8)) && *(p + 8) != '_') { tok->type = token_continue; ret += 8; }
	else if (memcmp(p, "return", 6) == 0 && !isalnum(*(p + 6)) && *(p + 6) != '_') { tok->type = token_return; ret += 6; }
	else if (isalpha(*p) || *p == '_') {
		int i;
		tok->type = token_id;
		tok->id = (char*)malloc(MAX_VAR_ID_LEN * sizeof(char));
		for (i = 0; isalnum(p[i]) || p[i] == '_'; i++) tok->id[i] = p[i];
		tok->id[i] = '\0';
		ret += i;
	}
	else if (isdigit(*p)) {
		int i;
		for (i = 0; isdigit(p[i]); i++);
		if (p[i] == '.') {
			float b;
			tok->type = token_fval;
			tok->fval = 0.0;
			for (i = 0; isdigit(p[i]); i++) {
				tok->fval *= 10.0;
				tok->fval += p[i] - '0';
			}
			b = 1;
			for (i++; isdigit(p[i]); i++) {
				b /= 10;
				tok->fval += (p[i] - '0') * b;
			}
			ret += i;
		}
		else {
			tok->type = token_ival;
			tok->ival = 0;
			for (i = 0; isdigit(p[i]); i++) {
				tok->ival *= 10;
				tok->ival += p[i] - '0';
			}
			ret += i;
		}
	}
	else error_log("Unexpected symbol %c", *p);

	env->opt[opt_p].nxt_tok = (void **)malloc(2*sizeof(void*));
	env->opt[opt_p].nxt_tok[0] = malloc(sizeof(struct TOKEN));
	((struct TOKEN *)(env->opt[opt_p].nxt_tok[0]))->type = tok->type;
	if (tok->type == token_id)
		((struct TOKEN *)(env->opt[opt_p].nxt_tok[0]))->id = (tok->id == NULL) ? NULL : strdup(tok->id);
	else if (tok->type == token_ival)
		((struct TOKEN *)(env->opt[opt_p].nxt_tok[0]))->ival = tok->ival;
	else if (tok->type == token_fval)
		((struct TOKEN *)(env->opt[opt_p].nxt_tok[0]))->fval = tok->fval;
	env->opt[opt_p].nxt_tok[1] = (void *)ret;
	return ret;
}

int next_token_pair(char *p, int type, struct ENV *env) {
	struct TOKEN tok;
	int i, j, k;
	int l, r;

	int opt_p = p - env->code;
	if (env->opt[opt_p].nxt_tok_p != NULL) {
		i = (type == '}') ? 0 : ((type == ']') ? 1 : 2);
		if ((int)(env->opt[opt_p].nxt_tok_p[i]) != -1)
			return (int)(env->opt[opt_p].nxt_tok_p[i]);
	}

	if (type == '}') { l = '{'; r = '}'; }
	else if (type == ']') { l = '['; r = ']'; }
	else /* ')' */ { l = '('; r = ')'; }
	i = 0;
	j = 0;
	while (1) {
		k = next_token(p + j, &tok, env);
		if (tok.type == l) i++;
		else if (tok.type == r) {
			i--;
			if (i < 0) break;
		}
		else if (tok.type == token_id) {
			free(tok.id);
		}
		j += k;
	}

	if (env->opt[opt_p].nxt_tok_p == NULL) {
		env->opt[opt_p].nxt_tok_p = (void **)malloc(3 * sizeof(void*));
		memset(env->opt[opt_p].nxt_tok_p, -1, (3 * sizeof(void*)));
	}
	env->opt[opt_p].nxt_tok_p[(type == '}') ? 0 : ((type == ']') ? 1 : 2)] = (void *)j;
	return j;
}

int next_token_type(char *p, int type, struct ENV *env) {
	struct TOKEN tok;
	int i, j;
	i = 0;
	while (1) {
		j = next_token(p + i, &tok, env);
		if (tok.type == token_id) free(tok.id);
		if (tok.type == type) break;
		i += j;
	}
	return i;
}

int next_token_operator(char *p, int len, int type[], int tsize, int assoc, int opnum, struct ENV *env) {
	//assoc 1: left-to-right  0: right-to-left
	//opnum 0: any  otherwise, the number of operand
	struct TOKEN tok;
	char *q = p + len;
	int i, j, k, l;
	int ret = -1;
	i = 0;
	k = 0;
	while (p+i < q) {
		j = next_token(p + i, &tok, env);
		if (tok.type == '{') { i += j; i += next_token_pair(p + i, '}', env); continue; }
		if (tok.type == '[') { i += j; i += next_token_pair(p + i, ']', env); continue; }
		if (tok.type == '(') { i += j; i += next_token_pair(p + i, ')', env); k = 1; continue; }
		for (l = 0; l < tsize; l++) {
			if (tok.type == type[l]) {
				if (opnum == 0 || k + 1 == opnum) {
					if (assoc == 0) return i;
					ret = i;
					break;
				}
			}
		}
		if (tok.type == token_id) {
			free(tok.id);
			k = 1;
		}
		else if (tok.type == '*' || tok.type == '/' || tok.type == '+' || tok.type == '-' || tok.type == '%'
			|| tok.type == '>' || tok.type == '/' || tok.type == token_geq || tok.type == token_leq || tok.type == token_eq
			|| tok.type == token_neq || tok.type == '&' || tok.type == '|' || tok.type == '^' || tok.type == '~'
			|| tok.type == token_land || tok.type == token_lor || tok.type == '!' || tok.type == '=' || tok.type == ',') {
			k = 0;
		}
		else if (tok.type == token_ival || tok.type == token_fval) {
			k = 1;
		}
		i += j;
	}
	return ret;
}

struct VAR parse_expression(char *p, int len, struct ENV *env) {
	struct TOKEN tok;
	char *q = p + len;
	struct VAR ret;
	int i, j, l;
	int tl[15][7] = {
		//num, assoc, opnum,  tok_1, tok_2, ...
		{ 1, 1, 2, ',' }, //0
		{ 1, 0, 2, '=' }, //1
		{ 0 }, //2
		{ 1, 1, 2, token_lor }, //3
		{ 1, 1, 2, token_land }, //4
		{ 1, 1, 2, '|' }, //5
		{ 1, 1, 2, '^' }, //6
		{ 1, 1, 2, '&' }, //7
		{ 2, 1, 2, token_neq, token_eq }, //8
		{ 4, 1, 2, '>', token_geq, '<', token_leq }, //9
		{ 2, 1, 2, token_shl, token_shr}, //10
		{ 2, 1, 2, '+', '-' }, //11
		{ 3, 1, 2, '/', '*', '%' }, //12
		{ 3, 0, 1, '-', '!', '~' }, //13
		{ 0 } //14
	};
	
	int opt_p = p - env->code, opt_s = -1, opt_r = 1;
	if (env->opt[opt_p].prs_exp == NULL) {
		env->opt[opt_p].prs_exp = (void **)malloc((2 + 4 * 8) * sizeof(void*));
		env->opt[opt_p].prs_exp[0] = (void*)8;
		env->opt[opt_p].prs_exp[1] = (void*)0;
	}
	else if (env->opt[opt_p].prs_exp[0] == env->opt[opt_p].prs_exp[1]) {
		void **_prs_exp = (void **)malloc((2 + 2 * 4 * (int)env->opt[opt_p].prs_exp[0]) * sizeof(void*));
		memcpy(_prs_exp, env->opt[opt_p].prs_exp, (2 + 4 * (int)env->opt[opt_p].prs_exp[0]) * sizeof(void*));
		_prs_exp[0] = (void*)(2 * (int)_prs_exp[0]);
		free(env->opt[opt_p].prs_exp);
		env->opt[opt_p].prs_exp = _prs_exp;
	}
	for (i = 0; i < (int)env->opt[opt_p].prs_exp[1]; i++) {
		if ((int)env->opt[opt_p].prs_exp[2 + 4 * i] == len) {
			l = (int)env->opt[opt_p].prs_exp[3 + 4 * i];
			opt_s = 3 + 4 * i;
			break;
		}
	}
	if (opt_s == -1) {
		l = 0;
		opt_r = 0;
		env->opt[opt_p].prs_exp[2 + 4 * (int)env->opt[opt_p].prs_exp[1]] = (void*)len;
		opt_s = 3 + 4 * (int)env->opt[opt_p].prs_exp[1];
		env->opt[opt_p].prs_exp[1] = (void*)((int)env->opt[opt_p].prs_exp[1] + 1);
	}

	ret.id = NULL;
	for (; l < 15; l++) {
		if (tl[l][0] == 0) continue;
		if (opt_r) {
			i = (int)env->opt[opt_p].prs_exp[opt_s + 1];
		}
		else {
			env->opt[opt_p].prs_exp[opt_s] = (void*)l;
			i = next_token_operator(p, len, &tl[l][3], tl[l][0], tl[l][1], tl[l][2], env);
			env->opt[opt_p].prs_exp[opt_s + 1] = (void*)i;
		}
		if (i == -1) continue;
		if (l == 0) { //','
			parse_expression(p, i, env);
			p += i;
			p += next_token(p, &tok, env); //','
			return parse_expression(p, q - p, env);
		}
		if (l == 1) { //'='
			struct VAR v, u;
			i += next_token(p + i, &tok, env); //'='
			v = parse_expression(p + i, q - (p + i), env);
			p += next_token(p, &tok, env); //id
			for (j = opt_r ? (int)env->opt[opt_p].prs_exp[opt_s + 2] : env->len - 1; j >= 0; j--) {
				if (opt_r || (env->var[j].id != NULL && strcmp(env->var[j].id, tok.id) == 0)) {
					free(tok.id);
					if (!opt_r) env->opt[opt_p].prs_exp[opt_s + 2] = (void*)j;
					if ((env->var[j].type == var_int || env->var[j].type == var_ints) && v.type == var_float) {
						v.type = var_int;
						v.ival = (int)v.fval;
					}
					else if ((env->var[j].type == var_float || env->var[j].type == var_floats) && v.type == var_int) {
						v.type = var_float;
						v.fval = (float)v.ival;
					}
					if (env->var[j].type == var_int) {
						env->var[j].ival = v.ival;
						ret.type = v.type;
						ret.ival = v.ival;
						return ret;
					}
					if (env->var[j].type == var_float) {
						env->var[j].fval = v.fval;
						ret.type = v.type;
						ret.fval = v.fval;
						return ret;
					}
					p += next_token(p, &tok, env); //[
					i = next_token_pair(p, ']', env);
					u = parse_expression(p, i, env);
					if (u.type == var_float) {
						u.type = var_int;
						u.ival = (int)u.fval;
					}
					if (env->var[j].type == var_ints) {
						env->var[j].ivals[u.ival] = v.ival;
						ret.type = v.type;
						ret.ival = v.ival;
						return ret;
					}
					if (env->var[j].type == var_floats) {
						env->var[j].fvals[u.ival] = v.fval;
						ret.type = v.type;
						ret.fval = v.fval;
						return ret;
					}
				}
			}
			error_log("Undefined variable %s", tok.id);
		}
		if (l == 3 || l == 4) { //token_lor, token_land
			struct VAR v;
			v = parse_expression(p, i, env);
			if (v.type == var_float) {
				v.type = var_int;
				v.ival = (int)v.fval;
			}
			if (l==3 && v.ival || l==4 && !v.ival) {
				ret.type = var_int;
				ret.ival = (1 == 3);
				return ret;
			}
			p += i;
			p += next_token(p, &tok, env); //token_lor, token_land
			v = parse_expression(p, q - p, env);
			if (v.type == var_float) {
				v.type = var_int;
				v.ival = (int)v.fval;
			}
			ret.type = var_int;
			ret.ival = (l == 3) ? (0 || v.ival) : (1 && v.ival);
			return ret;
		}
		if (l == 5 || l == 6 || l == 7 || l == 10) { //'|', '^', '&', token_shl, token_shr
			struct VAR v, u;
			v = parse_expression(p, i, env);
			if (v.type == var_float) {
				v.type = var_int;
				v.ival = (int)v.fval;
			}
			p += i;
			p += next_token(p, &tok, env); //'|', '^', '&'
			u = parse_expression(p, q - p, env);
			if (u.type == var_float) {
				u.type = var_int;
				u.ival = (int)u.fval;
			}
			ret.type = var_int;
			ret.ival = (tok.type == '|') ? (v.ival | u.ival) : (
					(tok.type == '^') ? (v.ival ^ u.ival) : (
					(tok.type == '&') ? (v.ival & u.ival) : (
					(tok.type == token_shl) ? (v.ival << u.ival) :
					/* token_shr */ (v.ival >> u.ival)
				)));
			return ret;
		}
		if (l == 8 || l == 9) { //token_neq, token_eq, '>', token_geq, '<', token_leq
			struct VAR v, u;
			v = parse_expression(p, i, env);
			p += i;
			p += next_token(p, &tok, env); //token_neq, token_eq, '>', token_geq, '<', token_leq
			u = parse_expression(p, q - p, env);

			if (v.type != u.type) {
				if (v.type == var_int) {
					v.type = var_float;
					v.fval = (float)v.ival;
				}
				else {
					u.type = var_float;
					u.fval = (float)u.ival;
				}
			}
			ret.type = var_int;
			if (tok.type == token_neq) ret.ival = (v.type == var_int) ? (v.ival != u.ival) : (v.fval != u.fval);
			else if (tok.type == token_eq) ret.ival = (v.type == var_int) ? (v.ival == u.ival) : (v.fval == u.fval);
			else if (tok.type == '>') ret.ival = (v.type == var_int) ? (v.ival > u.ival) : (v.fval > u.fval);
			else if (tok.type == token_geq) ret.ival = (v.type == var_int) ? (v.ival >= u.ival) : (v.fval >= u.fval);
			else if (tok.type == '<') ret.ival = (v.type == var_int) ? (v.ival < u.ival) : (v.fval < u.fval);
			else /*token_leq*/ ret.ival = (v.type == var_int) ? (v.ival <= u.ival) : (v.fval <= u.fval);
			return ret;
		}
		if (l == 11 || l == 12) { //'+', '-', '*', '/', '%'
			struct VAR v, u;
			v = parse_expression(p, i, env);
			p += i;
			p += next_token(p, &tok, env); //'+', '-', '*', '/', '%'
			u = parse_expression(p, q - p, env);

			if (v.type != u.type) {
				if (v.type == var_int) {
					v.type = var_float;
					v.fval = (float)v.ival;
				}
				else {
					u.type = var_float;
					u.fval = (float)u.ival;
				}
			}
			ret.type = v.type;
			if (tok.type == '+') {
				if (v.type == var_int) ret.ival = v.ival + u.ival; else ret.fval = v.fval + u.fval;
			}
			else if (tok.type == '-') {
				if (v.type == var_int) ret.ival = v.ival - u.ival; else ret.fval = v.fval - u.fval;
			}
			else if (tok.type == '*') {
				if (v.type == var_int) ret.ival = v.ival * u.ival; else ret.fval = v.fval * u.fval;
			}
			else if (tok.type == '/') {
				if (v.type == var_int) ret.ival = v.ival / u.ival; else ret.fval = v.fval / u.fval;
			}
			else { //'%'
				if (v.type == var_int) ret.ival = v.ival % u.ival;
				else error_log("The operands of %% must be integer");
			}
			return ret;
		}
		if (l == 13) { //'-', '!', '~'
			struct VAR v;
			p += i;
			p += next_token(p, &tok, env); //'-', '!', '~'
			v = parse_expression(p, q - p, env);

			if (tok.type == '-') {
				ret.type = v.type;
				if (v.type == var_int) ret.ival = -v.ival; else ret.fval = -v.fval;
			}
			else if (tok.type == '!') {
				ret.type = var_int;
				if (v.type == var_int) ret.ival = !v.ival;
				else error_log("The operands of ! must be integer");
			}
			else { //'~'
				ret.type = var_int;
				if (v.type == var_int) ret.ival = ~v.ival;
				else error_log("The operands of ~ must be integer");
			}
			return ret;
		}
	}
	//the rest is either (exp) or id or id[] or val
	//currently do not support id(...)
	p += next_token(p, &tok, env); //','
	if (tok.type == '(') {
		i = next_token_pair(p, ')', env);
		return parse_expression(p, i, env);
	}
	if (tok.type == token_ival) {
		ret.type = var_int;
		ret.ival = tok.ival;
		return ret;
	}
	if (tok.type == token_fval) {
		ret.type = var_float;
		ret.fval = tok.fval;
		return ret;
	}
	//tok.type == token_id
	if (tok.type != token_id) error_log("Unexpected syntax error");
	for (j = opt_r ? (int)env->opt[opt_p].prs_exp[opt_s + 2] : env->len - 1; j >= 0; j--) {
		if (opt_r || (env->var[j].id != NULL && strcmp(env->var[j].id, tok.id) == 0)) {
			struct VAR v;
			free(tok.id);
			if (!opt_r) env->opt[opt_p].prs_exp[opt_s + 2] = (void*)j;
			if (env->var[j].type == var_int) {
				ret.type = var_int;
				ret.ival = env->var[j].ival;
				return ret;
			}
			if (env->var[j].type == var_float) {
				ret.type = var_float;
				ret.fval = env->var[j].fval;
				return ret;
			}
			p += next_token(p, &tok, env); //'['
			i = next_token_pair(p, ']', env);
			v = parse_expression(p, i, env);
			if (v.type == var_float) {
				v.type = var_int;
				v.ival = (int)v.fval;
			}
			if (env->var[j].type == var_ints) {
				ret.type = var_int;
				ret.ival = env->var[j].ivals[v.ival];
				return ret;
			}
			if (env->var[j].type == var_floats) {
				ret.type = var_float;
				ret.fval = env->var[j].fvals[v.ival];
				return ret;
			}
		}
	}
	error_log("Undefined variable %s", tok.id);
	return ret;
}

int parse_statements(char *p, int len, int retp, struct ENV *env) { //1 return  2 break  3 continue  0 otherwise
	struct TOKEN tok;
	char *q = p + len;
	int i, ret = 0;
	
	while (p < q) {
		i = next_token(p, &tok, env);
		if (tok.type == ';') {
			p += i;
			continue;
		}
		if (tok.type == '{') {
			p += i;
			i = next_token_pair(p, '}', env);
			env_new_level(env);
			ret = parse_statements(p, i, retp, env);
			env_delete_level(env);
			if (ret) return ret;
			p += i;
			p += next_token(p, &tok, env); //'}'
			continue;
		}
		if (tok.type == token_if) {
			struct VAR cond;
			p += i;
			p += next_token(p, &tok, env); //'('
			i = next_token_pair(p, ')', env);
			cond = parse_expression(p, i, env);
			if (cond.type == var_float) {
				cond.type = var_int;
				cond.ival = (int)cond.fval;
			}
			p += i;
			p += next_token(p, &tok, env); //')'
			next_token(p, &tok, env);
			if (tok.type == '{') {
				p += next_token(p, &tok, env); //'{'
				i = next_token_pair(p, '}', env);
			}
			else {
				if (tok.type == token_id) free(tok.id);
				i = next_token_type(p, ';', env);
			}
			if (cond.ival) {
				env_new_level(env);
				ret = parse_statements(p, i, retp, env);
				env_delete_level(env);
				if (ret) return ret;
			}
			p += i;
			p += next_token(p, &tok, env); //'}' or ';'
			next_token(p, &tok, env);
			if (tok.type == token_else) {
				p += next_token(p, &tok, env); //else
				next_token(p, &tok, env);
				if (tok.type == '{') {
					p += next_token(p, &tok, env); //'{'
					i = next_token_pair(p, '}', env);
				}
				else {
					if (tok.type == token_id) free(tok.id);
					i = next_token_type(p, ';', env);
				}
				if (!cond.ival) {
					env_new_level(env);
					ret = parse_statements(p, i, retp, env);
					env_delete_level(env);
					if (ret) return ret;
				}
				p += i;
				p += next_token(p, &tok, env); //'}' or ';'
			}
			else {
				if (tok.type == token_id) free(tok.id);
			}
			continue;
		}
		if (tok.type == token_while) {
			struct VAR cond;
			char * _p;
			int _i;
			p += i;
			p += next_token(p, &tok, env); //'('
			i = next_token_pair(p, ')', env);
			_p = p + i;
			_p += next_token(_p, &tok, env); //')'
			next_token(_p, &tok, env);
			if (tok.type == '{') {
				_p += next_token(_p, &tok, env); //'{'
				_i = next_token_pair(_p, '}', env);
			}
			else {
				if (tok.type == token_id) free(tok.id);
				_i = next_token_type(_p, ';', env);
			}
			while (1) {
				cond = parse_expression(p, i, env);
				if (cond.type == var_float) {
					cond.type = var_int;
					cond.ival = (int)cond.fval;
				}
				if (cond.ival) {
					env_new_level(env);
					ret = parse_statements(_p, _i, retp, env);
					env_delete_level(env);
					if (ret == 1) return ret;
					if (ret == 2) break;
					if (ret == 3) continue;
				}
				else break;
			}
			p = _p + _i;
			p += next_token(p, &tok, env); //'}' or ';'
			continue;
		}
		if (tok.type == token_return) {
			struct VAR v;
			p += i;
			next_token(p, &tok, env);
			if (tok.type != ';') {//!="return;"
				if (tok.type == token_id) free(tok.id);
				i = next_token_type(p, ';', env);
				v = parse_expression(p, i, env);
				if (env->var[retp].type == var_int)
					env->var[retp].ival = (v.type == var_int ? v.ival : (int)v.fval);
				else if (env->var[retp].type == var_float)
					env->var[retp].fval = (v.type == var_float ? v.fval : (float)v.ival);
				else error_log("Type of return value must match the definition");
			}
			return 1;
		}
		if (tok.type == token_continue) return 3;
		if (tok.type == token_break) return 2;
		if (tok.type == token_int || tok.type == token_float) {
			int t = (tok.type == token_int ? var_int : var_float);
			p += i;
			while (1) {
				p += next_token(p, &tok, env);
				env_new_null(env);
				env->var[env->len - 1].id = tok.id;
				env->var[env->len - 1].type = t;
				p += next_token(p, &tok, env);
				if (tok.type == ',') continue;
				if (tok.type == ';') break;
				if (tok.type == '[') {
					struct VAR v;
					i = next_token_pair(p, ']', env);
					v = parse_expression(p, i, env);
					if (v.type == var_float) {
						v.type = var_int;
						v.ival = (int)v.fval;
					}
					if (t == var_int) {
						env->var[env->len - 1].type = var_ints;
						env->var[env->len - 1].ivals = (int *)malloc(v.ival * sizeof(int));
						memset(env->var[env->len - 1].ivals, 0, v.ival * sizeof(int));
					}
					else {
						env->var[env->len - 1].type = var_floats;
						env->var[env->len - 1].fvals = (float *)malloc(v.ival * sizeof(float));
						memset(env->var[env->len - 1].fvals, 0, v.ival * sizeof(float));
					}
					p += i;
					p += next_token(p, &tok, env); //']'
					p += next_token(p, &tok, env);
				}
				if (tok.type == ',') continue;
				if (tok.type == ';') break;
				if (tok.type == '=') {
					if (env->var[env->len - 1].type == var_ints || env->var[env->len - 1].type == var_floats) {
						int t[] = { ',', '}'};
						struct VAR v;
						int j = 0;
						p += next_token(p, &tok, env); //'{'
						if (tok.type != '{') error_log("Syntax of array initialization is incorrect");
						while (1) {
							i = next_token_operator(p, q - p, t, 2, 0, 0, env);
							v = parse_expression(p, i, env);
							if (env->var[env->len - 1].type == var_ints) {
								if (v.type == var_float) {
									v.type = var_int;
									v.ival = (int)v.fval;
								}
								env->var[env->len - 1].ivals[j] = v.ival;
							}
							else if (env->var[env->len - 1].type == var_floats) {
								if (v.type == var_int) {
									v.type = var_float;
									v.fval = (float)v.ival;
								}
								env->var[env->len - 1].fvals[j] = v.fval;
							}
							p += i;
							p += next_token(p, &tok, env); //',' or '}'
							if (tok.type == '}') break;
							if (tok.type != ',') error_log("Syntax of array initialization is incorrect");
							j++;
						}
					}
					else { //var_int or var_float
						int t[] = { ',', ';' };
						struct VAR v;
						i = next_token_operator(p, q - p, t, 2, 0, 0, env);
						v = parse_expression(p, i, env);
						if (env->var[env->len - 1].type == var_int) {
							if (v.type == var_float) {
								v.type = var_int;
								v.ival = (int)v.fval;
							}
							env->var[env->len - 1].ival = v.ival;
						}
						else if (env->var[env->len - 1].type == var_float) {
							if (v.type == var_int) {
								v.type = var_float;
								v.fval = (float)v.ival;
							}
							env->var[env->len - 1].fval = v.fval;
						}
						p += i;
					}
					p += next_token(p, &tok, env);
				}
				if (tok.type == ',') continue;
				if (tok.type == ';') break;
			}
			continue;
		}
		//the rest is expression
		if (tok.type == token_id) free(tok.id);
		i = next_token_type(p, ';', env);
		parse_expression(p, i, env);
		p += i;
		p += next_token(p, &tok, env); //';'
	}
	
	return ret;
}

void parse_function(char *p, struct ENV *env) {
	struct TOKEN type;
	struct TOKEN id;
	struct TOKEN tok;
	int i;
	int retp;
	
	p += next_token(p, &type, env);
	if (type.type == token_id) { //default function type is int
		id = type;
		type.type = token_int;
		type.id = NULL;
	}
	else {
		p += next_token(p, &id, env);
	}
	p += next_token(p, &tok, env); //'('
	p += next_token(p, &tok, env);
	i = 0;
	while (tok.type != ')') {
		i++;
		if (env->var[env->len - i].type == var_float || env->var[env->len - i].type == var_int) {
			if (tok.type == token_id) {
				if (env->var[env->len - i].type == var_float)
					env->var[env->len - i].ival = (int)env->var[env->len - i].fval;
				env->var[env->len - i].type = var_int;
			}
			else {
				if (env->var[env->len - i].type == var_float && tok.type == token_int)
					env->var[env->len - i].ival = (int)env->var[env->len - i].fval;
				else if (env->var[env->len - i].type == var_int && tok.type == token_float)
					env->var[env->len - i].fval = (float)env->var[env->len - i].ival;
				if (tok.type == token_int)
					env->var[env->len - i].type = var_int;
				else //token_float
					env->var[env->len - i].type = var_float;
				p += next_token(p, &tok, env); //get the id
			}
			env->var[env->len - i].id = tok.id;
		}
		else { //var_floats  or  var_ints
			if (tok.type == token_id) {
				env->var[env->len - i].type = var_ints;
			}
			else {
				if (tok.type == token_int)
					env->var[env->len - i].type = var_ints;
				else //token_float
					env->var[env->len - i].type = var_floats;
				p += next_token(p, &tok, env); //get the id
			}
			env->var[env->len - i].id = tok.id;
			do p += next_token(p, &tok, env); while (tok.type != ']');
		}
		do p += next_token(p, &tok, env); while (tok.type == ',');
	}
	retp = env->len - i - 1;
	if (type.type == token_int)
		env->var[retp].type = var_int;
	else if (type.type == token_void)
		env->var[retp].type = var_void;
	else //token_float
		env->var[retp].type = var_float;
	p += next_token(p, &tok, env); //'{'
	parse_statements(p, next_token_pair(p, '}', env), retp, env);
	free(id.id);
}

void parse_main(char *p) {
	struct ENV env;
	int retp;
	env_init(&env, p);
	env_new_level(&env);
	env_new_null(&env); //store return value
	retp = env.len - 1;
	//add params to env ...
	parse_function(p, &env);
	//process the return value
	if (env.var[retp].type == var_int)
		printf("val: %d\n", env.var[retp].ival);
	else if (env.var[retp].type == var_float)
		printf("val: %f\n", env.var[retp].fval);
	else
		printf("val: null\n");
	env_delete_level(&env);
	env_free(&env);
}

int main(int argc, char *argv[]) {
	if (argc >= 2) {
		int len = 0;
		char *code;
		FILE *in;
		code = (char*)malloc(MAX_CODE_LEN * sizeof(char));
		in = fopen(argv[1], "r");
		while (fscanf(in, "%c", &code[len]) != EOF) len++;
		code[len] = '\0';
		fclose(in);
		parse_main(code);
		free(code);
	}
	return 0;
}