#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include "rtl.h"
#include "tac.h"

typedef struct StringMap
{
    char *literal;
    char *label;
    struct StringMap *next;
} StringMap;

static StringMap *string_map_head = NULL;
static int next_string_id = 0;

static char *xstrdup(const char *s)
{
    size_t n;
    char *p;

    if (!s)
        s = "";

    n = strlen(s) + 1;
    p = (char *)malloc(n);
    if (!p)
    {
        fprintf(stderr, "Out of memory in RTL generator\n");
        exit(1);
    }

    memcpy(p, s, n);
    return p;
}

static void append_text(char **dest, const char *text)
{
    size_t old_len;
    size_t add_len;
    char *buffer;

    if (!text || !*text)
        return;

    if (!*dest)
    {
        *dest = xstrdup(text);
        return;
    }

    old_len = strlen(*dest);
    add_len = strlen(text);
    buffer = (char *)realloc(*dest, old_len + add_len + 1);
    if (!buffer)
    {
        fprintf(stderr, "Out of memory in RTL generator\n");
        exit(1);
    }

    memcpy(buffer + old_len, text, add_len + 1);
    *dest = buffer;
}

static void append_linef(char **dest, const char *fmt, ...)
{
    va_list args;
    va_list copy;
    int len;
    char *line;

    va_start(args, fmt);
    va_copy(copy, args);
    len = vsnprintf(NULL, 0, fmt, copy);
    va_end(copy);

    if (len < 0)
    {
        va_end(args);
        fprintf(stderr, "Formatting error in RTL generator\n");
        exit(1);
    }

    line = (char *)malloc((size_t)len + 1);
    if (!line)
    {
        va_end(args);
        fprintf(stderr, "Out of memory in RTL generator\n");
        exit(1);
    }

    vsnprintf(line, (size_t)len + 1, fmt, args);
    va_end(args);

    append_text(dest, line);
    append_text(dest, "\n");
    free(line);
}

static void trim(char *s)
{
    size_t len;
    size_t i = 0;

    if (!s)
        return;

    while (s[i] && isspace((unsigned char)s[i]))
        i++;
    if (i > 0)
        memmove(s, s + i, strlen(s + i) + 1);

    len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1]))
    {
        s[len - 1] = '\0';
        len--;
    }
}

static int is_integer_literal(const char *s)
{
    size_t i = 0;

    if (!s || !*s)
        return 0;

    if (s[i] == '+' || s[i] == '-')
        i++;

    if (!isdigit((unsigned char)s[i]))
        return 0;

    for (; s[i] != '\0'; i++)
    {
        if (!isdigit((unsigned char)s[i]))
            return 0;
    }

    return 1;
}

static int is_string_literal(const char *s)
{
    size_t len;

    if (!s)
        return 0;

    len = strlen(s);
    return len >= 2 && s[0] == '"' && s[len - 1] == '"';
}

static const char *intern_string_label(const char *literal)
{
    StringMap *cur = string_map_head;
    char name[32];
    StringMap *node;

    while (cur)
    {
        if (strcmp(cur->literal, literal) == 0)
            return cur->label;
        cur = cur->next;
    }

    node = (StringMap *)malloc(sizeof(StringMap));
    if (!node)
    {
        fprintf(stderr, "Out of memory in RTL generator\n");
        exit(1);
    }

    snprintf(name, sizeof(name), "_str_%d", next_string_id++);
    node->literal = xstrdup(literal);
    node->label = xstrdup(name);
    node->next = string_map_head;
    string_map_head = node;

    return node->label;
}

static void free_string_map(void)
{
    StringMap *cur = string_map_head;

    while (cur)
    {
        StringMap *next = cur->next;
        free(cur->literal);
        free(cur->label);
        free(cur);
        cur = next;
    }

    string_map_head = NULL;
}

static void emit_load_operand(const char *reg, const char *operand, char **out)
{
    if (is_integer_literal(operand))
        append_linef(out, "    iLoad:  %s <- %s", reg, operand);
    else
        append_linef(out, "    load:   %s <- %s", reg, operand);
}

static void emit_store_symbol(const char *symbol, const char *reg, char **out)
{
    append_linef(out, "    store:  %s <- %s", symbol, reg);
}

