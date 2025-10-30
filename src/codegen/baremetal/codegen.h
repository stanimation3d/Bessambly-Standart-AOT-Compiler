#ifndef BESSAMBLY_BAREMETAL_CODEGEN_H
#define BESSAMBLY_BAREMETAL_CODEGEN_H

#include <stdbool.h>
#include <stdint.h>
#include "ir_generator.h" // CodeBuffer yapısı için
#include "symbol_table.h" // Sembol Tablosu için

// Varsayım: Hedef mimari RISC-V RV32I (32-bit)
typedef uint32_t MachineCodeInstruction;

/**
 * @brief Üretilen RISC-V talimatlarını ham ikilik (flat binary) formata çevirir ve dosyaya yazar.
 * * Bare-Metal ortamı için çıktı, doğrudan hedef donanımın belleğine yüklenebilecek 
 * ham makine kodu dizisi olacaktır.
 * * @param buffer: Optimize edilmiş RISC-V talimatlarını içeren arabellek.
 * @param output_filename: İkilik kodun yazılacağı dosya yolu (örn: "program.bin").
 * @param sym_table: Sembol tablosu (atlama talimatlarındaki etiket adreslerini çözümlemek için).
 * @return true: Kod üretimi ve dosyaya yazma başarılıysa.
 */
bool codegen_write_baremetal_binary(CodeBuffer *buffer, const char *output_filename, SymbolTable *sym_table);

#endif // BESSAMBLY_BAREMETAL_CODEGEN_H
