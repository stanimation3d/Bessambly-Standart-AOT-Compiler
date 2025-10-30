#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"
#include "error.h" // Hata raporlama için
#include "utils.h" // Yardımcı fonksiyonlar ve sabitler için

// İleri hareket etmeden mevcut karakteri döndürür
static char lexer_peek(Lexer *lexer) {
    if (lexer->current_pos >= lexer->source_length) {
        return '\0'; // Dosya Sonu
    }
    return lexer->source_code[lexer->current_pos];
}

// Mevcut karakteri tüketir ve bir sonraki karaktere geçer
static char lexer_advance(Lexer *lexer) {
    char c = lexer_peek(lexer);
    if (c != '\0') {
        lexer->current_pos++;
        if (c == '\n') {
            lexer->current_line++;
        }
    }
    return c;
}

// Belirteçleri hızlıca karşılaştırmak için anahtar kelime kontrolü
static TokenType check_keyword(const char *lexeme) {
    if (strcmp(lexeme, "if") == 0) return TOKEN_KEYWORD_IF;
    if (strcmp(lexeme, "goto") == 0) return TOKEN_KEYWORD_GOTO;
    if (strcmp(lexeme, "MEM") == 0) return TOKEN_KEYWORD_MEM;
    
    // Anahtar kelime değilse, basit bir tanımlayıcıdır (kayıt veya etiket)
    return TOKEN_IDENTIFIER;
}

// Sayıları (Decimal veya Hexadecimal) işler
static void lexer_number(Lexer *lexer, Token *token) {
    char c = lexer_peek(lexer);
    int base = 10;
    
    // Hexadecimal kontrolü: 0x veya 0X
    if (c == '0' && (lexer_peek(lexer) == 'x' || lexer_peek(lexer) == 'X')) {
        lexer_advance(lexer); // 0
        lexer_advance(lexer); // x/X
        base = 16;
    }
    
    // Sayısal değeri oku ve belirtece yaz
    char *endptr;
    token->value = strtoll(&lexer->source_code[lexer->current_pos], &endptr, base);
    
    // Okunan bayt sayısını ilerlet
    lexer->current_pos += (endptr - &lexer->source_code[lexer->current_pos]);
    token->type = TOKEN_INTEGER;
}

// Tanımlayıcıları (Etiket/Kayıt) işler
static void lexer_identifier(Lexer *lexer, Token *token) {
    size_t start_pos = lexer->current_pos;
    size_t length = 0;
    
    // Geçerli tanımlayıcı karakterleri okumaya devam et
    while (lexer_peek(lexer) != '\0' && is_valid_char(lexer_peek(lexer)) && length < MAX_LABEL_LENGTH) {
        lexer_advance(lexer);
        length++;
    }

    if (length == 0) {
        // Bu durum olmamalı, çünkü is_valid_start_char zaten kontrol edildi.
        report_error(ERR_SYNTAX_INVALID_TOKEN, lexer->current_line, "Tanımlayıcı okunamadı.");
        return;
    }

    // Lexeme'i kopyala ve sonlandır
    strncpy(token->lexeme, &lexer->source_code[start_pos], length);
    token->lexeme[length] = '\0';

    // Anahtar kelime mi yoksa tanımlayıcı mı olduğunu kontrol et
    token->type = check_keyword(token->lexeme);
}

// --- Genel İşlev Uygulamaları ---

Lexer *lexer_init(char *source_code) {
    Lexer *lexer = (Lexer *)safe_malloc(sizeof(Lexer));
    lexer->source_code = source_code;
    lexer->source_length = strlen(source_code);
    lexer->current_pos = 0;
    lexer->current_line = 1; // Genellikle satır numarası 1'den başlar
    return lexer;
}

void lexer_free(Lexer *lexer) {
    // Kaynak kod başka bir yerde tutuluyorsa, sadece lexer yapısını serbest bırak
    free(lexer); 
}

