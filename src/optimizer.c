#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "optimizer.h"
#include "error.h"

// --- Optimizasyon Geçişleri (Temel Örnekler) ---

/**
 * @brief Optimizasyon 1: NOP (No Operation) komutlarını kaldırır ve basit ölü kod elemeyi yapar.
 * Bu geçişte, bir kayda atanıp hemen ardından kullanılmayan talimatlar da kaldırılabilir.
 */
static size_t optimize_pass_cleanup(CodeBuffer *buffer) {
    size_t removed_count = 0;
    
    // NOP olarak tanımlanmış bir talimatımız yok, ama bazı talimatlar NOP olarak işlev görebilir.
    // Örnek: ADD x0, x0, x0 -> x0 (zero register) değişmeyeceği için NOP'tur.
    
    Instruction *instructions = buffer->instructions;
    size_t write_idx = 0;
    
    for (size_t read_idx = 0; read_idx < buffer->count; read_idx++) {
        Instruction *current = &instructions[read_idx];

        bool is_redundant = false;

        // Kural 1: rd = rs1 + 0 (ADDI ile) veya rd = rs1 + x0 işlemleri redundant'tır (gereksiz kopyalama).
        if (current->type == I_ADDI && current->immediate == 0) {
            // Sadece register kopyalama: ADDI rd, rs1, 0
            // Eğer rd == rs1 ise tamamen NOP'tur.
            if (current->rd == current->rs1) {
                is_redundant = true;
            }
        }
        
        // Kural 2: ADD x0, rs1, rs2 (Zero register'a atama yapılıyorsa, sonucu kullanılmaz)
        // Bessambly için basit Ölü Kod Eleme (Dead Code Elimination)
        if (current->type != I_SW && current->rd == R_ZERO) {
            is_redundant = true;
        }

        if (is_redundant) {
            removed_count++;
        } else {
            // Talimatı tut
            if (write_idx != read_idx) {
                instructions[write_idx] = instructions[read_idx];
            }
            write_idx++;
        }
    }
    
    buffer->count = write_idx;
    return removed_count;
}


// --- Optimizasyon Bayrakları Yönetimi ---

OptimizationFlags get_optimization_flags(OptimizationLevel level) {
    OptimizationFlags flags = {0}; // Tüm bayrakları sıfırla

    switch (level) {
        case O_LEVEL_O0:
            // Sadece hata ayıklama için kullanılacak.
            break;
            
        case O_LEVEL_O1:
            flags.remove_nop = true;
            flags.peephole = true;
            break;
            
        case O_LEVEL_O2:
            flags.remove_nop = true;
            flags.peephole = true;
            flags.dead_code_elim = true;
            flags.aggressive_jump = true;
            break;
            
        case O_LEVEL_O3:
        case O_LEVEL_FAST: // O3'e ek olarak daha agresif varsayımlar (şimdilik aynı)
            flags.remove_nop = true;
            flags.peephole = true;
            flags.dead_code_elim = true;
            flags.aggressive_jump = true;
            flags.constant_folding = true;
            flags.register_alloc = true; 
            break;
            
        case O_LEVEL_OSIZE: // Boyut odaklı, performanstan çok küçültme
            flags.remove_nop = true;
            flags.peephole = true;
            flags.aggressive_jump = true;
            break;
            
        case O_LEVEL_OZ: // Maksimum boyut
        case O_LEVEL_NANO: // Teorik maksimum küçültme
            flags.remove_nop = true;
            flags.peephole = true;
            flags.aggressive_jump = true;
            flags.dead_code_elim = true;
            break;
            
        case O_LEVEL_FLASH: // Teorik maksimum performans
            // Tüm bayraklar aktif kabul edilir (Gelecekteki derin analizler dahil)
            flags.remove_nop = true;
            flags.peephole = true;
            flags.dead_code_elim = true;
            flags.aggressive_jump = true;
            flags.constant_folding = true;
            flags.register_alloc = true; 
            break;
    }
    return flags;
}

// --- Ana Optimizasyon İşlevi ---

bool optimize_code(CodeBuffer *buffer, OptimizationLevel level) {
    if (level == O_LEVEL_O0) {
        printf("Optimizasyon Seviyesi -O0: Optimizasyon atlandı.\n");
        return true;
    }
    
    printf("Optimizasyon Başladı (Seviye: %d)...\n", level);
    OptimizationFlags flags = get_optimization_flags(level);
    size_t total_removed = 0;
    
    // Optimizasyon döngüsü: Optimizasyonlar sürekli olarak kodu değiştirebildiği için
    // genellikle hiçbir şeyin değişmediği bir geçiş olana kadar çalıştırılır.
    size_t changes_made = 1; // En az bir kere çalışması için 1 ile başlatıldı.
    int iteration = 0;
    
    while (changes_made > 0 && iteration < 10) { // Maksimum 10 iterasyon sınırı koyuldu
        changes_made = 0;
        
        // 1. Geçiş: Temizlik ve Basit Ölü Kod Eleme
        if (flags.remove_nop || flags.dead_code_elim) {
            size_t removed = optimize_pass_cleanup(buffer);
            changes_made += removed;
            total_removed += removed;
        }
        
        // 2. Geçiş: Peephole Optimizasyonları (Basit komut çiftlerini iyileştirme)
        if (flags.peephole) {
            // ILERI: Gerçek peephole mantığı buraya eklenecektir.
        }

        // 3. Geçiş: Atlama Zinciri Düzleştirme
        if (flags.aggressive_jump) {
            // ILERI: goto L1; L1: goto L2 -> goto L2 yapısı buraya eklenecektir.
        }
        
        iteration++;
    }
    
    printf("Optimizasyon Başarılı. Toplam %zu gereksiz talimat kaldırıldı. Iterasyon: %d\n", total_removed, iteration);
    return true;
}