typedef struct TempRegMap
{
    char name[32];
    char reg[8];
    struct TempRegMap *next;
} TempRegMap;

typedef struct
{
    TempRegMap *head;
    int next_reg_index;
    char procedure_name[128];
    int has_procedure_name;
} RtlState;

static int is_temp_name(const char *s)
{
    size_t i = 0;

    if (!s)
        return 0;

    if (strncmp(s, "temp", 4) != 0)
        return 0;

    i = 4;
    if (!isdigit((unsigned char)s[i]))
        return 0;

    for (; s[i] != '\0'; i++)
    {
        if (!isdigit((unsigned char)s[i]))
            return 0;
    }

    return 1;
}

static const char *lookup_temp_reg(RtlState *state, const char *name)
{
    TempRegMap *cur;

    if (!state || !name)
        return NULL;

    cur = state->head;
    while (cur)
    {
        if (strcmp(cur->name, name) == 0)
            return cur->reg;
        cur = cur->next;
    }

    return NULL;
}

static const char *assign_temp_reg(RtlState *state, const char *name)
{
    TempRegMap *node;
    char reg_name[8];

    if (!state || !name)
        return "t0";

    if (lookup_temp_reg(state, name))
        return lookup_temp_reg(state, name);

    node = (TempRegMap *)malloc(sizeof(TempRegMap));
    if (!node)
    {
        fprintf(stderr, "Out of memory in RTL generator\n");
        exit(1);
    }

    snprintf(reg_name, sizeof(reg_name), "t%d", state->next_reg_index % 10);
    state->next_reg_index++;

    snprintf(node->name, sizeof(node->name), "%s", name);
    snprintf(node->reg, sizeof(node->reg), "%s", reg_name);
    node->next = state->head;
    state->head = node;

    return node->reg;
}

static const char *map_temp_to_reg(RtlState *state, const char *name, const char *reg)
{
    TempRegMap *cur;
    TempRegMap *node;

    if (!state || !name || !reg)
        return "t0";

    cur = state->head;
    while (cur)
    {
        if (strcmp(cur->name, name) == 0)
        {
            snprintf(cur->reg, sizeof(cur->reg), "%s", reg);
            return cur->reg;
        }
        cur = cur->next;
    }

    node = (TempRegMap *)malloc(sizeof(TempRegMap));
    if (!node)
    {
        fprintf(stderr, "Out of memory in RTL generator\n");
        exit(1);
    }

    snprintf(node->name, sizeof(node->name), "%s", name);
    snprintf(node->reg, sizeof(node->reg), "%s", reg);
    node->next = state->head;
    state->head = node;

    return node->reg;
}

static void free_temp_reg_map(RtlState *state)
{
    TempRegMap *cur;

    if (!state)
        return;

    cur = state->head;
    while (cur)
    {
        TempRegMap *next = cur->next;
        free(cur);
        cur = next;
    }

    state->head = NULL;
    state->next_reg_index = 0;
}

static const char *emit_operand_as_reg(const char *preferred_reg, const char *operand, RtlState *state, char **out)
{
    const char *mapped = NULL;

    if (is_temp_name(operand))
    {
        mapped = lookup_temp_reg(state, operand);
        if (mapped)
            return mapped;
    }

    emit_load_operand(preferred_reg, operand, out);
    return preferred_reg;
}

static const char *pick_scratch_reg(const char *avoid_reg)
{
    if (!avoid_reg || strcmp(avoid_reg, "t1") != 0)
        return "t1";
    return "t2";
}

static const char *pick_result_reg(const char *lhs_reg, const char *rhs_reg)
{
    if (strcmp("t0", lhs_reg) != 0 && strcmp("t0", rhs_reg) != 0)
        return "t0";
    if (strcmp("t1", lhs_reg) != 0 && strcmp("t1", rhs_reg) != 0)
        return "t1";
    return "t2";
}

static const char *arith_to_rtl(const char *op)
{
    if (strcmp(op, "+") == 0)
        return "add";
    if (strcmp(op, "-") == 0)
        return "sub";
    if (strcmp(op, "*") == 0)
        return "mul";
    if (strcmp(op, "/") == 0)
        return "div";
    if (strcmp(op, "&&") == 0)
        return "and";
    if (strcmp(op, "||") == 0)
        return "or";
    return NULL;
}

