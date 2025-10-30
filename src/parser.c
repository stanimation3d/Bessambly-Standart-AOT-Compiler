#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "error.h" // Hata raporlama için
#include "utils.h" // Yardımcı fonksiyonlar için

// --- Yardımcı Fonksiyonlar ---

// Belirteç Akışını Yönetme

/**
 * @brief Sonraki belirteci okur ve current_token'ı günceller.
 */
static void advance_token(Parser *parser) {
    parser->current_token = parser->peek_token;
    parser->peek_token = lexer_get_next_token(parser->lexer);
    parser->token_index++;

    // Yeni satır belirteçlerini yoksay
    while (parser->current_token.type == TOKEN_NEWLINE) {
         parser->current_token = parser->peek_token;
         parser->peek_token = lexer_get_next_token(parser->lexer);
    }
}

/**
 * @brief Mevcut belirtecin beklenen tipte olup olmadığını kontrol eder.
 * Beklenmiyorsa sözdizimi hatası rapor eder ve çıkar.
 */
static void expect_token(Parser *parser, TokenType expected_type, const char *error_msg) {
    if (parser->current_token.type != expected_type) {
        // Hata raporla: Satır, Hata Kodu, Detay Mesaj
        report_error(ERR_SYNTAX_INVALID_TOKEN, parser->current_token.line, error_msg);
    }
    // Başarılı olursa sonraki tokene geç
    advance_token(parser);
}

// AST Yapıları Oluşturma Yardımcıları

/**
 * @brief Token tipini AST Binary Operatör tipine çevirir.
 */
static BinaryOperatorType token_to_bin_op(TokenType type) {
    switch (type) {
        case TOKEN_OP_ADD: return OP_BIN_ADD;
        case TOKEN_OP_SUB: return OP_BIN_SUB;
        case TOKEN_OP_MUL: return OP_BIN_MUL;
        case TOKEN_OP_DIV: return OP_BIN_DIV;
        case TOKEN_OP_AND: return OP_BIN_AND;
        case TOKEN_OP_OR:  return OP_BIN_OR;
        default: return OP_BIN_ADD; // Varsayılan/Hata durumu
    }
}

/**
 * @brief Token tipini AST Koşul Operatör tipine çevirir.
 */
static ConditionOperatorType token_to_cond_op(TokenType type) {
    switch (type) {
        case TOKEN_OP_GT:    return OP_COND_GT;
        case TOKEN_OP_LT:    return OP_COND_LT;
        case TOKEN_OP_EQ_EQ: return OP_COND_EQ;
        case TOKEN_OP_NOT_EQ: return OP_COND_NE;
        case TOKEN_OP_GE:    return OP_COND_GE;
        case TOKEN_OP_LE:    return OP_COND_LE;
        default: report_error(ERR_SYNTAX_INVALID_TOKEN, 0, "Geçersiz koşul operatörü."); return OP_COND_EQ;
    }
}


/**
 * @brief Token'dan bir AST_Operand oluşturur (A, 10 veya MEM[addr]).
 * @param is_dest: Bu operantın atama hedefi olup olmadığı.
 */
static AST_Operand parse_operand(Parser *parser, bool is_dest) {
    AST_Operand operand = {0};
    operand.mem_type = MEM_ACCESS_NONE;

    if (parser->current_token.type == TOKEN_INTEGER) {
        // Doğrudan sayı sabiti (örn: 10)
        if (is_dest) {
             report_error(ERR_SYNTAX_INVALID_TOKEN, parser->current_token.line, "Atama hedefi bir sabit sayı olamaz.");
        }
        operand.type = OPR_TYPE_IMMEDIATE_INT;
        operand.value = parser->current_token.value;
        advance_token(parser);
        
    } else if (parser->current_token.type == TOKEN_IDENTIFIER) {
        // Kayıt (örn: A, SUM)
        operand.type = OPR_TYPE_REGISTER;
        strncpy(operand.name, parser->current_token.lexeme, MAX_LABEL_LENGTH);
        advance_token(parser);
        
    } else if (parser->current_token.type == TOKEN_KEYWORD_MEM) {
        // Bellek Adresi (MEM[addr])
        advance_token(parser); // MEM tüketildi
        expect_token(parser, TOKEN_LBRACKET, "'MEM' sonrasında '[' bekleniyor.");
        
        // Bellek adresi sadece INTEGER olmalıdır (Bessambly kısıtlaması)
        if (parser->current_token.type != TOKEN_INTEGER) {
            report_error(ERR_SYNTAX_INVALID_ADDRESS, parser->current_token.line, "Bellek adresi sadece sayısal sabit olmalıdır.");
        }
        
        operand.type = OPR_TYPE_MEMORY_ADDR;
        operand.mem_type = MEM_ACCESS_FIXED;
        operand.mem_address = parser->current_token.value;
        advance_token(parser); // Adres tüketildi
        
        expect_token(parser, TOKEN_RBRACKET, "Bellek adreslemesinden sonra ']' bekleniyor.");
        
    } else {
        // Geçersiz Operant
        report_error(ERR_SYNTAX_INVALID_TOKEN, parser->current_token.line, "Beklenmeyen bir operant türü.");
    }
    
    return operand;
}


