#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// Derleyici Bileşenleri
#include "error.h"             // Hata Yönetimi
#include "utils.h"             // Yardımcı Fonksiyonlar (safe_malloc vb.)
#include "lexer.h"             // Belirteçleyici
#include "parser.h"            // Ayrıştırıcı
#include "ast.h"               // Soyut Sözdizimi Ağacı
#include "symbol_table.h"      // Sembol Tablosu
#include "semantic_analyzer.h" // Anlambilim Analizi
#include "ir_generator.h"      // RISC-V Talimat Üretimi
#include "optimizer.h"         // Kod Optimizasyonu

// Hedefe Özgü Kod Üreticiler
#include "codegen/unix/codegen.h"       // UNIX için
#include "codegen/baremetal/codegen.h"  // Bare-Metal için


// --- 1. Yardımcı Fonksiyon: Dosyayı Belleğe Oku ---

/**
 * @brief Giriş dosyasının tüm içeriğini belleğe okur.
 * @param filename: Okunacak dosya yolu.
 * @return char*: Dosya içeriği (NULL ile sonlandırılmış), hata durumunda NULL.
 */
static char *read_source_file(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "HATA: Giriş dosyası açılamadı: %s\n", filename);
        return NULL;
    }

    // Dosya boyutunu bul
    fseek(fp, 0, SEEK_END);
    long length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Bellek tahsisi (uzunluk + 1 (null sonlandırıcı için))
    char *buffer = (char *)safe_malloc(length + 1);
    
    // İçeriği oku
    size_t read_bytes = fread(buffer, 1, length, fp);
    if (read_bytes != (size_t)length) {
        fprintf(stderr, "HATA: Dosya okuma hatası: %s\n", filename);
        free(buffer);
        fclose(fp);
        return NULL;
    }

    buffer[length] = '\0'; // Null sonlandırma
    fclose(fp);
    return buffer;
}


// --- 2. Yardımcı Fonksiyon: Kullanım Kılavuzu ---

static void print_usage(const char *prog_name) {
    fprintf(stderr, "Kullanım: %s <giriş_dosyası> [seçenekler]\n", prog_name);
    fprintf(stderr, "\nSeçenekler:\n");
    fprintf(stderr, "  -o <dosya>        Çıktı dosyasının adını belirtir (Varsayılan: a.out)\n");
    fprintf(stderr, "  -O<seviye>        Optimizasyon seviyesi (örn: -O1, -O2, -O3, -Ofast, -Oz)\n");
    fprintf(stderr, "  -target <platform> Hedef platform (unix veya baremetal) (Varsayılan: unix)\n");
    fprintf(stderr, "\nDesteklenen Optimizasyon Seviyeleri:\n");
    fprintf(stderr, "  -O0 (Kapalı), -O1, -O2, -O3, -Ofast, -Oflash, -Os, -Oz, -Onano\n");
}

// --- 3. Ana Fonksiyon: Komut Satırı Argümanlarını İşleme ve Derleme ---

