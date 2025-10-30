#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     // UNIX sistem çağrıları: write, close
#include <fcntl.h>      // Dosya açma: open, O_WRONLY, O_CREAT
#include <sys/stat.h>   // Dosya izinleri: S_IRUSR, S_IXUSR

#include "codegen.h"
#include "error.h"

// --- RISC-V Kodlama Sabitleri (RV32I) ---
// Kaynak: RISC-V Specification, Opcodes ve Fonksiyon alanları
#define OP_R_TYPE   0x33 // 0110011 (ADD, SUB, AND, OR)
#define OP_I_TYPE   0x13 // 0010011 (ADDI, ANDI, ORI)
#define OP_S_TYPE   0x23 // 0100011 (SW)
#define OP_B_TYPE   0x63 // 1100011 (BEQ, BNE, BLT, BGE)
#define FUNC3_ADD_SUB 0x0 // ADD/SUB için
#define FUNC7_ADD     0x00 // ADD için
#define FUNC7_SUB     0x20 // SUB için

// R-Tipi Talimat Formatı: [funct7 | rs2 | rs1 | funct3 | rd | opcode] (32-bit)
// I-Tipi Talimat Formatı: [imm[11:0] | rs1 | funct3 | rd | opcode] (32-bit)
// S-Tipi Talimat Formatı: [imm[11:5] | rs2 | rs1 | funct3 | imm[4:0] | opcode] (32-bit)


/**
 * @brief Tek bir Instruction yapısını 32-bit RISC-V makine koduna dönüştürür.
 * * @param inst: Dönüştürülecek Instruction yapısı.
 * @param current_address: Talimatın programdaki adresi (atlama ofsetleri için).
 * @param sym_table: Etiket adreslerini çözümlemek için.
 * @return MachineCodeInstruction: 32-bit makine kodu.
 */
static MachineCodeInstruction encode_riscv_instruction(Instruction *inst, int current_address, SymbolTable *sym_table) {
    MachineCodeInstruction encoding = 0;

    switch (inst->type) {
        // --- I-Type (ADDI, LW) ---
        case I_ADDI:
        case I_LW:
            // I-Tipi Format: [imm[11:0] | rs1 | funct3 | rd | opcode]
            encoding |= (uint32_t)inst->immediate << 20; // imm[11:0]
            encoding |= (uint32_t)inst->rs1 << 15;
            encoding |= (inst->type == I_ADDI ? (0x0 << 12) : (0x2 << 12)); // funct3 (ADDI: 0x0, LW: 0x2)
            encoding |= (uint32_t)inst->rd << 7;
            encoding |= OP_I_TYPE;
            break;

        // --- R-Type (ADD, SUB) ---
        case I_ADD:
        case I_SUB:
            // R-Tipi Format: [funct7 | rs2 | rs1 | funct3 | rd | opcode]
            encoding |= (uint32_t)inst->rs2 << 20;
            encoding |= (uint32_t)inst->rs1 << 15;
            encoding |= FUNC3_ADD_SUB << 12; // funct3 (0x0)
            encoding |= (uint32_t)inst->rd << 7;
            encoding |= OP_R_TYPE;
            
            if (inst->type == I_SUB) {
                encoding |= FUNC7_SUB << 25; // SUB için funct7 (0x20)
            } else {
                encoding |= FUNC7_ADD << 25; // ADD için funct7 (0x00)
            }
            break;
            
        // --- S-Type (SW) ---
        case I_SW:
            // S-Tipi Format: [imm[11:5] | rs2 | rs1 | funct3 | imm[4:0] | opcode]
            uint32_t imm_11_5 = (inst->immediate >> 5) & 0x7F; // imm[11:5]
            uint32_t imm_4_0 = inst->immediate & 0x1F;        // imm[4:0]
            
            encoding |= imm_11_5 << 25;
            encoding |= (uint32_t)inst->rs2 << 20; // rs2 (Kaynak Kayıt)
            encoding |= (uint32_t)inst->rs1 << 15; // rs1 (Temel Kayıt)
            encoding |= (0x2 << 12);               // funct3 (SW: 0x2)
            encoding |= imm_4_0 << 7;
            encoding |= OP_S_TYPE;
            break;

        // --- B-Type (GOTO/IF_GOTO - ILERI) ---
        case I_BEQ:
        case I_BNE:
            // Atlama adreslerini Sembol Tablosundan çözme (Çok basitleştirilmiş)
            // Gerçek ofset hesaplaması (target_addr - current_addr) burada yapılmalıdır.
            const Symbol *target_sym = symtable_lookup(sym_table, inst->label_name);
            if (target_sym == NULL) {
                // Bu durum Anlambilim Analizinde yakalanmış olmalıdır.
                report_error(ERR_SEMANTIC_UNKNOWN_LABEL, 0, "Atlama hedefini kod üretimi sırasında çözümlenemedi.");
                return 0;
            }
            // Hedef adres (satır numarası * 4)
            int target_address = (int)target_sym->details.address * sizeof(MachineCodeInstruction);
            int offset = target_address - current_address;
            
            // ILERI: Offset'i B-Tipi formatına göre bitlere bölme (imm[12|10:5|4:1|11])
            // Basitlik için bu aşama atlanmıştır.
            
            encoding |= OP_B_TYPE;
            break;

        // --- Sanal Komutlar ---
        case I_A_HALT:
            // HALT için özel bir talimat (Örn: C.EBREAK komutu kullanılabilir 0x9002)
            encoding = 0x100073; // EBREAK talimatı (000000000001 00000 000 00000 1110011)
            break;
            
        default:
            report_error(0, 0, "Bilinmeyen veya desteklenmeyen RISC-V talimat tipi.");
            return 0;
    }

    return encoding;
}


