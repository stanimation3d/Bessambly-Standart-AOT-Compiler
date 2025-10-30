#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ir_generator.h"
#include "error.h"
#include "utils.h"

#define INITIAL_CAPACITY 64 // Kod arabelleği başlangıç boyutu

// --- Yardımcı İşlevler: Kayıt Ataması ve Buffer Yönetimi ---

// Bessambly'de sadece basit tek harfli kayıtlar (A, B, C...) olduğunu varsayarsak,
// bunları RISC-V S-Type (Saved) kayıtlara eşleştirelim.
// Örneğin: A -> R_S1, B -> R_S2, C -> R_S3
static Register get_register_from_name(const char *name) {
    if (name[0] >= 'A' && name[0] <= 'Z' && name[1] == '\0') {
        // Basit tek harfli kayıt eşleştirmesi: A->S1, B->S2, C->S3...
        // S0 genellikle program akışı için ayrılır. S1'den başlatalım.
        return (Register)(R_S1 + (name[0] - 'A')); 
    }
    // Daha karmaşık kayıt isimleri için şimdilik R_T0 döndürüyoruz (Hata durumu veya geçici kullanım)
    return R_T0; 
}


static void buffer_append_instruction(CodeBuffer *buffer, Instruction inst) {
    if (buffer->count >= buffer->capacity) {
        buffer->capacity *= 2;
        buffer->instructions = (Instruction *)safe_realloc(
            buffer->instructions, 
            buffer->capacity * sizeof(Instruction)
        );
    }
    buffer->instructions[buffer->count++] = inst;
}


// --- AST Düğümlerinden RISC-V Talimatları Üretme ---

/**
 * @brief Tek bir AST operantını RISC-V talimatlarına çevirir. 
 * Sonucu hedef kayda (dest_reg) yükler.
 * @param opr: İşlenecek AST operantı.
 * @param dest_reg: Operantın değerinin yükleneceği kayıt.
 * @param buffer: Kod arabelleği.
 */
static void generate_operand_load(AST_Operand *opr, Register dest_reg, CodeBuffer *buffer) {
    if (opr->type == OPR_TYPE_REGISTER) {
        Register src_reg = get_register_from_name(opr->name);
        // Kaydı kopyala (Dest = Src + 0)
        buffer_append_instruction(buffer, (Instruction){I_ADD, dest_reg, src_reg, R_ZERO, 0});
        
    } else if (opr->type == OPR_TYPE_IMMEDIATE_INT) {
        // Sabit değeri yükle (Dest = ZERO + Sabit)
        // ADDI rd, x0, imm (imm 12-bit ile sinirli)
        // Simdilik 12-bit sinirlarini goz ardi ediyoruz, ILERI: LUI/ADDI kullanilmalidir
        buffer_append_instruction(buffer, (Instruction){I_ADDI, dest_reg, R_ZERO, 0, (int32_t)opr->value});
        
    } else if (opr->type == OPR_TYPE_MEMORY_ADDR) {
        // Bellekten yükle (LW rd, offset(rs1))
        // Burada rs1 olarak R_ZERO kullanilmistir. Bellek adresi doğrudan offset olarak kullanilir.
        // ILERI: Global pointer (R_GP) veya stack pointer (R_SP) ile adresleme kullanilmalidir.
        buffer_append_instruction(buffer, (Instruction){I_LW, dest_reg, R_ZERO, 0, (int32_t)opr->mem_address});
    }
}


/**
 * @brief İkili ifadeyi (A+B) RISC-V talimatlarına çevirir.
 * Sonucu rs1 kaydına atar.
 */
static void generate_binary_expr(AST_Expr *expr, Register rs1, CodeBuffer *buffer) {
    // 1. Sol operantı geçici T0 kaydına yükle
    generate_operand_load(&expr->data.binary_op.left, R_T0, buffer);
    
    // 2. Sağ operantı geçici T1 kaydına yükle
    generate_operand_load(&expr->data.binary_op.right, R_T1, buffer);
    
    // 3. İşlemi yap ve sonucu rs1'e kaydet (rs1 = T0 OP T1)
    InstructionType inst_type = I_ADD; 
    switch (expr->data.binary_op.op) {
        case OP_BIN_ADD: inst_type = I_ADD; break;
        case OP_BIN_SUB: inst_type = I_SUB; break;
        // Basitlik icin sadece ADD/SUB kullanilmistir. ILERI: MUL/DIV/AND/OR eklenmelidir.
        default: inst_type = I_ADD; break; 
    }
    
    buffer_append_instruction(buffer, (Instruction){inst_type, rs1, R_T0, R_T1, 0});
}