static const char *relop_to_rtl(const char *op)
{
    if (strcmp(op, "<") == 0)
        return "slt";
    if (strcmp(op, "<=") == 0)
        return "sle";
    if (strcmp(op, ">") == 0)
        return "sgt";
    if (strcmp(op, ">=") == 0)
        return "sge";
    if (strcmp(op, "==") == 0)
        return "seq";
    if (strcmp(op, "!=") == 0)
        return "sne";
    return NULL;
}

static void emit_print(const char *value, RtlState *state, char **out)
{
    char local[384];
    const char *mapped_temp = NULL;

    snprintf(local, sizeof(local), "%s", value ? value : "");
    trim(local);

    if (is_string_literal(local))
    {
        const char *label = intern_string_label(local);
        append_linef(out, "    iLoad:  v0 <- 4        ;; syscall: print string");
        append_linef(out, "    load_addr: a0 <- %s", label);
        append_linef(out, "    write");
        return;
    }

    append_linef(out, "    iLoad:  v0 <- 1        ;; syscall: print integer");
    if (is_integer_literal(local))
        append_linef(out, "    iLoad:  a0 <- %s", local);
    else if (is_temp_name(local) && (mapped_temp = lookup_temp_reg(state, local)) != NULL)
        append_linef(out, "    move:   a0 <- %s", mapped_temp);
    else
        append_linef(out, "    load:   a0 <- %s", local);
    append_linef(out, "    write");
}

static void emit_copy_assign(const char *dst, const char *rhs, RtlState *state, char **out)
{
    const char *rhs_reg = emit_operand_as_reg("v0", rhs, state, out);

    if (is_temp_name(dst))
    {
        const char *dst_reg = assign_temp_reg(state, dst);

        if (strcmp(dst_reg, rhs_reg) != 0)
            append_linef(out, "    move:   %s <- %s", dst_reg, rhs_reg);
        return;
    }

    emit_store_symbol(dst, rhs_reg, out);
}

static void emit_unary_assign(const char *dst, const char *op, const char *rhs, RtlState *state, char **out)
{
    const char *src_reg = emit_operand_as_reg("v0", rhs, state, out);
    const char *dst_reg = NULL;

    if (is_temp_name(dst))
    {
        if (strcmp(op, "!") == 0)
            dst_reg = map_temp_to_reg(state, dst, "v0");
        else
            dst_reg = assign_temp_reg(state, dst);
    }
    else
    {
        dst_reg = "t0";
    }

    if (strcmp(op, "-") == 0)
        append_linef(out, "    uminus: %s <- %s", dst_reg, src_reg);
    else
        append_linef(out, "    not:    %s <- %s", dst_reg, src_reg);

    if (!is_temp_name(dst))
        emit_store_symbol(dst, dst_reg, out);
}

static void emit_binary_assign(const char *dst, const char *lhs, const char *op, const char *rhs, RtlState *state, char **out)
{
    const char *rtl_op = arith_to_rtl(op);
    const char *lhs_reg;
    const char *rhs_reg;
    const char *dst_reg;
    const char *rhs_scratch;

    if (!rtl_op)
        rtl_op = relop_to_rtl(op);

    if (!rtl_op)
    {
        emit_copy_assign(dst, rhs, state, out);
        return;
    }

    lhs_reg = emit_operand_as_reg("v0", lhs, state, out);
    rhs_scratch = pick_scratch_reg(lhs_reg);
    rhs_reg = emit_operand_as_reg(rhs_scratch, rhs, state, out);
    if (is_temp_name(dst))
    {
        dst_reg = assign_temp_reg(state, dst);
        if (strcmp(dst_reg, lhs_reg) == 0 || strcmp(dst_reg, rhs_reg) == 0)
            dst_reg = map_temp_to_reg(state, dst, pick_result_reg(lhs_reg, rhs_reg));
    }
    else
    {
        dst_reg = "t0";
    }

    append_linef(out, "    %s:    %s <- %s, %s", rtl_op, dst_reg, lhs_reg, rhs_reg);
    if (!is_temp_name(dst))
        emit_store_symbol(dst, dst_reg, out);
}