int main(int argc, char *argv[]) {
    // Varsayılan Ayarlar
    const char *input_filename = NULL;
    const char *output_filename = "a.out";
    OptimizationLevel opt_level = O_LEVEL_O0;
    const char *target_platform = "unix";
    
    // Argümanları İşle
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                output_filename = argv[++i];
            } else {
                fprintf(stderr, "HATA: '-o' seçeneği dosya adı gerektirir.\n");
                return 1;
            }
        } else if (strncmp(argv[i], "-O", 2) == 0) {
            const char *level_str = argv[i] + 2;
            if (strcmp(level_str, "0") == 0) opt_level = O_LEVEL_O0;
            else if (strcmp(level_str, "1") == 0) opt_level = O_LEVEL_O1;
            else if (strcmp(level_str, "2") == 0) opt_level = O_LEVEL_O2;
            else if (strcmp(level_str, "3") == 0) opt_level = O_LEVEL_O3;
            else if (strcmp(level_str, "fast") == 0) opt_level = O_LEVEL_FAST;
            else if (strcmp(level_str, "flash") == 0) opt_level = O_LEVEL_FLASH;
            else if (strcmp(level_str, "s") == 0) opt_level = O_LEVEL_OSIZE;
            else if (strcmp(level_str, "z") == 0) opt_level = O_LEVEL_OZ;
            else if (strcmp(level_str, "nano") == 0) opt_level = O_LEVEL_NANO;
            else {
                fprintf(stderr, "HATA: Geçersiz optimizasyon seviyesi: %s\n", argv[i]);
                return 1;
            }
        } else if (strcmp(argv[i], "-target") == 0) {
            if (i + 1 < argc) {
                target_platform = argv[++i];
                if (strcmp(target_platform, "unix") != 0 && strcmp(target_platform, "baremetal") != 0) {
                    fprintf(stderr, "HATA: Desteklenmeyen hedef platform. 'unix' veya 'baremetal' olmalıdır.\n");
                    return 1;
                }
            } else {
                fprintf(stderr, "HATA: '-target' seçeneği platform adı gerektirir.\n");
                return 1;
            }
        } else if (input_filename == NULL) {
            input_filename = argv[i];
        } else {
            fprintf(stderr, "HATA: Bilinmeyen argüman veya birden fazla giriş dosyası: %s\n", argv[i]);
            return 1;
        }
    }

    if (input_filename == NULL) {
        print_usage(argv[0]);
        return 1;
    }

    // --- Derleme Akışı Başlangıcı ---
    
    char *source_code = read_source_file(input_filename);
    if (source_code == NULL) {
        return 1;
    }

    printf("Bessambly AOT Compiler Başlatılıyor...\n");
    printf("Giriş: %s, Çıktı: %s, Opt: -O%s, Hedef: %s\n", 
           input_filename, output_filename, argv[argc - (strcmp(argv[argc - 1], "fast") == 0 ? 1 : 1)], target_platform); // Basitleştirilmiş opt. seviyesi gösterimi
    
    // 1. Lexer (Belirteçleyici) Aşaması
    Lexer *lexer = lexer_init(source_code);
    Parser *parser = NULL;
    AST_Program *ast = NULL;
    SymbolTable *sym_table = symtable_init();
    CodeBuffer *riscv_code = NULL;
    int return_code = 0;
    
    // 2. Parser (Ayrıştırıcı) Aşaması
    parser = parser_init(lexer);
    printf("Parser Aşaması Başladı...\n");
    ast = parser_parse_program(parser);
    
    if (ast == NULL) {
        fprintf(stderr, "DERLEME HATA: Sözdizimi hataları nedeniyle durduruldu.\n");
        return_code = 1;
        goto cleanup;
    }

    // ast_print(ast); // DEBUG: AST'yi Yazdır

    // 3. Semantic Analyzer (Anlambilim Çözümleyici) Aşaması
    if (!analyze_semantic(ast, sym_table)) {
        fprintf(stderr, "DERLEME HATA: Anlambilim hataları nedeniyle durduruldu.\n");
        return_code = 1;
        goto cleanup;
    }

    // 4. IR Generator (Kod Üretimi) Aşaması
    riscv_code = generate_riscv_code(ast, sym_table);
    if (riscv_code == NULL) {
        fprintf(stderr, "DERLEME HATA: Kod üretimi başarısız oldu.\n");
        return_code = 1;
        goto cleanup;
    }
    
    // 5. Optimizer (Optimizasyon) Aşaması
    if (opt_level != O_LEVEL_O0) {
        if (!optimize_code(riscv_code, opt_level)) {
             fprintf(stderr, "DERLEME HATA: Optimizasyon başarısız oldu.\n");
             // Hata olsa bile devam edebiliriz, ancak güvenli bir çıkış yapalım.
             return_code = 1; 
             goto cleanup;
        }
    }
    
    // print_riscv_code(riscv_code); // DEBUG: Optimizasyon sonrası kodu yazdır

    // 6. Codegen (Hedefe Özgü İkilik Dosya Yazma) Aşaması
    bool codegen_success = false;
    if (strcmp(target_platform, "unix") == 0) {
        codegen_success = codegen_write_binary(riscv_code, output_filename, sym_table);
    } else if (strcmp(target_platform, "baremetal") == 0) {
        codegen_success = codegen_write_baremetal_binary(riscv_code, output_filename, sym_table);
    }

    if (!codegen_success) {
        fprintf(stderr, "DERLEME HATA: Kod çıktısı dosyaya yazılamadı.\n");
        return_code = 1;
    }

// --- Bellek Temizleme (Cleanup) ---
cleanup:
    if (riscv_code) code_buffer_free(riscv_code);
    if (sym_table) symtable_free(sym_table);
    if (ast) ast_program_free(ast);
    if (parser) parser_free(parser);
    if (lexer) lexer_free(lexer);
    if (source_code) free(source_code);

    if (return_code == 0) {
        printf("Tebrikler! Derleme başarılı. Çıktı dosyası: %s\n", output_filename);
    } else {
        printf("Derleme tamamlandı, ancak hatalar bulundu.\n");
    }

    return return_code;
}