/**
 * @brief Basit bir ifadeyi (Operand veya Binary İşlem) ayrıştırır.
 * (C = A + 10) veya (C = D) durumları için kullanılır.
 */
static AST_Expr parse_expression(Parser *parser) {
    AST_Expr expr = {0};
    
    // Sol tarafı operant olarak al (A veya 10 veya MEM[..])
    AST_Operand left_opr = parse_operand(parser, false);
    
    // Eğer operanttan sonra aritmetik operatör gelmiyorsa, ifade tek bir operanttır.
    if (parser->current_token.type < TOKEN_OP_ADD || parser->current_token.type > TOKEN_OP_OR) {
        expr.type = EXPR_TYPE_OPERAND;
        expr.data.operand = left_opr;
        return expr;
    }
    
    // İkili işlem (Binary Operation)
    TokenType op_type = parser->current_token.type;
    advance_token(parser); // Operatör tüketildi
    
    // Sağ tarafı operant olarak al
    AST_Operand right_opr = parse_operand(parser, false);
    
    // AST düğümünü oluştur
    expr.type = EXPR_TYPE_BINARY;
    expr.data.binary_op.op = token_to_bin_op(op_type);
    expr.data.binary_op.left = left_opr;
    expr.data.binary_op.right = right_opr;
    
    return expr;
}

// --- Komut Ayrıştırma İşlevleri ---

/**
 * @brief Atama komutunu (dest = expr) ayrıştırır.
 */
static AST_Node *parse_assignment(Parser *parser, int line_num) {
    AST_Node *node = ast_node_create(STMT_TYPE_ASSIGNMENT, line_num);
    
    // 1. Hedef Operant (Sol Taraf)
    node->data.assignment.destination = parse_operand(parser, true);
    
    // 2. Atama Operatörü
    expect_token(parser, TOKEN_OP_EQ, "'=' atama operatörü bekleniyor.");
    
    // 3. İfade (Sağ Taraf)
    node->data.assignment.expression = parse_expression(parser);
    
    return node;
}

/**
 * @brief Koşulsuz Atlama (goto TARGET) komutunu ayrıştırır.
 */
static AST_Node *parse_goto(Parser *parser, int line_num) {
    // 'goto' anahtar kelimesi zaten tüketilmiş varsayılıyor.
    
    // 1. Etiket İsmi (IDENTIFIER)
    expect_token(parser, TOKEN_IDENTIFIER, "'goto' sonrasında hedef etiket ismi bekleniyor.");
    
    AST_Node *node = ast_node_create(STMT_TYPE_GOTO, line_num);
    strncpy(node->data.goto_stmt.target_label, parser->current_token.lexeme, MAX_LABEL_LENGTH);
    advance_token(parser); // Etiket tüketildi
    
    return node;
}

/**
 * @brief Koşullu Atlama (if A > B goto TARGET) komutunu ayrıştırır.
 */
