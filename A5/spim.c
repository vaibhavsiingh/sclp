#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "spim.h"
#include "rtl.h"
#include "tac.h"

typedef struct NameNode
{
    char *name;
    int is_float;
    struct NameNode *next;
} NameNode;

typedef struct StringNode
{
    char *label;
    char *value;
    struct StringNode *next;
} StringNode;

typedef struct LineNode
{
    char *line;
    struct LineNode *next;
} LineNode;

static void *checked_malloc(size_t size)
{
    void *ptr = malloc(size);
    if (!ptr)
    {
        fprintf(stderr, "Out of memory in SPIM generator\n");
        exit(1);
    }
    return ptr;
}

static char *xstrdup(const char *s)
{
    size_t n;
    char *p;

    if (!s)
        s = "";

    n = strlen(s) + 1;
    p = (char *)checked_malloc(n);
    memcpy(p, s, n);
    return p;
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

static int is_float_literal(const char *s)
{
    size_t i = 0;
    int seen_digit = 0;
    int seen_dot = 0;

    if (!s || !*s)
        return 0;

    if (s[i] == '+' || s[i] == '-')
        i++;

    for (; s[i] != '\0'; i++)
    {
        if (isdigit((unsigned char)s[i]))
        {
            seen_digit = 1;
            continue;
        }

        if (s[i] == '.')
        {
            if (seen_dot)
                return 0;
            seen_dot = 1;
            continue;
        }

        return 0;
    }

    return seen_digit && seen_dot;
}

static int is_label_name(const char *s)
{
    size_t i = 0;

    if (!s)
        return 0;

    if (strncmp(s, "Label", 5) != 0)
        return 0;

    i = 5;
    if (!isdigit((unsigned char)s[i]))
        return 0;

    for (; s[i] != '\0'; i++)
    {
        if (!isdigit((unsigned char)s[i]))
            return 0;
    }

    return 1;
}

static int is_string_label(const char *s)
{
    size_t i = 0;

    if (!s)
        return 0;

    if (strncmp(s, "_str_", 5) != 0)
        return 0;

    i = 5;
    if (!isdigit((unsigned char)s[i]))
        return 0;

    for (; s[i] != '\0'; i++)
    {
        if (!isdigit((unsigned char)s[i]))
            return 0;
    }

    return 1;
}

static int is_register_name(const char *s)
{
    if (!s || !*s)
        return 0;

    if (strcmp(s, "zero") == 0 || strcmp(s, "ra") == 0 || strcmp(s, "sp") == 0 ||
        strcmp(s, "gp") == 0 || strcmp(s, "fp") == 0 || strcmp(s, "at") == 0 ||
        strcmp(s, "k0") == 0 || strcmp(s, "k1") == 0)
        return 1;

    if ((s[0] == 'v' && s[1] >= '0' && s[1] <= '1' && s[2] == '\0') ||
        (s[0] == 'a' && s[1] >= '0' && s[1] <= '3' && s[2] == '\0') ||
        (s[0] == 's' && s[1] >= '0' && s[1] <= '7' && s[2] == '\0'))
        return 1;

    if (s[0] == 't' && isdigit((unsigned char)s[1]))
    {
        int n = atoi(s + 1);
        return n >= 0 && n <= 9;
    }

    if (s[0] == 'f' && isdigit((unsigned char)s[1]))
    {
        int n = atoi(s + 1);
        return n >= 0 && n <= 31;
    }

    return 0;
}

static void format_reg(const char *name, char *out, size_t out_size)
{
    if (!out || out_size == 0)
        return;

    if (!name)
    {
        snprintf(out, out_size, "$zero");
        return;
    }

    if (name[0] == '$')
    {
        snprintf(out, out_size, "%s", name);
        return;
    }

    snprintf(out, out_size, "$%s", name);
}

static int name_exists(NameNode *head, const char *name)
{
    NameNode *cur = head;

    while (cur)
    {
        if (strcmp(cur->name, name) == 0)
            return 1;
        cur = cur->next;
    }

    return 0;
}

static void add_symbol(NameNode **head, const char *name, int is_float)
{
    NameNode *node;

    if (!head || !name || !*name)
        return;

    if (is_register_name(name) || is_integer_literal(name) || is_float_literal(name) ||
        is_label_name(name) || is_string_label(name))
        return;

    if (name_exists(*head, name))
    {
        if (is_float)
        {
            NameNode *cur = *head;
            while (cur)
            {
                if (strcmp(cur->name, name) == 0)
                {
                    cur->is_float = 1;
                    return;
                }
                cur = cur->next;
            }
        }
        return;
    }

    node = (NameNode *)checked_malloc(sizeof(NameNode));
    node->name = xstrdup(name);
    node->is_float = is_float;
    node->next = *head;
    *head = node;
}

static StringNode *find_string_by_label(StringNode *head, const char *label)
{
    StringNode *cur = head;

    while (cur)
    {
        if (strcmp(cur->label, label) == 0)
            return cur;
        cur = cur->next;
    }

    return NULL;
}

static StringNode *find_string_by_value(StringNode *head, const char *value)
{
    StringNode *cur = head;

    while (cur)
    {
        if (strcmp(cur->value, value) == 0)
            return cur;
        cur = cur->next;
    }

    return NULL;
}

static void add_string_literal(StringNode **head, const char *label, const char *value)
{
    StringNode *node;

    if (!head || !label || !value)
        return;

    if (find_string_by_label(*head, label))
        return;

    node = (StringNode *)checked_malloc(sizeof(StringNode));
    node->label = xstrdup(label);
    node->value = xstrdup(value);
    node->next = *head;
    *head = node;
}

static void append_line(LineNode **head, LineNode **tail, const char *line)
{
    LineNode *node;

    if (!head || !tail || !line)
        return;

    node = (LineNode *)checked_malloc(sizeof(LineNode));
    node->line = xstrdup(line);
    node->next = NULL;

    if (!*head)
        *head = node;
    else
        (*tail)->next = node;

    *tail = node;
}

static void free_lines(LineNode *head)
{
    LineNode *cur = head;

    while (cur)
    {
        LineNode *next = cur->next;
        free(cur->line);
        free(cur);
        cur = next;
    }
}

static void free_symbols(NameNode *head)
{
    NameNode *cur = head;

    while (cur)
    {
        NameNode *next = cur->next;
        free(cur->name);
        free(cur);
        cur = next;
    }
}

static void free_strings(StringNode *head)
{
    StringNode *cur = head;

    while (cur)
    {
        StringNode *next = cur->next;
        free(cur->label);
        free(cur->value);
        free(cur);
        cur = next;
    }
}

static void write_escaped_string(FILE *out, const char *s)
{
    const unsigned char *p = (const unsigned char *)s;

    fputc('"', out);
    while (*p)
    {
        if (*p == '\\' || *p == '"')
            fprintf(out, "\\%c", *p);
        else if (*p == '\n')
            fputs("\\n", out);
        else if (*p == '\t')
            fputs("\\t", out);
        else
            fputc(*p, out);
        p++;
    }
    fputc('"', out);
}

static void collect_string_literals_from_tac(Ast *root, StringNode **strings)
{
    FILE *tmp;
    char line[1024];
    int idx = 0;

    if (!root || !strings)
        return;

    tmp = tmpfile();
    if (!tmp)
        return;

    tac_generate(root, tmp);
    rewind(tmp);

    while (fgets(line, sizeof(line), tmp))
    {
        char *start = strchr(line, '"');
        char *end = strrchr(line, '"');
        char label[64];

        if (!start || !end || end == start)
            continue;

        *end = '\0';
        start++;
        if (!*start)
            continue;

        if (find_string_by_value(*strings, start))
            continue;

        snprintf(label, sizeof(label), "_str_%d", idx++);
        add_string_literal(strings, label, start);
    }

    fclose(tmp);
}

static void emit_op2(const char *op, const char *dst, const char *src,
                     NameNode **symbols, StringNode **strings,
                     LineNode **text_head, LineNode **text_tail)
{
    char out[512];
    char rd[64], rs[64];

    if (strcmp(op, "iLoad") == 0)
    {
        format_reg(dst, rd, sizeof(rd));
        snprintf(out, sizeof(out), "    li %s, %s", rd, src);
        append_line(text_head, text_tail, out);
        return;
    }

    if (strcmp(op, "iLoad.d") == 0)
    {
        format_reg(dst, rd, sizeof(rd));
        snprintf(out, sizeof(out), "    li.s %s, %s", rd, src);
        append_line(text_head, text_tail, out);
        return;
    }

    if (strcmp(op, "load") == 0)
    {
        add_symbol(symbols, src, 0);
        format_reg(dst, rd, sizeof(rd));
        snprintf(out, sizeof(out), "    lw %s, %s", rd, src);
        append_line(text_head, text_tail, out);
        return;
    }

    if (strcmp(op, "load.d") == 0)
    {
        add_symbol(symbols, src, 1);
        format_reg(dst, rd, sizeof(rd));
        snprintf(out, sizeof(out), "    l.s %s, %s", rd, src);
        append_line(text_head, text_tail, out);
        return;
    }

    if (strcmp(op, "store") == 0)
    {
        add_symbol(symbols, dst, 0);
        format_reg(src, rs, sizeof(rs));
        snprintf(out, sizeof(out), "    sw %s, %s", rs, dst);
        append_line(text_head, text_tail, out);
        return;
    }

    if (strcmp(op, "store.d") == 0)
    {
        add_symbol(symbols, dst, 1);
        format_reg(src, rs, sizeof(rs));
        snprintf(out, sizeof(out), "    s.s %s, %s", rs, dst);
        append_line(text_head, text_tail, out);
        return;
    }

    if (strcmp(op, "load_addr") == 0)
    {
        format_reg(dst, rd, sizeof(rd));
        if (is_string_label(src) && !find_string_by_label(*strings, src))
            add_string_literal(strings, src, "");
        snprintf(out, sizeof(out), "    la %s, %s", rd, src);
        append_line(text_head, text_tail, out);
        return;
    }

    if (strcmp(op, "move") == 0)
    {
        format_reg(dst, rd, sizeof(rd));
        format_reg(src, rs, sizeof(rs));
        snprintf(out, sizeof(out), "    move %s, %s", rd, rs);
        append_line(text_head, text_tail, out);
        return;
    }

    if (strcmp(op, "move.d") == 0)
    {
        format_reg(dst, rd, sizeof(rd));
        format_reg(src, rs, sizeof(rs));
        snprintf(out, sizeof(out), "    mov.s %s, %s", rd, rs);
        append_line(text_head, text_tail, out);
        return;
    }

    if (strcmp(op, "uminus") == 0)
    {
        format_reg(dst, rd, sizeof(rd));
        format_reg(src, rs, sizeof(rs));
        snprintf(out, sizeof(out), "    sub %s, $zero, %s", rd, rs);
        append_line(text_head, text_tail, out);
        return;
    }

    if (strcmp(op, "uminus.d") == 0)
    {
        format_reg(dst, rd, sizeof(rd));
        format_reg(src, rs, sizeof(rs));
        snprintf(out, sizeof(out), "    neg.s %s, %s", rd, rs);
        append_line(text_head, text_tail, out);
        return;
    }

    if (strcmp(op, "not") == 0)
    {
        format_reg(dst, rd, sizeof(rd));
        format_reg(src, rs, sizeof(rs));
        snprintf(out, sizeof(out), "    seq %s, %s, $zero", rd, rs);
        append_line(text_head, text_tail, out);
        return;
    }

    snprintf(out, sizeof(out), "    # unsupported op2 %s: %s <- %s", op, dst, src);
    append_line(text_head, text_tail, out);
}

static void emit_op3(const char *op, const char *dst, const char *src1, const char *src2,
                     LineNode **text_head, LineNode **text_tail)
{
    char out[512];
    char rd[64], r1[64], r2[64];

    format_reg(dst, rd, sizeof(rd));
    format_reg(src1, r1, sizeof(r1));
    format_reg(src2, r2, sizeof(r2));

    if (strcmp(op, "add") == 0 || strcmp(op, "sub") == 0 || strcmp(op, "mul") == 0 ||
        strcmp(op, "and") == 0 || strcmp(op, "or") == 0 || strcmp(op, "slt") == 0 ||
        strcmp(op, "sle") == 0 || strcmp(op, "sgt") == 0 || strcmp(op, "sge") == 0 ||
        strcmp(op, "seq") == 0 || strcmp(op, "sne") == 0)
    {
        snprintf(out, sizeof(out), "    %s %s, %s, %s", op, rd, r1, r2);
        append_line(text_head, text_tail, out);
        return;
    }

    if (strcmp(op, "div") == 0)
    {
        snprintf(out, sizeof(out), "    div %s, %s", r1, r2);
        append_line(text_head, text_tail, out);
        snprintf(out, sizeof(out), "    mflo %s", rd);
        append_line(text_head, text_tail, out);
        return;
    }

    if (strcmp(op, "add.d") == 0 || strcmp(op, "sub.d") == 0 ||
        strcmp(op, "mul.d") == 0 || strcmp(op, "div.d") == 0)
    {
        char mop[16];
        snprintf(mop, sizeof(mop), "%c%c%c.s", op[0], op[1], op[2]);
        snprintf(out, sizeof(out), "    %s %s, %s, %s", mop, rd, r1, r2);
        append_line(text_head, text_tail, out);
        return;
    }

    if (strcmp(op, "movf") == 0 || strcmp(op, "movt") == 0)
    {
        if (src2 && *src2)
            snprintf(out, sizeof(out), "    %s %s, %s, %s", op, rd, r1, src2);
        else
            snprintf(out, sizeof(out), "    %s %s, %s", op, rd, r1);
        append_line(text_head, text_tail, out);
        return;
    }

    snprintf(out, sizeof(out), "    # unsupported op3 %s: %s <- %s, %s", op, dst, src1, src2);
    append_line(text_head, text_tail, out);
}

static void emit_op2_comma(const char *op, const char *src1, const char *src2,
                           LineNode **text_head, LineNode **text_tail)
{
    char out[512];
    char r1[64], r2[64];

    format_reg(src1, r1, sizeof(r1));
    format_reg(src2, r2, sizeof(r2));

    if (strcmp(op, "seq.d") == 0)
        snprintf(out, sizeof(out), "    c.eq.s %s, %s", r1, r2);
    else if (strcmp(op, "slt.d") == 0)
        snprintf(out, sizeof(out), "    c.lt.s %s, %s", r1, r2);
    else if (strcmp(op, "sle.d") == 0)
        snprintf(out, sizeof(out), "    c.le.s %s, %s", r1, r2);
    else
        snprintf(out, sizeof(out), "    # unsupported op2-comma %s: %s, %s", op, src1, src2);

    append_line(text_head, text_tail, out);
}

static void parse_rtl_and_emit(Ast *root, NameNode **symbols, StringNode **strings,
                               LineNode **text_head, LineNode **text_tail)
{
    FILE *tmp;
    char line[1024];

    if (!root)
        return;

    tmp = tmpfile();
    if (!tmp)
        return;

    rtl_generate(root, tmp);
    rewind(tmp);

    while (fgets(line, sizeof(line), tmp))
    {
        char op[64], a[256], b[256], c[256];
        char *comment;

        trim(line);
        if (line[0] == '\0' || line[0] == '*')
            continue;

        comment = strstr(line, ";;");
        if (comment)
            *comment = '\0';
        trim(line);
        if (line[0] == '\0')
            continue;

        if (sscanf(line, "goto: %255s", a) == 1)
        {
            char out[320];
            snprintf(out, sizeof(out), "    b %s", a);
            append_line(text_head, text_tail, out);
            continue;
        }

        if (sscanf(line, "bgtz: %255[^,], %255s", a, b) == 2)
        {
            char out[320];
            char rc[64];
            trim(a);
            trim(b);
            format_reg(a, rc, sizeof(rc));
            snprintf(out, sizeof(out), "    bgtz %s, %s", rc, b);
            append_line(text_head, text_tail, out);
            continue;
        }

        if (strcmp(line, "write") == 0 || strcmp(line, "read") == 0)
        {
            append_line(text_head, text_tail, "    syscall");
            continue;
        }

        if (strcmp(line, "return") == 0)
        {
            append_line(text_head, text_tail, "    li $v0, 10");
            append_line(text_head, text_tail, "    syscall");
            continue;
        }

        if (sscanf(line, "%63[^:]: %255s <- %255[^,], %255s", op, a, b, c) == 4)
        {
            trim(op);
            trim(a);
            trim(b);
            trim(c);
            emit_op3(op, a, b, c, text_head, text_tail);
            continue;
        }

        if (sscanf(line, "%63[^:]: %255[^,], %255s", op, a, b) == 3)
        {
            trim(op);
            trim(a);
            trim(b);
            emit_op2_comma(op, a, b, text_head, text_tail);
            continue;
        }

        if (sscanf(line, "%63[^:]: %255s <- %255s", op, a, b) == 3)
        {
            trim(op);
            trim(a);
            trim(b);
            emit_op2(op, a, b, symbols, strings, text_head, text_tail);
            continue;
        }

        if (line[strlen(line) - 1] == ':')
        {
            append_line(text_head, text_tail, line);
            continue;
        }

        {
            char out[1080];
            snprintf(out, sizeof(out), "    # unparsed RTL: %s", line);
            append_line(text_head, text_tail, out);
        }
    }

    fclose(tmp);
}

void spim_generate(Ast *root, FILE *out)
{
    NameNode *symbols = NULL;
    StringNode *strings = NULL;
    LineNode *text_head = NULL;
    LineNode *text_tail = NULL;
    NameNode *sym;
    StringNode *str;
    LineNode *line;

    if (!out)
        return;

    collect_string_literals_from_tac(root, &strings);
    parse_rtl_and_emit(root, &symbols, &strings, &text_head, &text_tail);

    fputs(".data\n", out);

    for (str = strings; str; str = str->next)
    {
        fprintf(out, "%s: .asciiz ", str->label);
        write_escaped_string(out, str->value);
        fputc('\n', out);
    }

    for (sym = symbols; sym; sym = sym->next)
    {
        if (sym->is_float)
            fprintf(out, "%s: .float 0.0\n", sym->name);
        else
            fprintf(out, "%s: .word 0\n", sym->name);
    }

    fputs("\n.text\n", out);
    fputs(".globl main\n", out);
    fputs("main:\n", out);

    for (line = text_head; line; line = line->next)
        fprintf(out, "%s\n", line->line);

    fputs("    li $v0, 10\n", out);
    fputs("    syscall\n", out);

    free_lines(text_head);
    free_symbols(symbols);
    free_strings(strings);
}

int spim_generate_to_path(Ast *root, const char *path)
{
    FILE *fp;

    if (!path)
        return 0;

    fp = fopen(path, "w");
    if (!fp)
        return 0;

    spim_generate(root, fp);
    fclose(fp);
    return 1;
}