// --- Ana Kod Üretim İşlevi ---

bool codegen_write_binary(CodeBuffer *buffer, const char *output_filename, SymbolTable *sym_table) {
    if (buffer == NULL || buffer->count == 0) {
        fprintf(stderr, "HATA: Üretilecek RISC-V talimatı bulunamadı.\n");
        return false;
    }
    
    // 1. Dosyayı UNIX sistem çağrısıyla açma (O_WRONLY: Yazma, O_CREAT: Yoksa oluştur, O_TRUNC: İçeriği sil)
    // İzinler: Okuma (r) ve Çalıştırma (x) sadece kullanıcıya (S_IRUSR | S_IXUSR)
    int fd = open(output_filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IXUSR);
    if (fd < 0) {
        perror("Dosya açma hatası");
        return false;
    }
    
    printf("RISC-V Makine Kodu \"%s\" dosyasına yazılıyor...\n", output_filename);
    
    // 2. Her talimatı ikilik koda çevir ve dosyaya yaz
    for (size_t i = 0; i < buffer->count; i++) {
        Instruction *inst = &buffer->instructions[i];
        
        // Talimatın mevcut adresi (4 baytlık talimatlar varsayılarak)
        int current_address = (int)i * sizeof(MachineCodeInstruction); 
        
        MachineCodeInstruction encoded = encode_riscv_instruction(inst, current_address, sym_table);
        
        // RISC-V küçük endian (little-endian) mimaridir. 
        // Burada host sistemin endianness'ı göz ardı edilmiştir.
        // Gerçek uygulamada endianness kontrolü yapılmalıdır!

        ssize_t bytes_written = write(fd, &encoded, sizeof(MachineCodeInstruction));
        
        if (bytes_written != sizeof(MachineCodeInstruction)) {
            perror("Dosyaya yazma hatası");
            close(fd);
            return false;
        }
    }
    
    // 3. Dosyayı kapat
    if (close(fd) < 0) {
        perror("Dosya kapatma hatası");
        return false;
    }

    printf("Kod üretimi tamamlandı. Dosya boyutu: %zu bayt.\n", buffer->count * sizeof(MachineCodeInstruction));
    return true;
}