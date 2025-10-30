#ifndef BESSAMBLY_PARSER_H
#define BESSAMBLY_PARSER_H

#include <stdbool.h>
#include "lexer.h"  // Token ve Lexer yapıları için
#include "ast.h"    // AST yapıları için

// Parser Durum Yapısı
typedef struct {
    Lexer *lexer;   // İlişkili Lexer (Token kaynağı)
    Token current_token; // Şu anda işlenmekte olan belirteç
    Token peek_token;    // Bir sonraki belirteç (göz ucuyla bakmak için)
    int token_index;     // Debug amaçlı token sayacı
} Parser;

/**
 * @brief Parser yapısını başlatır.
 * @param lexer: Kullanılacak başlatılmış Lexer yapısı.
 * @return Parser*: Başlatılmış Parser yapısının işaretçisi.
 */
Parser *parser_init(Lexer *lexer);

/**
 * @brief Bessambly kaynak kodunu ayrıştırır ve bir AST oluşturur.
 * @param parser: Başlatılmış Parser yapısının işaretçisi.
 * @return AST_Program*: Başarılı olursa oluşturulan AST'nin kök düğümü, başarısız olursa NULL.
 */
AST_Program *parser_parse_program(Parser *parser);

/**
 * @brief Parser tarafından kullanılan belleği serbest bırakır.
 * @param parser: Serbest bırakılacak Parser yapısının işaretçisi.
 */
void parser_free(Parser *parser);

#endif // BESSAMBLY_PARSER_H