Token lexer_get_next_token(Lexer *lexer) {
    Token token;
    token.line = lexer->current_line;
    token.value = 0;
    token.lexeme[0] = '\0';

    // 1. Boşlukları ve Yorumları Atla
    while (1) {
        char c = lexer_peek(lexer);

        // Boşlukları atla (newline hariç)
        while (c != '\0' && is_whitespace(c) && c != '\n') {
            lexer_advance(lexer);
            c = lexer_peek(lexer);
        }

        // Yorumları atla
        if (c == '/' && lexer->source_code[lexer->current_pos + 1] == '/') {
            // İki slash'ı da tüket
            lexer_advance(lexer);
            lexer_advance(lexer);
            
            // Satır sonuna kadar ilerle
            while (lexer_peek(lexer) != '\0' && lexer_peek(lexer) != '\n') {
                lexer_advance(lexer);
            }
            continue; // Yeniden boşluk kontrolü yap
        }

        // Yorum veya boşluk yoksa döngüden çık
        break;
    }

    // 2. Dosya Sonu Kontrolü
    char c = lexer_advance(lexer);
    if (c == '\0') {
        token.type = TOKEN_EOF;
        return token;
    }
    
    // 3. Yeni Satır (Komut Sonu)
    if (c == '\n') {
        token.type = TOKEN_NEWLINE;
        // token.line zaten ayarlandı.
        strcpy(token.lexeme, "\\n");
        return token;
    }

    // 4. Tanımlayıcılar (Kayıt, Etiket, Anahtar Kelime)
    if (is_valid_start_char(c)) {
        // Geri sar (çünkü advance ile tüketildi)
        lexer->current_pos--; 
        lexer->current_line = (c == '\n' ? lexer->current_line - 1 : lexer->current_line);
        
        lexer_identifier(lexer, &token);
        return token;
    }

    // 5. Sayılar (Tamsayılar)
    if (isdigit((unsigned char)c)) {
        // Geri sar (çünkü advance ile tüketildi)
        lexer->current_pos--;
        lexer_number(lexer, &token);
        return token;
    }

    // 6. Tek ve Çift Karakterli Operatörler/Ayırıcılar
    token.lexeme[0] = c;
    token.lexeme[1] = '\0';
    
    char next_c = lexer_peek(lexer);

    switch (c) {
        case '+': token.type = TOKEN_OP_ADD; break;
        case '-': token.type = TOKEN_OP_SUB; break;
        case '*': token.type = TOKEN_OP_MUL; break;
        case '/': token.type = TOKEN_OP_DIV; break;
        case '&': token.type = TOKEN_OP_AND; break;
        case '|': token.type = TOKEN_OP_OR; break;
        case ':': token.type = TOKEN_COLON; break;
        case '[': token.type = TOKEN_LBRACKET; break;
        case ']': token.type = TOKEN_RBRACKET; break;
        
        case '=':
            if (next_c == '=') {
                lexer_advance(lexer);
                token.type = TOKEN_OP_EQ_EQ; // ==
                strcat(token.lexeme, "=");
            } else {
                token.type = TOKEN_OP_EQ;    // = (Atama)
            }
            break;
            
        case '>':
            if (next_c == '=') {
                lexer_advance(lexer);
                token.type = TOKEN_OP_GE;    // >=
                strcat(token.lexeme, "=");
            } else {
                token.type = TOKEN_OP_GT;    // >
            }
            break;
            
        case '<':
            if (next_c == '=') {
                lexer_advance(lexer);
                token.type = TOKEN_OP_LE;    // <=
                strcat(token.lexeme, "=");
            } else {
                token.type = TOKEN_OP_LT;    // <
            }
            break;
            
        case '!':
            if (next_c == '=') {
                lexer_advance(lexer);
                token.type = TOKEN_OP_NOT_EQ; // !=
                strcat(token.lexeme, "=");
            } else {
                // Sadece '!' karakteri Bessambly'de geçerli bir belirteç değildir.
                report_error(ERR_SYNTAX_INVALID_TOKEN, token.line, "Tek başına '!' geçersiz.");
                token.type = TOKEN_ERROR;
            }
            break;

        default:
            // Tanımlanmamış karakter
            report_error(ERR_SYNTAX_INVALID_TOKEN, token.line, "Bilinmeyen karakter.");
            token.type = TOKEN_ERROR;
            break;
    }

    return token;
}