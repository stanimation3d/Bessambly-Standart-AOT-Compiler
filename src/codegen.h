#ifndef BESSAMBLY_CODEGEN_H
#define BESSAMBLY_CODEGEN_H

#include <stdbool.h>
#include <stdint.h>
#include "ir_generator.h" // CodeBuffer yapısı için

// Varsayım: Hedef mimari RISC-V RV32I (32-bit)
typedef uint32_t MachineCodeInstruction;

/**
 * @brief Üretilen RISC-V talimatlarını ikilik formata çevirir ve belirtilen dosyaya yazar.
 * * @param buffer: Optimize edilmiş RISC-V talimatlarını içeren arabellek.
 * @param output_filename: İkilik kodun yazılacağı dosya yolu (örn: "a.out").
 * @param sym_table: Sembol tablosu (atlama talimatlarındaki etiket adreslerini çözümlemek için).
 * @return true: Kod üretimi ve dosyaya yazma başarılıysa.
 */
bool codegen_write_binary(CodeBuffer *buffer, const char *output_filename, SymbolTable *sym_table);

#endif // BESSAMBLY_CODEGEN_H