static void generate_assignment(AST_Node *node, CodeBuffer *buffer) {
    // Hedef kayıt (Destination Register)
    Register dest_reg;
    bool is_mem_store = false;
    
    if (node->data.assignment.destination.type == OPR_TYPE_REGISTER) {
        dest_reg = get_register_from_name(node->data.assignment.destination.name);
    } else if (node->data.assignment.destination.type == OPR_TYPE_MEMORY_ADDR) {
        // Belleğe yazılacaksa, sonuç geçici bir kayda (R_T0) hesaplanmalı
        dest_reg = R_T0; 
        is_mem_store = true;
    } else {
        // Bu durum anlambilim analizinde yakalanmış olmalı (Sabit hedefe atama)
        return; 
    }

    // İfadeyi hesapla
    if (node->data.assignment.expression.type == EXPR_TYPE_OPERAND) {
        // Sadece tek bir operant atanıyor (A = B veya A = 10)
        generate_operand_load(&node->data.assignment.expression.data.operand, dest_reg, buffer);
    } else {
        // İkili ifade atanıyor (C = A + B)
        generate_binary_expr(&node->data.assignment.expression, dest_reg, buffer);
    }
    
    // Bellek Depolama İşlemi (MEM[addr] = expr)
    if (is_mem_store) {
        // SW rs2, offset(rs1)
        // rs2: Hesaplanan değer (R_T0)
        // offset: Bellek adresi
        buffer_append_instruction(buffer, 
            (Instruction){
                I_SW, 
                R_ZERO, // rd kullanılmaz
                R_ZERO, // rs1 (Temel adres: 0x0)
                dest_reg, // rs2 (Kaynak Kayıt - Hesaplanan değer)
                (int32_t)node->data.assignment.destination.mem_address
            }
        );
    }
}

// ILERI: GOTO ve IF_GOTO talimat uretimi eklenmelidir.

// --- Genel İşlev Uygulamaları ---

CodeBuffer *code_buffer_init() {
    CodeBuffer *buffer = (CodeBuffer *)safe_malloc(sizeof(CodeBuffer));
    buffer->instructions = (Instruction *)safe_malloc(INITIAL_CAPACITY * sizeof(Instruction));
    buffer->count = 0;
    buffer->capacity = INITIAL_CAPACITY;
    return buffer;
}

CodeBuffer *generate_riscv_code(AST_Program *program, SymbolTable *sym_table) {
    printf("RISC-V Kod Üretimi Başladı...\n");
    CodeBuffer *buffer = code_buffer_init();
    
    // AST'yi dolaş
    AST_Node *current = program->first_statement;
    while (current != NULL) {
        switch (current->type) {
            case STMT_TYPE_LABEL_DEF:
                // Etiketler sadece Sembol Tablosunda adreslenir, makine kodu üretmez.
                break;
            case STMT_TYPE_ASSIGNMENT:
                generate_assignment(current, buffer);
                break;
            case STMT_TYPE_GOTO:
                // ILERI: GOTO komutu (JAL veya B-Type) üretilmelidir.
                break;
            case STMT_TYPE_IF_GOTO:
                // ILERI: IF_GOTO komutu (B-Type) üretilmelidir.
                break;
        }
        current = current->next;
    }
    
    // Program sonuna HALT komutu ekle
    buffer_append_instruction(buffer, (Instruction){I_A_HALT, R_ZERO, R_ZERO, R_ZERO, 0});
    
    printf("RISC-V Kod Üretimi Başarılı. Toplam %zu talimat üretildi.\n", buffer->count);
    return buffer;
}

void code_buffer_free(CodeBuffer *buffer) {
    if (buffer != NULL) {
        free(buffer->instructions);
        free(buffer);
    }
}

// --- DEBUG Fonksiyonu ---
static const char *get_reg_name(Register r) {
    if (r == R_ZERO) return "x0/zero";
    if (r == R_SP) return "x2/sp";
    if (r >= R_T0 && r <= R_T2) {
        static char name[5]; sprintf(name, "t%d", r - R_T0); return name;
    }
    if (r >= R_S0 && r <= R_S3) {
        static char name[5]; sprintf(name, "s%d", r - R_S0); return name;
    }
    return "x??";
}

void print_riscv_code(CodeBuffer *buffer) {
    printf("\n--- RISC-V Talimat Çıktısı ---\n");
    for (size_t i = 0; i < buffer->count; i++) {
        Instruction *inst = &buffer->instructions[i];
        printf("0x%04zX: ", i * 4); // Varsayımsal 4 baytlık talimat adresi
        
        switch (inst->type) {
            case I_ADDI: 
                printf("ADDI %s, %s, %d\n", get_reg_name(inst->rd), get_reg_name(inst->rs1), inst->immediate); 
                break;
            case I_ADD: 
                printf("ADD %s, %s, %s\n", get_reg_name(inst->rd), get_reg_name(inst->rs1), get_reg_name(inst->rs2)); 
                break;
            case I_SUB: 
                printf("SUB %s, %s, %s\n", get_reg_name(inst->rd), get_reg_name(inst->rs1), get_reg_name(inst->rs2)); 
                break;
            case I_LW: 
                printf("LW %s, %d(%s)\n", get_reg_name(inst->rd), inst->immediate, get_reg_name(inst->rs1)); 
                break;
            case I_SW: 
                printf("SW %s, %d(%s)\n", get_reg_name(inst->rs2), inst->immediate, get_reg_name(inst->rs1)); 
                break;
            case I_A_HALT: 
                printf("HALT (Sanal Komut)\n"); 
                break;
            // ... diğer komutlar
            default: printf("UNKNOWN_INST\n");
        }
    }
    printf("-------------------------------\n");
}