static AST_Node *parse_if_goto(Parser *parser, int line_num) {
    // 'if' anahtar kelimesi zaten tüketilmiş varsayılıyor.
    
    AST_Node *node = ast_node_create(STMT_TYPE_IF_GOTO, line_num);
    
    // 1. Sol Operant
    node->data.if_goto_stmt.left = parse_operand(parser, false);
    
    // 2. Koşul Operatörü
    TokenType op_type = parser->current_token.type;
    if (op_type < TOKEN_OP_LT || op_type > TOKEN_OP_GE) {
         report_error(ERR_SYNTAX_INVALID_TOKEN, parser->current_token.line, "Geçerli bir koşul operatörü bekleniyor (>, <, ==, vb.).");
    }
    node->data.if_goto_stmt.op = token_to_cond_op(op_type);
    advance_token(parser); // Operatör tüketildi
    
    // 3. Sağ Operant
    node->data.if_goto_stmt.right = parse_operand(parser, false);
    
    // 4. 'goto' Anahtar Kelimesi
    expect_token(parser, TOKEN_KEYWORD_GOTO, "Koşuldan sonra 'goto' anahtar kelimesi bekleniyor.");
    
    // 5. Etiket İsmi
    expect_token(parser, TOKEN_IDENTIFIER, "'goto' sonrasında hedef etiket ismi bekleniyor.");
    strncpy(node->data.if_goto_stmt.target_label, parser->current_token.lexeme, MAX_LABEL_LENGTH);
    advance_token(parser); // Etiket tüketildi
    
    return node;
}


/**
 * @brief Tek bir komutu (statement) ayrıştırır.
 */
static AST_Node *parse_statement(Parser *parser) {
    Token current = parser->current_token;
    int line_num = current.line;
    
    // 1. Etiket Tanımlama Kontrolü (LABEL:)
    if (current.type == TOKEN_IDENTIFIER && parser->peek_token.type == TOKEN_COLON) {
        AST_Node *node = ast_node_create(STMT_TYPE_LABEL_DEF, line_num);
        
        // Etiket adı
        strncpy(node->data.label_def.label_name, current.lexeme, MAX_LABEL_LENGTH);
        advance_token(parser); // Etiket adı tüketildi
        advance_token(parser); // Kolon (:) tüketildi
        
        return node;
    }
    
    // 2. Kontrol Yapıları
    if (current.type == TOKEN_KEYWORD_IF) {
        advance_token(parser); // 'if' tüketildi
        return parse_if_goto(parser, line_num);
    }
    
    if (current.type == TOKEN_KEYWORD_GOTO) {
        advance_token(parser); // 'goto' tüketildi
        return parse_goto(parser, line_num);
    }
    
    // 3. Atama İşlemi (dest = expr)
    // Atama işlemi bir kayıt (A) veya bir bellek adresi (MEM[..]) ile başlamalıdır.
    if (current.type == TOKEN_IDENTIFIER || current.type == TOKEN_KEYWORD_MEM) {
        return parse_assignment(parser, line_num);
    }
    
    // 4. Hata Durumu (Beklenmeyen Komut Başlangıcı)
    report_error(ERR_SYNTAX_INVALID_TOKEN, line_num, "Geçersiz komut başlangıcı (Etiket, Atama, 'if' veya 'goto' bekleniyor).");
    return NULL;
}


// --- Genel İşlev Uygulamaları ---

Parser *parser_init(Lexer *lexer) {
    Parser *parser = (Parser *)safe_malloc(sizeof(Parser));
    parser->lexer = lexer;
    parser->token_index = 0;
    
    // Parser'ı başlatmak için ilk iki token'ı oku
    parser->peek_token = lexer_get_next_token(lexer);
    advance_token(parser); 
    
    return parser;
}

AST_Program *parser_parse_program(Parser *parser) {
    AST_Program *program = ast_program_create();
    AST_Node *current_stmt = NULL;
    
    // Dosya sonuna kadar komutları ayrıştır
    while (parser->current_token.type != TOKEN_EOF) {
        AST_Node *new_stmt = parse_statement(parser);
        
        if (new_stmt != NULL) {
            // AST düğümünü bağlı listeye ekle
            if (program->first_statement == NULL) {
                program->first_statement = new_stmt;
            } else {
                current_stmt->next = new_stmt;
            }
            current_stmt = new_stmt;
        }
        
        // Atamalar ve kontrol komutlarından sonra NEWLINE beklenir (Lexer'da yoksayılıyor) 
        // Ancak burada bir sonraki komutun başında olası fazlalık tokenları kontrol etmeye devam et
    }
    
    // Ayrıştırma başarılı oldu
    return program;
}

void parser_free(Parser *parser) {
    // Lexer'ı burada serbest bırakmak güvenli olabilir, ancak main'de kontrol etmek daha iyidir.
    // lexer_free(parser->lexer); 
    free(parser);
}