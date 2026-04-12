#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "spim.h"
#include "rtl.h"
#include "symbol_table.h"

/* ============================================================ */
/* Utility Functions                                            */
/* ============================================================ */

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

/* ============================================================ */
/* Data Symbol Collection                                       */
/* ============================================================ */

typedef struct Asm_Symbol
{
    char *name;
    struct Asm_Symbol *next;
} Asm_Symbol;

static int is_register_name(const char *name)
{
    size_t i;

    if (!name || !*name)
        return 0;

    if (strcmp(name, "sp") == 0 || strcmp(name, "fp") == 0 || strcmp(name, "ra") == 0 || strcmp(name, "zero") == 0)
        return 1;

    if ((name[0] == 't' || name[0] == 's' || name[0] == 'a' || name[0] == 'v' || name[0] == 'f') && isdigit((unsigned char)name[1]))
    {
        for (i = 2; name[i] != '\0'; i++)
        {
            if (!isdigit((unsigned char)name[i]))
                return 0;
        }
        return 1;
    }

    return 0;
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

static int is_string_literal(const char *s)
{
    size_t len;

    if (!s)
        return 0;

    len = strlen(s);
    return len >= 2 && s[0] == '"' && s[len - 1] == '"';
}

static int is_temp_name(const char *s)
{
    size_t i = 0;

    if (!s)
        return 0;

    if (strcmp(s, "extra") == 0)
        return 1;

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

static int is_saved_temp_name(const char *s)
{
    size_t i = 0;

    if (!s)
        return 0;

    if (strncmp(s, "stemp", 5) != 0)
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

static int is_codegen_label_name(const char *name)
{
    size_t i;

    if (!name)
        return 0;

    if (strncmp(name, "Label", 5) != 0)
        return 0;

    if (!isdigit((unsigned char)name[5]))
        return 0;

    for (i = 6; name[i] != '\0'; i++)
    {
        if (!isdigit((unsigned char)name[i]))
            return 0;
    }

    return 1;
}

static int is_data_symbol_name(const char *name)
{
    if (!name || !*name)
        return 0;
    if (is_register_name(name))
        return 0;
    if (is_integer_literal(name) || is_float_literal(name) || is_string_literal(name))
        return 0;
    if (is_temp_name(name) || is_saved_temp_name(name))
        return 0;
    if (is_codegen_label_name(name))
        return 0;
    if (strncmp(name, "_str_", 5) == 0)
        return 0;
    return 1;
}

static void add_asm_symbol(Asm_Symbol **head, const char *name)
{
    Asm_Symbol *cur;
    Asm_Symbol *prev = NULL;
    Asm_Symbol *node;

    if (!head || !is_data_symbol_name(name))
        return;

    for (cur = *head; cur; cur = cur->next)
    {
        if (strcmp(cur->name, name) == 0)
            return;
        if (strcmp(cur->name, name) > 0)
            break;
        prev = cur;
    }

    node = (Asm_Symbol *)checked_malloc(sizeof(Asm_Symbol));
    node->name = xstrdup(name);
    if (!prev)
    {
        node->next = *head;
        *head = node;
    }
    else
    {
        node->next = prev->next;
        prev->next = node;
    }
}

static void free_asm_symbols(Asm_Symbol *head)
{
    Asm_Symbol *cur = head;

    while (cur)
    {
        Asm_Symbol *next = cur->next;
        free(cur->name);
        free(cur);
        cur = next;
    }
}

static void collect_data_symbols_from_seq(const Rtl_Seq *seq, Asm_Symbol **head)
{
    Rtl *cur;

    if (!seq || !head)
        return;

    for (cur = seq->head; cur; cur = cur->next)
    {
        if (cur->kind == RTL_OP2)
        {
            Rtl_Op2 *ins = (Rtl_Op2 *)cur;

            if (strcmp(ins->op, "load") == 0 || strcmp(ins->op, "load.d") == 0)
                add_asm_symbol(head, ins->src);
            else if (strcmp(ins->op, "store") == 0 || strcmp(ins->op, "store.d") == 0)
                add_asm_symbol(head, ins->dst);
        }
    }
}

/* ============================================================ */
/* Local Variable Frame Addressing                             */
/* ============================================================ */

static int local_offset_for_symbol(const Procedure_Ast *proc, const char *name, int *offset)
{
    Ast_List *cur;
    int slot = 0;

    if (!proc || !name)
        return 0;

    cur = proc->locals;
    while (cur)
    {
        if (cur->stmt && cur->stmt->kind == AST_NAME)
        {
            Name_Ast *local = (Name_Ast *)cur->stmt;
            slot++;
            if (local->entry && local->entry->name && strcmp(local->entry->name, name) == 0)
            {
                if (offset)
                    *offset = -4 * slot;
                return 1;
            }
        }
        cur = cur->next;
    }

    return 0;
}

/* ============================================================ */
/* Label Alias / Canonicalization                              */
/* ============================================================ */

typedef struct LabelAlias
{
    char *from;
    char *to;
    struct LabelAlias *next;
} LabelAlias;





/* ============================================================ */
/* SPIM Instruction Emission                                    */
/* ============================================================ */

static void fprint_reg(FILE *out, const char *name)
{
    if (!name)
        return;
    if (name[0] == '$')
        fprintf(out, "%s", name);
    else
        fprintf(out, "$%s", name);
}

static void fprint_operand(FILE *out, const char *value)
{
    if (is_register_name(value))
        fprint_reg(out, value);
    else
        fprintf(out, "%s", value ? value : "");
}

static const char *strip_trailing_proc_suffix(const char *name)
{
    static char buf[256];
    size_t len;

    if (!name)
        return "";

    len = strlen(name);
    if (len > 0 && name[len - 1] == '_')
    {
        size_t n = len - 1;
        if (n >= sizeof(buf))
            n = sizeof(buf) - 1;
        memcpy(buf, name, n);
        buf[n] = '\0';
        return buf;
    }

    return name;
}

static void emit_spim_op2(FILE *out, const Rtl_Op2 *ins, const Procedure_Ast *proc)
{
    int local_off = 0;

    if (strcmp(ins->op, "load") == 0)
    {
        fprintf(out, "    lw ");
        fprint_reg(out, ins->dst);
        if (local_offset_for_symbol(proc, ins->src, &local_off))
            fprintf(out, ", %d($fp)\n", local_off);
        else
            fprintf(out, ", %s\n", ins->src);
    }
    else if (strcmp(ins->op, "load.d") == 0)
    {
        fprintf(out, "    l.d ");
        fprint_reg(out, ins->dst);
        if (local_offset_for_symbol(proc, ins->src, &local_off))
            fprintf(out, ", %d($fp)\n", local_off);
        else
            fprintf(out, ", %s\n", ins->src);
    }
    else if (strcmp(ins->op, "store") == 0)
    {
        fprintf(out, "    sw ");
        fprint_operand(out, ins->src);
        if (local_offset_for_symbol(proc, ins->dst, &local_off))
            fprintf(out, ", %d($fp)\n", local_off);
        else
            fprintf(out, ", %s\n", ins->dst);
    }
    else if (strcmp(ins->op, "store.d") == 0)
    {
        fprintf(out, "    s.d ");
        fprint_operand(out, ins->src);
        if (local_offset_for_symbol(proc, ins->dst, &local_off))
            fprintf(out, ", %d($fp)\n", local_off);
        else
            fprintf(out, ", %s\n", ins->dst);
    }
    else if (strcmp(ins->op, "iLoad") == 0)
    {
        fprintf(out, "    li ");
        fprint_reg(out, ins->dst);
        fprintf(out, ", %s\n", ins->src);
    }
    else if (strcmp(ins->op, "iLoad.d") == 0)
    {
        fprintf(out, "    li.d ");
        fprint_reg(out, ins->dst);
        fprintf(out, ", %s\n", ins->src);
    }
    else if (strcmp(ins->op, "move") == 0)
    {
        fprintf(out, "    move ");
        fprint_reg(out, ins->dst);
        fprintf(out, ", ");
        fprint_operand(out, ins->src);
        fprintf(out, "\n");
    }
    else if (strcmp(ins->op, "move.d") == 0)
    {
        fprintf(out, "    mov.d ");
        fprint_reg(out, ins->dst);
        fprintf(out, ", ");
        fprint_operand(out, ins->src);
        fprintf(out, "\n");
    }
    else if (strcmp(ins->op, "uminus") == 0)
    {
        fprintf(out, "    sub ");
        fprint_reg(out, ins->dst);
        fprintf(out, ", $zero, ");
        fprint_operand(out, ins->src);
        fprintf(out, "\n");
    }
    else if (strcmp(ins->op, "uminus.d") == 0)
    {
        fprintf(out, "    neg.d ");
        fprint_reg(out, ins->dst);
        fprintf(out, ", ");
        fprint_operand(out, ins->src);
        fprintf(out, "\n");
    }
    else if (strcmp(ins->op, "not") == 0)
    {
        fprintf(out, "    xori ");
        fprint_reg(out, ins->dst);
        fprintf(out, ", ");
        fprint_operand(out, ins->src);
        fprintf(out, ", 1\n");
    }
    else if (strcmp(ins->op, "load_addr") == 0)
    {
        fprintf(out, "    la ");
        fprint_reg(out, ins->dst);
        fprintf(out, ", %s\n", ins->src);
    }
}

static void emit_spim_op3(FILE *out, const Rtl_Op3 *ins)
{
    fprintf(out, "    %s ", ins->op ? ins->op : "");
    fprint_operand(out, ins->dst);
    fprintf(out, ", ");
    fprint_operand(out, ins->src1);
    fprintf(out, ", ");
    fprint_operand(out, ins->src2);
    fprintf(out, "\n");
}

static void emit_spim_op2_comma(FILE *out, const Rtl_Op2Comma *ins)
{
    const char *mapped = ins->op;

    if (strcmp(ins->op, "slt.d") == 0)
        mapped = "c.lt.d";
    else if (strcmp(ins->op, "sle.d") == 0)
        mapped = "c.le.d";
    else if (strcmp(ins->op, "seq.d") == 0)
        mapped = "c.eq.d";

    fprintf(out, "    %s ", mapped ? mapped : "");
    fprint_operand(out, ins->src1);
    fprintf(out, ", ");
    fprint_operand(out, ins->src2);
    fprintf(out, "\n");
}

static int data_type_size_bytes(Data_Type type)
{
    switch (type)
    {
    case VOID_TYPE:
        return 0;
    default:
        return 4;
    }
}

static const Procedure_Ast *find_procedure_ast(const Ast *root, const char *proc_name)
{
    const Program_Ast *prog;
    Ast_List *cur;

    if (!root || root->kind != AST_PROGRAM || !proc_name)
        return NULL;

    prog = (const Program_Ast *)root;
    for (cur = prog->procedures; cur; cur = cur->next)
    {
        if (cur->stmt && cur->stmt->kind == AST_PROCEDURE)
        {
            const Procedure_Ast *proc = (const Procedure_Ast *)cur->stmt;
            if (proc->name && strcmp(proc->name, proc_name) == 0)
                return proc;
        }
    }

    return NULL;
}

static int compute_frame_size_bytes(const Ast *root, const char *proc_name)
{
    const Procedure_Ast *proc = find_procedure_ast(root, proc_name);
    int locals_size = 0;
    int return_size = 0;

    if (proc)
    {
        locals_size = proc->local_var_bytes;
        return_size = data_type_size_bytes(proc->return_type);
    }

    return locals_size + 8 + return_size;
}

static void emit_spim_seq(FILE *out, const Rtl_Seq *seq, const char *proc_name, int frame_size, const Procedure_Ast *proc, const Ast *root)
{
    const Rtl *cur;
    const char *name = (proc_name && *proc_name) ? proc_name : "anon";

    fprintf(out, "\n.text\n");
    fprintf(out, ".globl %s\n", name);
    fprintf(out, "%s:\n", name);
    fprintf(out, "    # Prologue begins\n");
    fprintf(out, "    sw $ra, 0($sp)\n");
    fprintf(out, "    sw $fp, -4($sp)\n");
    fprintf(out, "    sub $fp, $sp, 4\n");
    fprintf(out, "    sub $sp, $sp, %d\n", frame_size);
    fprintf(out, "    # Prologue ends\n\n");

    for (cur = seq ? seq->head : NULL; cur; cur = cur->next)
    {
        switch (cur->kind)
        {
        case RTL_LABEL:
        {
            const Rtl_Label *ins = (const Rtl_Label *)cur;
            if (ins->label)
                fprintf(out, "%s:\n", ins->label);
            break;
        }
        case RTL_GOTO:
        {
            const Rtl_Goto *ins = (const Rtl_Goto *)cur;
            fprintf(out, "    j %s\n", ins->label ? ins->label : "");
            break;
        }
        case RTL_BGTZ:
        {
            const Rtl_Bgtz *ins = (const Rtl_Bgtz *)cur;
            fprintf(out, "    bgtz ");
            fprint_operand(out, ins->reg);
            fprintf(out, ", %s\n", ins->label ? ins->label : "");
            break;
        }
        case RTL_CALL:
        {
            const Rtl_Call *ins = (const Rtl_Call *)cur;
            fprintf(out, "    jal %s\n", strip_trailing_proc_suffix(ins->name));
            break;
        }
        case RTL_OP0:
        {
            const Rtl_Op0 *ins = (const Rtl_Op0 *)cur;
            if (strcmp(ins->op, "write") == 0 || strcmp(ins->op, "read") == 0)
                fprintf(out, "    syscall\n");
            else if (strcmp(ins->op, "pop") == 0)
                fprintf(out, "    add $sp, $sp, 4\n");
            else if (strcmp(ins->op, "return") == 0)
                fprintf(out, "    j epilogue_%s\n", name);
            break;
        }
        case RTL_OP1:
        {
            const Rtl_Op1 *ins = (const Rtl_Op1 *)cur;
            if (strcmp(ins->op, "push") == 0)
            {
                fprintf(out, "    sub $sp, $sp, 4\n");
                fprintf(out, "    sw ");
                fprint_operand(out, ins->src);
                fprintf(out, ", 0($sp)\n");
            }
            else if (strcmp(ins->op, "return") == 0)
            {
                if (ins->src && *ins->src && strcmp(ins->src, "v1") != 0 && strcmp(ins->src, "f0") != 0)
                {
                    fprintf(out, "    move $v1, ");
                    fprint_operand(out, ins->src);
                    fprintf(out, "\n");
                }
                fprintf(out, "    j epilogue_%s\n", name);
            }
            break;
        }
        case RTL_OP2:
            emit_spim_op2(out, (const Rtl_Op2 *)cur, proc);
            break;
        case RTL_OP3:
            emit_spim_op3(out, (const Rtl_Op3 *)cur);
            break;
        case RTL_OP2_COMMA:
            emit_spim_op2_comma(out, (const Rtl_Op2Comma *)cur);
            break;
        default:
            break;
        }
    }

    fprintf(out, "\nepilogue_%s:\n", name);
    fprintf(out, "    add $sp, $sp, %d\n", frame_size);
    fprintf(out, "    lw $fp, -4($sp)\n");
    fprintf(out, "    lw $ra, 0($sp)\n");
    fprintf(out, "    jr $ra\n");
}

/* ============================================================ */
/* Public API                                                   */
/* ============================================================ */

void spim_generate(Ast *root, FILE *out)
{
    Rtl *cur;
    Asm_Symbol *symbols = NULL;
    Asm_Symbol *sym;
    Proc_Rtl_Block *blocks = NULL;
    int block_count = 0;

    if (!root || !out)
        return;

    rtl_reset_counters();
    blocks = rtl_collect_proc_blocks(root, &block_count);
    if (!blocks || block_count == 0)
        return;

    for (int i = 0; i < block_count; i++)
        collect_data_symbols_from_seq(blocks[i].seq, &symbols);

    fputs(".data\n", out);
    for (sym = symbols; sym; sym = sym->next)
    {
        Data_Type t = lookup_symbol_type_safe(sym->name);
        if (t == FLOAT_TYPE)
            fprintf(out, "%s: .float 0.0\n", sym->name);
        else
            fprintf(out, "%s: .word 0\n", sym->name);
    }

    for (int i = 0; i < block_count; i++)
    {
        const Procedure_Ast *proc = find_procedure_ast(root, blocks[i].name);
        int frame_size = compute_frame_size_bytes(root, blocks[i].name);
        emit_spim_seq(out, blocks[i].seq, blocks[i].name, frame_size, proc, root);
    }

    free_asm_symbols(symbols);
    rtl_free_proc_blocks(blocks, block_count);
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

void spim_reset_counters(void)
{
    rtl_reset_counters();
}
