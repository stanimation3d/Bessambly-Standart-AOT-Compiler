#ifndef BESSAMBLY_LEXER_H
#define BESSAMBLY_LEXER_H

#include <stddef.h> // size_t için
#include "utils.h"  // MAX_LINE_LENGTH için

// Bessambly Dilindeki Tüm Belirteç Tipleri
typedef enum {
    // 1. Anahtar Kelimeler (Reserved Keywords)
    TOKEN_KEYWORD_IF,      // if
    TOKEN_KEYWORD_GOTO,    // goto
    TOKEN_KEYWORD_MEM,     // MEM
    // Not: Bessambly'nin minimalist yapısı gereği bu aşamada başka anahtar kelime yok.

    // 2. Operatörler ve Ayırıcılar (Operators and Delimiters)
    TOKEN_OP_ADD,       // +
    TOKEN_OP_SUB,       // -
    TOKEN_OP_MUL,       // *
    TOKEN_OP_DIV,       // /
    TOKEN_OP_AND,       // &
    TOKEN_OP_OR,        // |
    TOKEN_OP_LT,        // <
    TOKEN_OP_GT,        // >
    TOKEN_OP_EQ_EQ,     // ==
    TOKEN_OP_NOT_EQ,    // !=
    TOKEN_OP_LE,        // <=
    TOKEN_OP_GE,        // >=
    TOKEN_OP_EQ,        // = (Atama)
    TOKEN_COLON,        // : (Etiket Tanımlayıcı)
    TOKEN_LBRACKET,     // [
    TOKEN_RBRACKET,     // ]
    
    // 3. Değerler ve Tanımlayıcılar (Values and Identifiers)
    TOKEN_INTEGER,      // Sayısal sabitler (örn: 10, 0xAF)
    TOKEN_IDENTIFIER,   // Kayıt isimleri, Etiket isimleri (örn: A, B, SUM, LOOP_START)
    
    // 4. Özel Belirteçler (Special Tokens)
    TOKEN_NEWLINE,      // Yeni satır karakteri (Komut sonu)
    TOKEN_EOF,          // Dosya Sonu (End of File)
    TOKEN_ERROR         // Tanımlanamayan bir karakter veya hata
    
} TokenType;

// Belirteç Yapısı
typedef struct {
    TokenType type;                   // Belirteç tipi
    char lexeme[MAX_LINE_LENGTH + 1]; // Kaynak koddaki belirtecin metin değeri (lexeme)
    int line;                         // Belirtecin bulunduğu satır numarası
    
    // Değerler için (eğer belirteç bir sayı ise)
    long long int value;
} Token;

// Lexer Durum Yapısı (Dosya okuma bağlamını tutar)
typedef struct {
    char *source_code;    // Kaynak kodun tamamı
    size_t source_length; // Kaynak kodun uzunluğu
    size_t current_pos;   // Şu an işlenen karakterin konumu
    int current_line;     // Şu an işlenen satır numarası
} Lexer;


/**
 * @brief Lexer yapısını başlatır.
 * @param source_code: Bellekteki Bessambly kaynak kodunun işaretçisi.
 * @return Lexer*: Başlatılmış Lexer yapısının işaretçisi.
 */
Lexer *lexer_init(char *source_code);

/**
 * @brief Sonraki belirteci okur ve döndürür.
 * @param lexer: Lexer yapısının işaretçisi.
 * @return Token: Okunan belirteç.
 */
Token lexer_get_next_token(Lexer *lexer);

/**
 * @brief Lexer tarafından kullanılan belleği serbest bırakır (kaynak kod hariç).
 * @param lexer: Lexer yapısının işaretçisi.
 */
void lexer_free(Lexer *lexer);

#endif // BESSAMBLY_LEXER_H