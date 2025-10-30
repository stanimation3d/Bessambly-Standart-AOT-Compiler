#include <stdio.h>
#include <stdlib.h>
#include "error.h"

// Hata kodlarına karşılık gelen kullanıcı dostu mesaj dizisi
static const char *error_messages[] = {
    "Başarılı",
    "Sözdizimi Hatası: Geçersiz veya beklenmeyen bir belirteç bulundu.",
    "Sözdizimi Hatası: '=' atama operatörü bekleniyor.",
    "Sözdizimi Hatası: Bellek adresi formatı geçersiz (MEM[adres] beklenen).",
    
    "Anlambilim Hatası: Tanımlanmamış bir etikete atlama girişimi.",
    "Anlambilim Hatası: Aynı etiket birden fazla kez tanımlanmış.",
    "Anlambilim Hatası: Tanımlanmamış bir kayıt (register) kullanılıyor.",
    
    "Sistem Hatası: Kaynak dosya bulunamadı.",
    "Sistem Hatası: Bellek tahsisi başarısız (Out of Memory).",
    
    "Bilinmeyen Hata Kodu" // ERR_COUNT'a karşılık gelir (Hata kodu sınır dışı ise)
};

void report_error(BESSAMBLY_ERROR error_code, int line_number, const char *message) {
    // Hata kodunun geçerli aralıkta olup olmadığını kontrol et
    if (error_code >= ERR_COUNT) {
        error_code = ERR_COUNT; // 'Bilinmeyen Hata Kodu'nu kullan
    }
    
    const char *error_desc = error_messages[error_code];
    
    // Hata mesajını biçimlendir ve çıktı al
    if (line_number > 0) {
        fprintf(stderr, "\n[HATA] Satır %d: %s\n", line_number, error_desc);
    } else {
        fprintf(stderr, "\n[HATA] %s\n", error_desc);
    }
    
    // Eğer ek bir mesaj varsa onu da yazdır
    if (message != NULL && message[0] != '\0') {
        fprintf(stderr, " -> Detay: %s\n", message);
    }
    
    // Derleyicinin çalışmasını sonlandır (Kritik Hata)
    exit(EXIT_FAILURE);
}