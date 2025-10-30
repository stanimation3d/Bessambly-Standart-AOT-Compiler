#ifndef BESSAMBLY_SYMBOL_TABLE_H
#define BESSAMBLY_SYMBOL_TABLE_H

#include <stddef.h>
#include <stdbool.h>
#include "utils.h" // MAX_LABEL_LENGTH için

// Bessambly Sembol Tipleri
typedef enum {
    SYM_TYPE_LABEL,      // Etiketler (LOOP_START)
    SYM_TYPE_REGISTER    // Kullanıcı Tanımlı Kayıtlar (SUM, I)
    // Bellek adresleri (MEM[...]) genellikle burada tutulmaz, 
    // doğrudan sabit adresler olarak AST'de temsil edilir.
} SymbolType;

// Sembol Yapısı
typedef struct {
    char name[MAX_LABEL_LENGTH]; // Sembolün Adı (örn: "LOOP_START", "SUM")
    SymbolType type;             // Sembolün Tipi
    
    // Değerler
    union {
        // SYM_TYPE_LABEL için: Etiketin programdaki konumu (adres/satır numarası)
        long long address; 
        
        // SYM_TYPE_REGISTER için: Kaydın olası tipi/boyutu (Gelecekteki geliştirmeler için)
        int register_size; 
    } details;
} Symbol;

// Sembol Tablosu Yapısı (Basit bir dizi tabanlı tablo kullanıyoruz)
// Gerçek derleyiciler Hash Table kullanır, ancak başlangıç için dizi yeterlidir.
#define MAX_SYMBOLS 1024

typedef struct {
    Symbol entries[MAX_SYMBOLS];
    int count; // Tablodaki mevcut sembol sayısı
} SymbolTable;

/**
 * @brief Sembol Tablosunu başlatır.
 * @return SymbolTable*: Başlatılmış Sembol Tablosunun işaretçisi.
 */
SymbolTable *symtable_init();

/**
 * @brief Tabloya yeni bir sembol ekler.
 * @param table: Sembol Tablosu işaretçisi.
 * @param name: Sembolün adı.
 * @param type: Sembolün tipi (Etiket veya Kayıt).
 * @param address: Sembolün adresi (Etiketler için zorunlu).
 * @return true eğer ekleme başarılıysa, false tablo doluysa veya sembol zaten varsa.
 */
bool symtable_add(SymbolTable *table, const char *name, SymbolType type, long long address);

/**
 * @brief Tabloda isimle bir sembol arar.
 * @param table: Sembol Tablosu işaretçisi.
 * @param name: Aranacak sembolün adı.
 * @return const Symbol*: Sembol bulunursa işaretçisi, bulunamazsa NULL.
 */
const Symbol *symtable_lookup(SymbolTable *table, const char *name);

/**
 * @brief Sembol Tablosu için ayrılan belleği serbest bırakır.
 * @param table: Serbest bırakılacak Sembol Tablosu işaretçisi.
 */
void symtable_free(SymbolTable *table);

#endif // BESSAMBLY_SYMBOL_TABLE_H