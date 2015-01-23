/*
 if else while break continue return
 float int void
 * / + - %
 > < >= <= == !=
 & ^ ~ |
 && || !
 =
 ( ) [ ] , ; { }
 1 dim array only
*/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#define MAX_VAR_ID_LEN 32
#define MAX_CODE_LEN 256*1024

enum {var_int, var_float, var_void, var_ints, var_floats, var_null};

enum {
	token_int = 128, token_float, token_void,
	token_if, token_else, token_while, token_break, token_continue, token_return,
	token_geq, token_leq, token_eq, token_neq,
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

struct ENV {
	struct VAR *var;
	int size;
	int len;
};

void env_enlarge(struct ENV *env) {
	if (env->len == env->size) {
		struct VAR *new_var = (struct VAR*)malloc(env->size * 2 * sizeof(struct VAR));
		int i;
		for (i = 0; i < env->len; i++) {
			new_var[i] = env->var[i];
		}
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
		env->len--;
	}
	env->len--;
}

void env_init(struct ENV *env) {
	env->len = 0;
	env->size = 128;
	env->var = (struct VAR*)malloc(env->size * sizeof(struct VAR));
}

void env_free(struct ENV *env) {
	free(env->var);
}

int next_token(char *p, struct TOKEN *tok) {
	int ret = 0;
	tok->type = token_null;
	tok->id = NULL;
	while (isspace(*p)) {
		p++;
		ret++;
	}
	if (*p == '&' && *(p + 1) == '&') {
		tok->type = token_land;
		ret += 2;
	}
	else if (*p == '|' && *(p + 1) == '|') {
		tok->type = token_lor;
		ret += 2;
	}
	else if (*p == '>' && *(p + 1) == '=') {
		tok->type = token_geq;
		ret += 2;
	}
	else if (*p == '<' && *(p + 1) == '=') {
		tok->type = token_leq;
		ret += 2;
	}
	else if (*p == '=' && *(p + 1) == '=') {
		tok->type = token_eq;
		ret += 2;
	}
	else if (*p == '!' && *(p + 1) == '=') {
		tok->type = token_neq;
		ret += 2;
	}
	else if (*p == '*' || *p == '/' || *p == '+' || *p == '-' || *p == '%'
		|| *p == '>' || *p == '<'
		|| *p == '&' || *p == '^' || *p == '~' || *p == '|'
		|| *p == '!'
		|| *p == '='
		|| *p == '(' || *p == ')' || *p == '[' || *p == ']' || *p == ',' || *p == ';' || *p == '{' || *p == '}') {
		tok->type = *p;
		ret += 1;
	}
	else if (*p == 'i' && *(p + 1) == 'n' && *(p + 2) == 't' && !isalnum(*(p + 3)) && *(p + 3) != '_') {
		tok->type = token_int;
		ret += 3;
	}
	else if (*p == 'f' && *(p + 1) == 'l' && *(p + 2) == 'o' && *(p + 3) == 'a' && *(p + 4) == 't' && !isalnum(*(p + 5)) && *(p + 5) != '_') {
		tok->type = token_float;
		ret += 5;
	}
	else if (*p == 'v' && *(p + 1) == 'o' && *(p + 2) == 'i' && *(p + 3) == 'd' && !isalnum(*(p + 4)) && *(p + 4) != '_') {
		tok->type = token_void;
		ret += 4;
	}
	else if (*p == 'i' && *(p + 1) == 'f' && !isalnum(*(p + 2)) && *(p + 2) != '_') {
		tok->type = token_if;
		ret += 2;
	}
	else if (*p == 'e' && *(p + 1) == 'l' && *(p + 2) == 's' && *(p + 3) == 'e' && !isalnum(*(p + 4)) && *(p + 4) != '_') {
		tok->type = token_else;
		ret += 4;
	}
	else if (*p == 'w' && *(p + 1) == 'h' && *(p + 2) == 'i' && *(p + 3) == 'l' && *(p + 4) == 'e' && !isalnum(*(p + 5)) && *(p + 5) != '_') {
		tok->type = token_while;
		ret += 5;
	}
	else if (*p == 'b' && *(p + 1) == 'r' && *(p + 2) == 'e' && *(p + 3) == 'a' && *(p + 4) == 'k' && !isalnum(*(p + 5)) && *(p + 5) != '_') {
		tok->type = token_break;
		ret += 5;
	}
	else if (*p == 'c' && *(p + 1) == 'o' && *(p + 2) == 'n' && *(p + 3) == 't' && *(p + 4) == 'i'
		&& *(p + 5) == 'n' && *(p + 6) == 'u' && *(p + 7) == 'e' && !isalnum(*(p + 8)) && *(p + 8) != '_') {
		tok->type = token_continue;
		ret += 8;
	}
	else if (*p == 'r' && *(p + 1) == 'e' && *(p + 2) == 't' && *(p + 3) == 'u' && *(p + 4) == 'r'
		&& *(p + 5) == 'n' && !isalnum(*(p + 6)) && *(p + 6) != '_') {
		tok->type = token_return;
		ret += 6;
	}
	else if (isalpha(*p) || *p == '_') {
		int i;
		tok->type = token_id;
		tok->id = (char*)malloc(MAX_VAR_ID_LEN * sizeof(char));
		for (i = 0; isalnum(p[i]) || p[i] == '_'; i++) {
			tok->id[i] = p[i];
		}
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
	return ret;
}

int next_token_pair(char *p, int type) {
	struct TOKEN tok;
	int i, j, k;
	int l, r;
	if (type == '}') {
		l = '{';
		r = '}';
	}
	else if (type == ']') {
		l = '[';
		r = ']';
	}
	else { //')'
		l = '(';
		r = ')';
	}
	i = 0;
	j = 0;
	while (1) {
		k = next_token(p + j, &tok);
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
	return j;
}

int next_token_type(char *p, int type) {
	struct TOKEN tok;
	int i, j;
	i = 0;
	while (1) {
		j = next_token(p + i, &tok);
		if (tok.type == token_id) {
			free(tok.id);
		}
		if (tok.type == type) break;
		i += j;
	}
	return i;
}

int next_token_operator(char *p, int len, int type[], int tsize, int assoc, int opnum) { //assoc 1: left-to-right  0: right-to-left
	struct TOKEN tok;
	char *q = p + len;
	int i, j, k, l;
	int ret = -1;
	i = 0;
	k = 0;
	while (p+i < q) {
		j = next_token(p + i, &tok);
		if (tok.type == '{') {
			i += j;
			i += next_token_pair(p + i, '}');
			continue;
		}
		if (tok.type == '[') {
			i += j;
			i += next_token_pair(p + i, ']');
			continue;
		}
		if (tok.type == '(') {
			i += j;
			i += next_token_pair(p + i, ')');
			k = 1;
			continue;
		}
		for (l = 0; l < tsize; l++) {
			if (tok.type == type[l]) {
				if (k + 1 == opnum) {
					if (assoc == 0)
						return i;
					else {
						ret = i;
						break;
					}
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
	int i, j;
	int t1[] = { ',' };
	int t2[] = { '=' };
	int t3[] = { token_lor };
	int t4[] = { token_land };
	int t5[] = { '|' };
	int t6[] = { '^' };
	int t7[] = { '&' };
	int t8[] = { token_neq };
	int t9[] = { token_eq };
	int t10[] = { '>', token_geq, '<', token_leq };
	int t11[] = { '+', '-' };
	int t12[] = { '/', '*', '%' };
	int t13[] = { '-', '!', '~' };

	ret.id = NULL;
	if ((i = next_token_operator(p, len, t1, 1, 1, 2)) != -1) { //','
		parse_expression(p, i, env);
		p += i;
		p += next_token(p, &tok); //','
		return parse_expression(p, q - p, env);
	}
	if ((i = next_token_operator(p, len, t2, 1, 0, 2)) != -1) { //'='
		struct VAR v, u;
		i += next_token(p + i, &tok); //'='
		v = parse_expression(p + i, q - (p + i), env);
		p += next_token(p, &tok); //id
		for (j = env->len - 1; j >= 0; j--) {
			if (env->var[j].id != NULL && strcmp(env->var[j].id, tok.id) == 0) {
				free(tok.id);
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
				p += next_token(p, &tok); //[
				i = next_token_pair(p, ']');
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
	}
	if ((i = next_token_operator(p, len, t3, 1, 1, 2)) != -1) { //token_lor
		struct VAR v;
		v = parse_expression(p, i, env);
		if (v.type == var_float) {
			v.type = var_int;
			v.ival = (int)v.fval;
		}
		if (v.ival) {
			ret.type = var_int;
			ret.ival = 1;
			return ret;
		}
		p += i;
		p += next_token(p, &tok); //token)lor
		v = parse_expression(p, q - p, env);
		if (v.type == var_float) {
			v.type = var_int;
			v.ival = (int)v.fval;
		}
		ret.type = var_int;
		ret.ival = 0 || v.ival;
		return ret;
	}
	if ((i = next_token_operator(p, len, t4, 1, 1, 2)) != -1) { //token_land
		struct VAR v;
		v = parse_expression(p, i, env);
		if (v.type == var_float) {
			v.type = var_int;
			v.ival = (int)v.fval;
		}
		if (!v.ival) {
			ret.type = var_int;
			ret.ival = 0;
			return ret;
		}
		p += i;
		p += next_token(p, &tok); //token_land
		v = parse_expression(p, q - p, env);
		if (v.type == var_float) {
			v.type = var_int;
			v.ival = (int)v.fval;
		}
		ret.type = var_int;
		ret.ival = 1 && v.ival;
		return ret;
	}
	if ((i = next_token_operator(p, len, t5, 1, 1, 2)) != -1) { //'|'
		struct VAR v, u;
		v = parse_expression(p, i, env);
		if (v.type == var_float) {
			v.type = var_int;
			v.ival = (int)v.fval;
		}
		p += i;
		p += next_token(p, &tok); //'|'
		u = parse_expression(p, q - p, env);
		if (u.type == var_float) {
			u.type = var_int;
			u.ival = (int)u.fval;
		}
		ret.type = var_int;
		ret.ival = v.ival | u.ival;
		return ret;
	}
	if ((i = next_token_operator(p, len, t6, 1, 1, 2)) != -1) { //'^'
		struct VAR v, u;
		v = parse_expression(p, i, env);
		if (v.type == var_float) {
			v.type = var_int;
			v.ival = (int)v.fval;
		}
		p += i;
		p += next_token(p, &tok); //'^'
		u = parse_expression(p, q - p, env);
		if (u.type == var_float) {
			u.type = var_int;
			u.ival = (int)u.fval;
		}
		ret.type = var_int;
		ret.ival = v.ival ^ u.ival;
		return ret;
	}
	if ((i = next_token_operator(p, len, t7, 1, 1, 2)) != -1) { //'&'
		struct VAR v, u;
		v = parse_expression(p, i, env);
		if (v.type == var_float) {
			v.type = var_int;
			v.ival = (int)v.fval;
		}
		p += i;
		p += next_token(p, &tok); //'&'
		u = parse_expression(p, q - p, env);
		if (u.type == var_float) {
			u.type = var_int;
			u.ival = (int)u.fval;
		}
		ret.type = var_int;
		ret.ival = v.ival & u.ival;
		return ret;
	}
	if ((i = next_token_operator(p, len, t8, 1, 1, 2)) != -1) { //token_neq
		struct VAR v, u;
		v = parse_expression(p, i, env);
		p += i;
		p += next_token(p, &tok); //token_neq
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
		if (v.type == var_int)
			ret.ival = v.ival != u.ival;
		else
			ret.ival = v.fval != u.fval;
		return ret;
	}
	if ((i = next_token_operator(p, len, t9, 1, 1, 2)) != -1) { //token_eq
		struct VAR v, u;
		v = parse_expression(p, i, env);
		p += i;
		p += next_token(p, &tok); //token_eq
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
		if (v.type == var_int)
			ret.ival = v.ival == u.ival;
		else
			ret.ival = v.fval == u.fval;
		return ret;
	}
	if ((i = next_token_operator(p, len, t10, 4, 1, 2)) != -1) { //'>', token_geq, '<', token_leq
		struct VAR v, u;
		v = parse_expression(p, i, env);
		p += i;
		p += next_token(p, &tok); //'>', token_geq, '<', token_leq
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
		if (tok.type == '>') {
			if (v.type == var_int)
				ret.ival = v.ival > u.ival;
			else
				ret.ival = v.fval > u.fval;
		}
		else if (tok.type == token_geq) {
			if (v.type == var_int)
				ret.ival = v.ival >= u.ival;
			else
				ret.ival = v.fval >= u.fval;
		}
		else if (tok.type == '<') {
			if (v.type == var_int)
				ret.ival = v.ival < u.ival;
			else
				ret.ival = v.fval < u.fval;
		}
		else { //token_leq
			if (v.type == var_int)
				ret.ival = v.ival <= u.ival;
			else
				ret.ival = v.fval <= u.fval;
		}
		return ret;
	}
	if ((i = next_token_operator(p, len, t11, 2, 1, 2)) != -1) { //'+', '-'
		struct VAR v, u;
		v = parse_expression(p, i, env);
		p += i;
		p += next_token(p, &tok); //'+', '-'
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
			if (v.type == var_int)
				ret.ival = v.ival + u.ival;
			else
				ret.fval = v.fval + u.fval;
		}
		else { //'-'
			if (v.type == var_int)
				ret.ival = v.ival - u.ival;
			else
				ret.fval = v.fval - u.fval;
		}
		return ret;
	}
	if ((i = next_token_operator(p, len, t12, 3, 1, 2)) != -1) { //'*', '/', '%'
		struct VAR v, u;
		v = parse_expression(p, i, env);
		p += i;
		p += next_token(p, &tok); //'*', '/', '%'
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
		if (tok.type == '*') {
			if (v.type == var_int)
				ret.ival = v.ival * u.ival;
			else
				ret.fval = v.fval * u.fval;
		}
		else if (tok.type == '/') {
			if (v.type == var_int)
				ret.ival = v.ival / u.ival;
			else
				ret.fval = v.fval / u.fval;
		}
		else { //'%'
			if (v.type == var_int)
				ret.ival = v.ival % u.ival;
			//else log("ERROR")
		}
		return ret;
	}
	if ((i = next_token_operator(p, len, t13, 3, 0, 1)) != -1) { //'-', '!', '~'
		struct VAR v;
		p += i;
		p += next_token(p, &tok); //'-', '!', '~'
		v = parse_expression(p, q - p, env);

		if (tok.type == '-') {
			ret.type = v.type;
			if (v.type == var_int)
				ret.ival = -v.ival;
			else
				ret.fval = -v.fval;
		}
		else if (tok.type == '!') {
			ret.type = var_int;
			if (v.type == var_int)
				ret.ival = !v.ival;
			//else log("ERROR")
		}
		else { //'~'
			ret.type = var_int;
			if (v.type == var_int)
				ret.ival = ~v.ival;
			//else log("ERROR")
		}
		return ret;
	}
	//the rest is either (exp) or id or id[] or val
	//currently do not support id(...)
	p += next_token(p, &tok); //','
	if (tok.type == '(') {
		i = next_token_pair(p, ')');
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
	for (j = env->len - 1; j >= 0; j--) {
		if (env->var[j].id != NULL && strcmp(env->var[j].id, tok.id) == 0) {
			struct VAR v;
			free(tok.id);
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
			p += next_token(p, &tok); //'['
			i = next_token_pair(p, ']');
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
	return ret;
}

int parse_statements(char *p, int len, int retp, struct ENV *env) { //1 return  2 break  0 otherwise
	struct TOKEN tok;
	char *q = p + len;
	int i, ret = 0;
	
	while (p < q) {
		i = next_token(p, &tok);
		if (tok.type == ';') {
			p += i;
			continue;
		}
		if (tok.type == '{') {
			p += i;
			i = next_token_pair(p, '}');
			env_new_level(env);
			ret = parse_statements(p, i, retp, env);
			env_delete_level(env);
			if (ret) return ret;
			p += i;
			p += next_token(p, &tok); //'}'
			continue;
		}
		if (tok.type == token_if) {
			struct VAR cond;
			p += i;
			p += next_token(p, &tok); //'('
			i = next_token_pair(p, ')');
			cond = parse_expression(p, i, env);
			if (cond.type == var_float) {
				cond.type = var_int;
				cond.ival = (int)cond.fval;
			}
			p += i;
			p += next_token(p, &tok); //')'
			next_token(p, &tok);
			if (tok.type == '{') {
				p += next_token(p, &tok); //'{'
				i = next_token_pair(p, '}');
			}
			else {
				if (tok.type == token_id) free(tok.id);
				i = next_token_type(p, ';');
			}
			if (cond.ival) {
				env_new_level(env);
				ret = parse_statements(p, i, retp, env);
				env_delete_level(env);
				if (ret) return ret;
			}
			p += i;
			p += next_token(p, &tok); //'}' or ';'
			next_token(p, &tok);
			if (tok.type == token_else) {
				p += next_token(p, &tok); //else
				next_token(p, &tok);
				if (tok.type == '{') {
					p += next_token(p, &tok); //'{'
					i = next_token_pair(p, '}');
				}
				else {
					if (tok.type == token_id) free(tok.id);
					i = next_token_type(p, ';');
				}
				if (!cond.ival) {
					env_new_level(env);
					ret = parse_statements(p, i, retp, env);
					env_delete_level(env);
					if (ret) return ret;
				}
				p += i;
				p += next_token(p, &tok); //'}' or ';'
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
			p += next_token(p, &tok); //'('
			i = next_token_pair(p, ')');
			_p = p + i;
			_p += next_token(_p, &tok); //')'
			next_token(_p, &tok);
			if (tok.type == '{') {
				_p += next_token(_p, &tok); //'{'
				_i = next_token_pair(_p, '}');
			}
			else {
				if (tok.type == token_id) free(tok.id);
				_i = next_token_type(_p, ';');
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
				}
				else break;
			}
			p = _p + _i;
			p += next_token(p, &tok); //'}' or ';'
			continue;
		}
		if (tok.type == token_return) {
			struct VAR v;
			p += i;
			next_token(p, &tok);
			if (tok.type != ';') {//!="return;"
				if (tok.type == token_id) free(tok.id);
				i = next_token_type(p, ';');
				v = parse_expression(p, i, env);
				if (env->var[retp].type == var_int)
					env->var[retp].ival = (v.type == var_int ? v.ival : (int)v.fval);
				else if (env->var[retp].type == var_float)
					env->var[retp].fval = (v.type == var_float ? v.fval : (float)v.ival);
				//else log("ERROR")
			}
			return 1;
		}
		if (tok.type == token_continue) return 0;
		if (tok.type == token_break) return 2;
		if (tok.type == token_int || tok.type == token_float) {
			int t = (tok.type == token_int ? var_int : var_float);
			p += i;
			while (1) {
				p += next_token(p, &tok);
				env_new_null(env);
				env->var[env->len - 1].id = tok.id;
				env->var[env->len - 1].type = t;
				p += next_token(p, &tok);
				if (tok.type == ',') continue;
				if (tok.type == ';') break;
				if (tok.type == '[') {
					struct VAR v;
					i = next_token_pair(p, ']');
					v = parse_expression(p, i, env);
					if (v.type == var_float) {
						v.type = var_int;
						v.ival = (int)v.fval;
					}
					if (t == var_int) {
						env->var[env->len - 1].type = var_ints;
						env->var[env->len - 1].ivals = (int *)malloc(v.ival * sizeof(int));
					}
					else {
						env->var[env->len - 1].type = var_floats;
						env->var[env->len - 1].fvals = (float *)malloc(v.ival * sizeof(int));
					}
					p += i;
					p += next_token(p, &tok); //']'
				}
				else {
					env->var[env->len - 1].type = t;
				}
				p += next_token(p, &tok);
				if (tok.type == ',') continue;
				if (tok.type == ';') break;
				if (tok.type == '=') {
					//TODO
				}
				if (tok.type == ',') continue;
				if (tok.type == ';') break;
			}
			continue;
		}
		//the rest is expression
		if (tok.type == token_id) free(tok.id);
		i = next_token_type(p, ';');
		parse_expression(p, i, env);
		p += i;
		p += next_token(p, &tok); //';'
	}
	
	return ret;
}

void parse_function(char *p, struct ENV *env) {
	struct TOKEN type;
	struct TOKEN id;
	struct TOKEN tok;
	int i;
	int retp;
	
	p += next_token(p, &type);
	if (type.type == token_id) { //default function type is int
		id = type;
		type.type = token_int;
		type.id = NULL;
	}
	else {
		p += next_token(p, &id);
	}
	p += next_token(p, &tok); //'('
	p += next_token(p, &tok);
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
				p += next_token(p, &tok); //get the id
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
				p += next_token(p, &tok); //get the id
			}
			env->var[env->len - i].id = tok.id;
			do {
				p += next_token(p, &tok);
			} while (tok.type != ']');
		}
		do {
			p += next_token(p, &tok);
		} while (tok.type == ',');
	}
	retp = env->len - i - 1;
	if (type.type == token_int)
		env->var[retp].type = var_int;
	else if (type.type == token_void)
		env->var[retp].type = var_void;
	else //token_float
		env->var[retp].type = var_float;
	p += next_token(p, &tok); //'{'
	parse_statements(p, next_token_pair(p, '}'), retp, env);
	free(id.id);
}

void parse_main(char *p) {
	struct ENV env;
	int retp;
	env_init(&env);
	env_new_level(&env);
	env_new_null(&env); //store return value
	retp = env.len - 1;
	//add params to env ...
	parse_function(p, &env);
	//process the return value
	if (env.var[retp].type == var_int) {
		printf("val: %d\n", env.var[retp].ival);
	}
	else if (env.var[retp].type == var_float) {
		printf("val: %f\n", env.var[retp].fval);
	}
	else {
		printf("val: null\n");
	}
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
		while (fscanf(in, "%c", &code[len]) != EOF) {
			len++;
		}
		code[len] = '\0';
		fclose(in);
		parse_main(code);
		free(code);
	}
	return 0;
}