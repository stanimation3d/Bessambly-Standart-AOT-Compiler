#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantic_analyzer.h"
#include "error.h" // Hata raporlama için

// --- Geçiş 1: Etiket Toplama ---

/**
 * @brief AST'yi dolaşır ve tüm etiket tanımlamalarını Sembol Tablosuna kaydeder.
 * * @param program: AST'nin kök düğümü.
 * @param sym_table: Sembol Tablosu.
 * @return true eğer tüm etiketler benzersiz ve başarılı bir şekilde eklendiyse.
 */
static bool pass_one_collect_labels(AST_Program *program, SymbolTable *sym_table) {
    AST_Node *current = program->first_statement;
    int instruction_address = 0; // Bessambly'de satır numarasını adres olarak kullanıyoruz

    while (current != NULL) {
        if (current->type == STMT_TYPE_LABEL_DEF) {
            const char *label_name = current->data.label_def.label_name;

            // 1. Etiketin Zaten Tanımlı Olup Olmadığını Kontrol Et
            if (symtable_lookup(sym_table, label_name) != NULL) {
                // Hata: Aynı etiket iki kez tanımlanmış
                report_error(ERR_SEMANTIC_DUPLICATE_LABEL, current->line_number, 
                             label_name);
                return false;
            }

            // 2. Etiketi tabloya ekle (Adres olarak satır numarasını kullanıyoruz)
            if (!symtable_add(sym_table, label_name, SYM_TYPE_LABEL, (long long)instruction_address)) {
                // symtable_add zaten hata raporlar, sadece çık
                return false; 
            }
        }
        
        // Etiketler de dahil olmak üzere her komut, program akışında bir yer kaplar.
        instruction_address++;
        current = current->next;
    }
    return true;
}

// --- Geçiş 2: Atlama Doğrulama ---

/**
 * @brief AST'yi dolaşır ve tüm 'goto' ve 'if-goto' komutlarındaki hedef etiketlerinin 
 * Sembol Tablosunda tanımlı olup olmadığını kontrol eder.
 * * @param program: AST'nin kök düğümü.
 * @param sym_table: Sembol Tablosu.
 * @return true eğer tüm atlama hedefleri bulunursa.
 */
static bool pass_two_verify_jumps(AST_Program *program, SymbolTable *sym_table) {
    AST_Node *current = program->first_statement;
    
    while (current != NULL) {
        const char *target_label = NULL;

        if (current->type == STMT_TYPE_GOTO) {
            target_label = current->data.goto_stmt.target_label;
        } else if (current->type == STMT_TYPE_IF_GOTO) {
            target_label = current->data.if_goto_stmt.target_label;
        }

        if (target_label != NULL) {
            // Hedef etiketi Sembol Tablosunda ara
            const Symbol *target_sym = symtable_lookup(sym_table, target_label);

            if (target_sym == NULL || target_sym->type != SYM_TYPE_LABEL) {
                // Hata: Hedef etiket bulunamadı
                report_error(ERR_SEMANTIC_UNKNOWN_LABEL, current->line_number, 
                             target_label);
                return false;
            }
        }
        
        current = current->next;
    }
    return true;
}

// --- Ana Analiz İşlevi ---

bool analyze_semantic(AST_Program *program, SymbolTable *sym_table) {
    printf("Anlambilim Analizi Başladı...\n");

    // 1. Geçiş: Etiket Toplama
    if (!pass_one_collect_labels(program, sym_table)) {
        printf("HATA: Etiket Toplama başarısız oldu.\n");
        return false;
    }

    // DEBUG: Toplanan Etiketleri Yazdır (Opsiyonel)
    // symtable_print(sym_table); 

    // 2. Geçiş: Atlama Doğrulama
    if (!pass_two_verify_jumps(program, sym_table)) {
        printf("HATA: Atlama Doğrulama başarısız oldu.\n");
        return false;
    }
    
    // (Gelecekteki Geliştirmeler: Kayıt/Tip/Kapsam Kontrolleri burada yapılabilir)

    printf("Anlambilim Analizi Başarılı.\n");
    return true;
}