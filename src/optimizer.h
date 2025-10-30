#ifndef BESSAMBLY_OPTIMIZER_H
#define BESSAMBLY_OPTIMIZER_H

#include <stdbool.h>
#include "ir_generator.h" // CodeBuffer ve Instruction yapıları için

// Bessambly Derleyicisi Optimizasyon Seviyeleri
typedef enum {
    O_LEVEL_O0 = 0,     // Optimizasyon Yok (Hata ayıklamayı kolaylaştırır)
    O_LEVEL_O1,         // Temel Optimizasyonlar (Hızlı derleme, basit iyileştirmeler)
    O_LEVEL_O2,         // Daha Kapsamlı Optimizasyonlar (Veri akışı analizi vb.)
    O_LEVEL_O3,         // Maksimum Performans Odaklı Optimizasyonlar
    O_LEVEL_FAST,       // O3 + Agresif ve Potansiyel Olarak Güvenli Olmayan Optimizasyonlar (-Ofast)
    O_LEVEL_FLASH,      // Teorik Maksimum Performans (Derin, zaman alan analizler)
    O_LEVEL_OSIZE,      // Boyut Odaklı (Temel optimizasyonlar + boyut küçültme) (-Os)
    O_LEVEL_OZ,         // Maksimum Boyut Küçültme (Daha agresif boyut optimizasyonları) (-Oz)
    O_LEVEL_NANO        // Teorik En Küçük Boyut (Minimalizm, benzersiz küçültme algoritmaları)
} OptimizationLevel;

// Optimizasyon Geçişleri İçin Bayraklar
typedef struct {
    bool constant_folding;  // Sabit Katlama (A = 5 + 3 -> A = 8)
    bool dead_code_elim;    // Ölü Kod Eleme
    bool peephole;          // Küçük Kod Bloklarının İyileştirilmesi
    bool register_alloc;    // Kayıt Ataması İyileştirmesi (Şimdilik Basit)
    bool aggressive_jump;   // Atlama zincirlerini düzleştirme (goto L1; L1: goto L2 -> goto L2)
    bool remove_nop;        // NOP (No Operation) komutlarını kaldırma
} OptimizationFlags;

/**
 * @brief Optimizasyon bayraklarını verilen optimizasyon seviyesine göre ayarlar.
 */
OptimizationFlags get_optimization_flags(OptimizationLevel level);

/**
 * @brief Kod arabelleğini (RISC-V talimatlarını) belirtilen optimizasyon seviyesine göre optimize eder.
 * @param buffer: Üzerinde çalışılacak CodeBuffer işaretçisi.
 * @param level: Uygulanacak optimizasyon seviyesi.
 * @return true: Optimizasyon başarılıysa.
 */
bool optimize_code(CodeBuffer *buffer, OptimizationLevel level);

#endif // BESSAMBLY_OPTIMIZER_H