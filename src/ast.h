#ifndef BESSAMBLY_AST_H
#define BESSAMBLY_AST_H

#include <stddef.h>
#include <stdbool.h>
#include "utils.h" // MAX_LABEL_LENGTH için

// --- İleri Bildirimler ---
typedef struct AST_Node AST_Node;
typedef struct AST_Program AST_Program;

// --- 1. Temel Yapılar: Operandlar ve İfadeler ---

// Operant Tipi: İşlemlerde kullanılan değerin kaynağını/hedefini belirtir.
typedef enum {
    OPR_TYPE_REGISTER,     // Kayıt (örn: A, B, SUM)
    OPR_TYPE_MEMORY_ADDR,  // Bellek Adresi (örn: MEM[0x10])
    OPR_TYPE_IMMEDIATE_INT // Anlık Tamsayı Sabiti (örn: 10, 0xFF)
} OperandType;

// Bellek Erişim Tipi
typedef enum {
    MEM_ACCESS_NONE = 0, // Bellek erişimi yok (kayıt veya sabit)
    MEM_ACCESS_FIXED,    // Sabit adres (örn: MEM[0x100])
    // Not: Bessambly'de sadece sabit adres desteklenir.
} MemAccessType;


// Tek bir operantı temsil eder (A, 10, MEM[0x50])
typedef struct {
    OperandType type;
    char name[MAX_LABEL_LENGTH]; // Kayıt adı veya Etiket adı (eğer REGISTER ise)
    
    // Değerler
    long long int value; // Eğer OPR_TYPE_IMMEDIATE_INT ise
    
    // Bellek erişim detayları
    MemAccessType mem_type;
    long long int mem_address; // Eğer MEMORY_ADDR ise
} AST_Operand;


// İfade Tipi (C = A + B'deki A+B kısmı)
typedef enum {
    EXPR_TYPE_OPERAND,  // Sadece tek bir operant (örn: A veya 10)
    EXPR_TYPE_BINARY    // İkili işlem (örn: A + B)
} ExpressionType;

// İkili Operatör Tipi (A + B'deki '+')
typedef enum {
    OP_BIN_ADD, // +
    OP_BIN_SUB, // -
    OP_BIN_MUL, // *
    OP_BIN_DIV, // /
    OP_BIN_AND, // &
    OP_BIN_OR   // |
} BinaryOperatorType;

// İfade (Expr) yapısı
typedef struct AST_Expr {
    ExpressionType type;
    
    union {
        AST_Operand operand; // EXPR_TYPE_OPERAND ise
        
        struct {
            BinaryOperatorType op;
            AST_Operand left;
            AST_Operand right;
        } binary_op; // EXPR_TYPE_BINARY ise
    } data;
} AST_Expr;


// --- 2. Komut Tipleri (Statements) ---

// Koşul Operatörü Tipi (if A > B'deki >)
typedef enum {
    OP_COND_GT,  // >
    OP_COND_LT,  // <
    OP_COND_EQ,  // ==
    OP_COND_NE,  // !=
    OP_COND_GE,  // >=
    OP_COND_LE   // <=
} ConditionOperatorType;


// Temel Komut Tipi: Bessambly'nin desteklediği tüm komut türlerini listeler
typedef enum {
    STMT_TYPE_LABEL_DEF,    // Etiket Tanımlama (LOOP_START:)
    STMT_TYPE_ASSIGNMENT,   // Atama İşlemi (C = A + B)
    STMT_TYPE_GOTO,         // Koşulsuz Atlama (goto END)
    STMT_TYPE_IF_GOTO       // Koşullu Atlama (if A > B goto LOOP)
} StatementType;


// Her bir komutu temsil eden ana AST Düğümü
struct AST_Node {
    StatementType type;
    int line_number; // Hata raporlama ve debug için satır numarası
    
    union {
        // STMT_TYPE_LABEL_DEF
        struct {
            char label_name[MAX_LABEL_LENGTH];
        } label_def;
        
        // STMT_TYPE_ASSIGNMENT
        struct {
            AST_Operand destination; // Atama hedefi (örn: C veya MEM[0x10])
            AST_Expr expression;     // Atanan ifade (örn: A + B veya sadece 10)
        } assignment;
        
        // STMT_TYPE_GOTO
        struct {
            char target_label[MAX_LABEL_LENGTH];
        } goto_stmt;
        
        // STMT_TYPE_IF_GOTO
        struct {
            AST_Operand left;
            ConditionOperatorType op;
            AST_Operand right;
            char target_label[MAX_LABEL_LENGTH];
        } if_goto_stmt;
    } data;
    
    AST_Node *next; // Program akışındaki bir sonraki komuta işaret eder
};


// --- 3. Program Yapısı ---

// Tüm programı temsil eden kök düğüm
struct AST_Program {
    AST_Node *first_statement; // Bağlı listenin başı
};


// --- AST İşlev Prototipleri ---

/**
 * @brief Yeni bir AST Program yapısı oluşturur ve belleği tahsis eder.
 */
AST_Program *ast_program_create();

/**
 * @brief Bir komut düğümü (AST_Node) oluşturur ve belleği tahsis eder.
 */
AST_Node *ast_node_create(StatementType type, int line_number);

/**
 * @brief Tüm AST yapısını ve bağlı düğümleri serbest bırakır.
 */
void ast_program_free(AST_Program *program);

/**
 * @brief (DEBUG) AST'yi konsola yazdırmak için yardımcı işlev.
 */
void ast_print(AST_Program *program);


#endif // BESSAMBLY_AST_H