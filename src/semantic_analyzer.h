#ifndef BESSAMBLY_SEMANTIC_ANALYZER_H
#define BESSAMBLY_SEMANTIC_ANALYZER_H

#include <stdbool.h>
#include "ast.h"           // AST yapısı için
#include "symbol_table.h"  // Sembol Tablosu yapısı için

/**
 * @brief Bessambly AST'sini anlambilimsel olarak analiz eder.
 * * Analiz iki geçişte yapılır:
 * 1. Etiket Toplama: Tüm etiket tanımlarını bulur ve Sembol Tablosuna kaydeder.
 * 2. Atlama Doğrulama: Tüm 'goto' ve 'if-goto' komutlarının hedef etiketlerinin
 * Sembol Tablosunda tanımlı olup olmadığını kontrol eder.
 * * @param program: Ayrıştırılmış AST'nin kök düğümü.
 * @param sym_table: Başlatılmış Sembol Tablosu işaretçisi.
 * @return true: Anlambilimsel analiz başarılıysa (hata yoksa).
 * @return false: Analiz sırasında kritik bir anlambilim hatası bulunursa.
 */
bool analyze_semantic(AST_Program *program, SymbolTable *sym_table);

#endif // BESSAMBLY_SEMANTIC_ANALYZER_H