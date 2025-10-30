#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "utils.h" // safe_malloc/safe_realloc için

// --- Yardımcı İşlevler ---

// Hata ayıklama için operatör/tip isimlerini dizeye çevirme
static const char *get_stmt_type_name(StatementType type) {
    switch (type) {
        case STMT_TYPE_LABEL_DEF:    return "LABEL_DEF";
        case STMT_TYPE_ASSIGNMENT:   return "ASSIGNMENT";
        case STMT_TYPE_GOTO:         return "GOTO";
        case STMT_TYPE_IF_GOTO:      return "IF_GOTO";
        default:                     return "UNKNOWN";
    }
}

static void ast_operand_print(AST_Operand *opr) {
    switch (opr->type) {
        case OPR_TYPE_REGISTER:
            printf("%s", opr->name);
            break;
        case OPR_TYPE_MEMORY_ADDR:
            printf("MEM[0x%llX]", opr->mem_address);
            break;
        case OPR_TYPE_IMMEDIATE_INT:
            printf("%lld", opr->value);
            break;
    }
}

static void ast_expression_print(AST_Expr *expr) {
    if (expr->type == EXPR_TYPE_OPERAND) {
        ast_operand_print(&expr->data.operand);
    } else if (expr->type == EXPR_TYPE_BINARY) {
        printf("(");
        ast_operand_print(&expr->data.binary_op.left);
        printf(" %c ", 
            expr->data.binary_op.op == OP_BIN_ADD ? '+' :
            expr->data.binary_op.op == OP_BIN_SUB ? '-' :
            expr->data.binary_op.op == OP_BIN_MUL ? '*' :
            expr->data.binary_op.op == OP_BIN_DIV ? '/' :
            expr->data.binary_op.op == OP_BIN_AND ? '&' :
            '|'); // OP_BIN_OR
        ast_operand_print(&expr->data.binary_op.right);
        printf(")");
    }
}

// --- Program Yapısı İşlevleri ---

AST_Program *ast_program_create() {
    AST_Program *program = (AST_Program *)safe_malloc(sizeof(AST_Program));
    program->first_statement = NULL;
    return program;
}

AST_Node *ast_node_create(StatementType type, int line_number) {
    AST_Node *node = (AST_Node *)safe_malloc(sizeof(AST_Node));
    node->type = type;
    node->line_number = line_number;
    node->next = NULL;
    return node;
}

void ast_node_free(AST_Node *node) {
    if (node == NULL) return;
    // İç yapılar dinamik bellek tahsis etmediği için sadece düğümü serbest bırakıyoruz.
    free(node);
}

void ast_program_free(AST_Program *program) {
    if (program == NULL) return;
    
    AST_Node *current = program->first_statement;
    AST_Node *next;
    
    // Bağlı listenin tüm düğümlerini dolaşarak serbest bırak
    while (current != NULL) {
        next = current->next;
        ast_node_free(current);
        current = next;
    }
    
    free(program);
}

// --- Debug/Yazdırma İşlevi ---

void ast_print(AST_Program *program) {
    if (program == NULL || program->first_statement == NULL) {
        printf("AST Boş.\n");
        return;
    }

    AST_Node *current = program->first_statement;
    printf("\n--- Bessambly AST Başlangıcı ---\n");
    
    int stmt_index = 0;
    while (current != NULL) {
        printf("[%03d] (Satır %d) %s: ", stmt_index++, current->line_number, get_stmt_type_name(current->type));
        
        switch (current->type) {
            case STMT_TYPE_LABEL_DEF:
                printf("%s:\n", current->data.label_def.label_name);
                break;
                
            case STMT_TYPE_ASSIGNMENT:
                ast_operand_print(&current->data.assignment.destination);
                printf(" = ");
                ast_expression_print(&current->data.assignment.expression);
                printf("\n");
                break;
                
            case STMT_TYPE_GOTO:
                printf("goto %s\n", current->data.goto_stmt.target_label);
                break;
                
            case STMT_TYPE_IF_GOTO:
                printf("if ");
                ast_operand_print(&current->data.if_goto_stmt.left);
                // Operatör Sembolünü Yazdır
                printf(" [OP] "); 
                ast_operand_print(&current->data.if_goto_stmt.right);
                printf(" goto %s\n", current->data.if_goto_stmt.target_label);
                break;
        }
        
        current = current->next;
    }
    printf("--- Bessambly AST Sonu ---\n");
}