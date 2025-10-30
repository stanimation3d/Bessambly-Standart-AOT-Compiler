#ifndef BESSAMBLY_UTILS_H
#define BESSAMBLY_UTILS_H

#include <stddef.h> // size_t için
#include <stdbool.h> // bool için

// Sabitler
#define MAX_LINE_LENGTH 256 // Bessambly kaynak kodunda maksimum satır uzunluğu
#define MAX_LABEL_LENGTH 32 // Etiketler için maksimum isim uzunluğu

// Genel Bellek Yönetimi İşlevleri (Hata Kontrollü)

/**
 * @brief Güvenli bellek tahsisi (malloc). Başarısız olursa hata raporlar ve çıkar.
 * @param size: Tahsis edilecek bayt sayısı.
 * @return void*: Tahsis edilen bellek bloğunun işaretçisi.
 */
void *safe_malloc(size_t size);

/**
 * @brief Güvenli bellek yeniden tahsisi (realloc). Başarısız olursa hata raporlar ve çıkar.
 * @param ptr: Yeniden boyutlandırılacak mevcut bellek işaretçisi.
 * @param size: Yeni bayt sayısı.
 * @return void*: Yeniden tahsis edilen bellek bloğunun işaretçisi.
 */
void *safe_realloc(void *ptr, size_t size);


// Dize (String) İşlevleri

/**
 * @brief Bir karakterin boşluk karakteri olup olmadığını kontrol eder.
 * @param c: Kontrol edilecek karakter.
 * @return true eğer boşluksa (space, tab, newline vb.), false değilse.
 */
bool is_whitespace(char c);

/**
 * @brief Bir karakterin Bessambly'de tanımlayıcı (etiket/kayıt) için geçerli başlangıç karakteri olup olmadığını kontrol eder.
 * @param c: Kontrol edilecek karakter.
 * @return true eğer geçerli başlangıç karakteriyse, false değilse.
 */
bool is_valid_start_char(char c);

/**
 * @brief Bir karakterin Bessambly'de tanımlayıcı (etiket/kayıt) için geçerli bir karakter olup olmadığını kontrol eder.
 * @param c: Kontrol edilecek karakter.
 * @return true eğer geçerli bir tanımlayıcı karakteriyse, false değilse.
 */
bool is_valid_char(char c);

#endif // BESSAMBLY_UTILS_H