static void parse_assign_line(const char *buf, RtlState *state, char **out)
{
    char dst[128];
    char rhs[384];
    char a[128], b[128], op[8];
    char unary_rhs[256];

    if (sscanf(buf, "%127s = %383[^\n]", dst, rhs) != 2)
        return;

    trim(dst);
    trim(rhs);

    if (rhs[0] == '!' || rhs[0] == '-')
    {
        snprintf(unary_rhs, sizeof(unary_rhs), "%s", rhs + 1);
        trim(unary_rhs);
        emit_unary_assign(dst, (rhs[0] == '!') ? "!" : "-", unary_rhs, state, out);
        return;
    }

    if (sscanf(rhs, "%127s %7s %127s", a, op, b) == 3)
    {
        emit_binary_assign(dst, a, op, b, state, out);
        return;
    }

    emit_copy_assign(dst, rhs, state, out);
}

static void parse_tac_line(const char *line, RtlState *state, char **out)
{
    char buf[512];
    char cond[128], label[128], dst[128];
    char proc_name[128], proc_kw[128];
    const char *cond_reg = NULL;

    if (!line)
        return;

    snprintf(buf, sizeof(buf), "%s", line);
    trim(buf);

    if (!*buf)
        return;

    if (sscanf(buf, "proc %127s %127s", proc_name, proc_kw) == 2)
    {
        if (strcmp(proc_kw, "begin") == 0)
        {
            snprintf(state->procedure_name, sizeof(state->procedure_name), "%s", proc_name);
            state->has_procedure_name = 1;
        }
        return;
    }

    if (strcmp(buf, "return") == 0)
    {
        append_linef(out, "    return");
        return;
    }

    if (buf[strlen(buf) - 1] == ':')
    {
        append_linef(out, "%s", buf);
        return;
    }

    if (sscanf(buf, "goto %127s", label) == 1)
    {
        append_linef(out, "    goto:   %s", label);
        return;
    }

    if (sscanf(buf, "if %127s goto %127s", cond, label) == 2)
    {
        cond_reg = emit_operand_as_reg("v0", cond, state, out);
        append_linef(out, "    bgtz:   %s, %s", cond_reg, label);
        return;
    }

    if (sscanf(buf, "read %127s", dst) == 1)
    {
        append_linef(out, "    iLoad:  v0 <- 5        ;; syscall: read integer");
        append_linef(out, "    read");
        append_linef(out, "    store:  %s <- v0", dst);
        return;
    }

    if (strncmp(buf, "print ", 6) == 0)
    {
        emit_print(buf + 6, state, out);
        return;
    }

    parse_assign_line(buf, state, out);
}

static char *tac_to_rtl(Ast *root, char *procedure_name, size_t procedure_name_size)
{
    FILE *fp;
    char line[512];
    char *rtl = NULL;
    RtlState state = {NULL, 0, "", 0};

    fp = tmpfile();
    if (!fp)
        return xstrdup("");

    tac_generate(root, fp);
    rewind(fp);

    while (fgets(line, sizeof(line), fp))
        parse_tac_line(line, &state, &rtl);

    fclose(fp);

    if (procedure_name && procedure_name_size > 0)
    {
        if (state.has_procedure_name)
            snprintf(procedure_name, procedure_name_size, "%s", state.procedure_name);
        else
            procedure_name[0] = '\0';
    }

    free_temp_reg_map(&state);

    if (!rtl)
        rtl = xstrdup("");
    return rtl;
}

void rtl_reset_counters(void)
{
    next_string_id = 0;
    free_string_map();
}

void rtl_generate(Ast *root, FILE *out)
{
    char *code;
    char procedure_name[128] = "";

    if (!out)
        return;

    rtl_reset_counters();
    code = tac_to_rtl(root, procedure_name, sizeof(procedure_name));

    if (procedure_name[0] != '\0')
    {
        fputs("*PROCEDURE: ", out);
        fputs(procedure_name, out);
        fputs("\n", out);
    }

    fputs("**BEGIN: RTL Statements\n\n", out);
    fputs(code, out);
    fputs("\n**END: RTL Statements\n", out);

    free(code);
}

int rtl_generate_to_path(Ast *root, const char *path)
{
    FILE *fp;

    if (!path)
        return 0;

    fp = fopen(path, "w");
    if (!fp)
        return 0;

    rtl_generate(root, fp);
    fclose(fp);
    return 1;
}
