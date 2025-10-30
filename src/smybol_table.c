#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"
#include "error.h" // Hata raporlama için
#include "utils.h" // safe_malloc/safe_realloc için

SymbolTable *symtable_init() {
    SymbolTable *table = (SymbolTable *)safe_malloc(sizeof(SymbolTable));
    table->count = 0;
    // Diziyi sıfırlamaya gerek yok, count ile kontrol ediliyor.
    return table;
}

bool symtable_add(SymbolTable *table, const char *name, SymbolType type, long long address) {
    if (table->count >= MAX_SYMBOLS) {
        report_error(ERR_OUT_OF_MEMORY, 0, "Sembol Tablosu dolu, yeni sembol eklenemiyor.");
        return false;
    }

    // Sembolün zaten var olup olmadığını kontrol et (Özellikle Etiketler için önemli)
    if (symtable_lookup(table, name) != NULL) {
        // Sembolik analiz aşamasında bu durum "DUPLICATE_LABEL" hatası olarak raporlanmalıdır.
        return false; 
    }

    // Yeni Sembolü Ekle
    Symbol *new_sym = &table->entries[table->count];
    
    strncpy(new_sym->name, name, MAX_LABEL_LENGTH - 1);
    new_sym->name[MAX_LABEL_LENGTH - 1] = '\0';
    new_sym->type = type;
    
    if (type == SYM_TYPE_LABEL) {
        new_sym->details.address = address;
    } 
    // Kayıtlar için şimdilik adres/size ataması yapmıyoruz, ileride eklenebilir.
    
    table->count++;
    return true;
}

const Symbol *symtable_lookup(SymbolTable *table, const char *name) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->entries[i].name, name) == 0) {
            return &table->entries[i];
        }
    }
    return NULL; // Bulunamadı
}

void symtable_free(SymbolTable *table) {
    if (table != NULL) {
        free(table);
    }
}

// --- DEBUG Fonksiyonu (Opsiyonel) ---
void symtable_print(SymbolTable *table) {
    printf("\n--- Sembol Tablosu İçeriği (%d Sembol) ---\n", table->count);
    for (int i = 0; i < table->count; i++) {
        const Symbol *sym = &table->entries[i];
        printf("[%03d] %-15s | Tipi: %-10s", 
               i, 
               sym->name, 
               sym->type == SYM_TYPE_LABEL ? "LABEL" : "REGISTER");
        
        if (sym->type == SYM_TYPE_LABEL) {
            printf("| Adresi: 0x%llX (Satır %lld)", sym->details.address, sym->details.address);
        }
        printf("\n");
    }
    printf("------------------------------------------\n");
}