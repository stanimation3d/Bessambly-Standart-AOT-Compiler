#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> // isspace, isalnum vb. için
#include "utils.h"
#include "error.h" // Hata raporlama için

// Bellek Yönetimi İşlevleri

void *safe_malloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        // Bellek tahsisi başarısız olursa kritik hata raporla ve çık.
        report_error(ERR_OUT_OF_MEMORY, 0, "Gereken bellek tahsis edilemedi.");
    }
    return ptr;
}

void *safe_realloc(void *ptr, size_t size) {
    void *new_ptr = realloc(ptr, size);
    if (new_ptr == NULL) {
        // Bellek yeniden tahsisi başarısız olursa kritik hata raporla ve çık.
        // Orijinal ptr hala geçerlidir, ancak programı durdurmak en güvenlisidir.
        report_error(ERR_OUT_OF_MEMORY, 0, "Bellek yeniden tahsis edilemedi.");
    }
    return new_ptr;
}


// Dize (String) İşlevleri

bool is_whitespace(char c) {
    // isspace() fonksiyonu, boşluk, sekme, yeni satır vb. kontrol eder.
    return (bool)isspace((unsigned char)c);
}

bool is_valid_start_char(char c) {
    // Bessambly'de tanımlayıcılar (etiketler/kayıtlar) harfle veya '_' ile başlamalıdır.
    // Bessambly'nin basit yapısı gereği tek harfli kayıtlar (A, B, C...) de bu kurala uyar.
    return (bool)isalpha((unsigned char)c) || c == '_';
}

bool is_valid_char(char c) {
    // Bessambly'de tanımlayıcılar harf, rakam veya '_' içerebilir.
    return (bool)isalnum((unsigned char)c) || c == '_';
}