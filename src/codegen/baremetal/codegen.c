#include <stdio.h>   // Standart C I/O: fopen, fwrite, fclose
#include <stdlib.h>
#include <string.h>
#include "codegen.h"
#include "error.h"

// Unix/Linux'a özgü <unistd.h> çağrıları burada kullanılmaz.

// --- RISC-V Kodlama Sabitleri (RV32I) ---
// (Önceki UNIX kodunda tanımlanan sabitler burada da kullanılacaktır.)
#define OP_R_TYPE   0x33
#define OP_I_TYPE   0x13
#define OP_S_TYPE   0x23
#define OP_B_TYPE   0x63
#define FUNC3_ADD_SUB 0x0 
#define FUNC7_ADD     0x00
#define FUNC7_SUB     0x20

/**
 * @brief Tek bir Instruction yapısını 32-bit RISC-V makine koduna dönüştürür.
 * * UNIX versiyonuyla aynı mantığı kullanır, bu sayede ikilik çıktı aynı kalır, 
 * sadece dosya yazma yöntemi değişir.
 * * @param inst: Dönüştürülecek Instruction yapısı.
 * @param current_address: Talimatın programdaki adresi.
 * @param sym_table: Etiket adreslerini çözümlemek için.
 * @return MachineCodeInstruction: 32-bit makine kodu.
 */
static MachineCodeInstruction encode_riscv_instruction(Instruction *inst, int current_address, SymbolTable *sym_table) {
    MachineCodeInstruction encoding = 0;
    
    // Basitlik için sadece ADD/SUB/ADDI/LW/SW ve HALT dönüşümlerini uygulayalım.
    // Detaylı RISC-V kodlama (B-Type offset hesaplaması) ileride eklenebilir.

    switch (inst->type) {
        case I_ADDI:
        case I_LW:
            // I-Type
            encoding |= (uint32_t)inst->immediate << 20;
            encoding |= (uint32_t)inst->rs1 << 15;
            encoding |= (inst->type == I_ADDI ? (0x0 << 12) : (0x2 << 12)); 
            encoding |= (uint32_t)inst->rd << 7;
            encoding |= OP_I_TYPE;
            break;

        case I_ADD:
        case I_SUB:
            // R-Type
            encoding |= (uint32_t)inst->rs2 << 20;
            encoding |= (uint32_t)inst->rs1 << 15;
            encoding |= FUNC3_ADD_SUB << 12;
            encoding |= (uint32_t)inst->rd << 7;
            encoding |= OP_R_TYPE;
            encoding |= (inst->type == I_SUB ? FUNC7_SUB : FUNC7_ADD) << 25;
            break;
            
        case I_SW:
            // S-Type
            uint32_t imm_11_5 = (inst->immediate >> 5) & 0x7F;
            uint32_t imm_4_0 = inst->immediate & 0x1F;        
            
            encoding |= imm_11_5 << 25;
            encoding |= (uint32_t)inst->rs2 << 20; 
            encoding |= (uint32_t)inst->rs1 << 15; 
            encoding |= (0x2 << 12);               
            encoding |= imm_4_0 << 7;
            encoding |= OP_S_TYPE;
            break;

        case I_A_HALT:
            // EBREAK talimatı
            encoding = 0x100073; 
            break;
            
        case I_BEQ: // B-Type (Basitlik için burada tam kodlanmayacaktır)
        case I_BNE:
            // ILERI: Atlama ofset çözümü ve B-tipi kodlama buraya eklenecektir.
            // Şimdilik NOP olarak bırakıyoruz.
            encoding = 0x13; // ADDI x0, x0, 0 (NOP)
            break;

        default:
            report_error(0, 0, "Bilinmeyen veya desteklenmeyen Bare-Metal RISC-V talimat tipi.");
            return 0;
    }

    return encoding;
}


// --- Ana Kod Üretim İşlevi (Dosya I/O için Standart C kullanır) ---

bool codegen_write_baremetal_binary(CodeBuffer *buffer, const char *output_filename, SymbolTable *sym_table) {
    if (buffer == NULL || buffer->count == 0) {
        fprintf(stderr, "HATA: Üretilecek RISC-V talimatı bulunamadı.\n");
        return false;
    }
    
    // 1. Dosyayı ikilik yazma modunda açma
    FILE *fp = fopen(output_filename, "wb");
    if (fp == NULL) {
        perror("Bare-Metal çıktı dosyası açılamadı");
        return false;
    }
    
    printf("RISC-V Ham Makine Kodu \"%s\" dosyasına yazılıyor (Bare-Metal için)...\n", output_filename);
    
    // 2. Her talimatı ikilik koda çevir ve dosyaya yaz
    for (size_t i = 0; i < buffer->count; i++) {
        Instruction *inst = &buffer->instructions[i];
        
        int current_address = (int)i * sizeof(MachineCodeInstruction); 
        
        MachineCodeInstruction encoded = encode_riscv_instruction(inst, current_address, sym_table);
        
        // fwrite: Bellekten dosyaya yazma
        size_t written = fwrite(&encoded, sizeof(MachineCodeInstruction), 1, fp);
        
        if (written != 1) {
            perror("Dosyaya yazma hatası");
            fclose(fp);
            return false;
        }
    }
    
    // 3. Dosyayı kapat
    if (fclose(fp) == EOF) {
        perror("Dosya kapatma hatası");
        return false;
    }

    printf("Bare-Metal kod üretimi tamamlandı. Dosya boyutu: %zu bayt.\n", buffer->count * sizeof(MachineCodeInstruction));
    return true;
}
