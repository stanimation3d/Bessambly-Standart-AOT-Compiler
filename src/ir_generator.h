#ifndef BESSAMBLY_IR_GENERATOR_H
#define BESSAMBLY_IR_GENERATOR_H

#include <stdbool.h>
#include <stdint.h>
#include "ast.h"
#include "symbol_table.h"

// --- RISC-V Temel Yapıları ---

// RISC-V Kayıtları için sembolik isimler (x1-x31)
typedef enum {
    // T-Kayitlar (Temporaries) - Aritmetik icin kullanilacak
    R_T0 = 5, R_T1, R_T2,
    // S-Kayitlar (Saved) - Bessambly Kayitlarini tutmak icin kullanilabilir
    R_S0 = 8, R_S1, R_S2, R_S3, 
    // SP: Yigin Isaretcisi (Stack Pointer), GP: Global Pointer
    R_SP = 2, R_GP = 3,
    // Zero Register
    R_ZERO = 0,
    // Kalan T/S kayitlarinin kullanilabilecegi varsayilmistir.
    R_COUNT = 32
} Register;

// RISC-V Talimat Tipleri (Sadece Bessambly'nin ihtiyac duyduklari)
typedef enum {
    // Aritmetik/Mantık (R-Type ve I-Type)
    I_ADDI,    // Add Immediate: rd = rs1 + imm
    I_SUB,     // Subtract: rd = rs1 - rs2
    I_ADD,     // Add: rd = rs1 + rs2
    I_ANDI,    // And Immediate: rd = rs1 & imm
    I_ORI,     // Or Immediate: rd = rs1 | imm
    
    // Yükleme/Depolama (I-Type ve S-Type)
    I_LW,      // Load Word: rd = MEM[rs1 + offset]
    I_SW,      // Store Word: MEM[rs1 + offset] = rs2
    
    // Atlama/Dallanma (B-Type)
    I_BEQ,     // Branch Equal: if (rs1 == rs2) branch
    I_BNE,     // Branch Not Equal: if (rs1 != rs2) branch
    I_BLT,     // Branch Less Than
    I_BGE,     // Branch Greater or Equal
    I_JAL,     // Jump and Link (Şimdilik GOTO için kullanılmayabilir, B tipi yeterli)
    I_JALR,    // Jump and Link Register
    
    // Özel
    I_LUI,     // Load Upper Immediate
    I_A_HALT   // Programin sonu icin (Gercek RISC-V komutu degil, sanal makineyi durdurur)
} InstructionType;

// Tek bir RISC-V talimatını temsil eden yapı
typedef struct {
    InstructionType type;
    Register rd;       // Hedef kayıt (Destination)
    Register rs1;      // Kaynak kayıt 1
    Register rs2;      // Kaynak kayıt 2 (R-Type)
    int32_t immediate; // Sabit değer veya offset/adres (I/S/B-Type)
    char label_name[MAX_LABEL_LENGTH]; // Atlama talimatları için hedef etiket adı
} Instruction;

// Üretilen tüm talimat dizisini tutan yapı
typedef struct {
    Instruction *instructions;
    size_t count;
    size_t capacity;
} CodeBuffer;

// --- Kod Üretimi Ana İşlevleri ---

/**
 * @brief Kod Üreteci arabelleğini başlatır.
 */
CodeBuffer *code_buffer_init();

/**
 * @brief AST'yi dolaşır ve RISC-V talimatlarını CodeBuffer'a üretir.
 * @param program: AST'nin kök düğümü.
 * @param sym_table: Sembol Tablosu (Etiket adresleri için gereklidir).
 * @return CodeBuffer*: Üretilen talimatları içeren arabellek.
 */
CodeBuffer *generate_riscv_code(AST_Program *program, SymbolTable *sym_table);

/**
 * @brief CodeBuffer için ayrılan belleği serbest bırakır.
 */
void code_buffer_free(CodeBuffer *buffer);

/**
 * @brief (DEBUG) Üretilen talimatları konsola yazdırır.
 */
void print_riscv_code(CodeBuffer *buffer);

#endif // BESSAMBLY_IR_GENERATOR_H