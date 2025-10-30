#ifndef BESSAMBLY_ERROR_H
#define BESSAMBLY_ERROR_H

// Bessambly Derleyicisi Hata Kodları
typedef enum {
    ERR_NONE = 0,               // Hata Yok
    ERR_SYNTAX_INVALID_TOKEN,   // Geçersiz veya Beklenmeyen Belirteç
    ERR_SYNTAX_EXPECTED_ASSIGN, // '=' Atama Operatörü Bekleniyor
    ERR_SYNTAX_INVALID_ADDRESS, // Geçersiz Bellek Adresi Formatı (örn: MEM[yanlis])
    
    ERR_SEMANTIC_UNKNOWN_LABEL, // Tanımlanmamış Etikete Atlama Girişimi (goto/if-goto)
    ERR_SEMANTIC_DUPLICATE_LABEL, // Aynı Etiketin İki Kez Tanımlanması
    ERR_SEMANTIC_UNKNOWN_REGISTER, // Tanımlanmamış Kayıt Kullanımı (A, B, C...)
    
    ERR_FILE_NOT_FOUND,         // Kaynak Dosya Bulunamadı
    ERR_OUT_OF_MEMORY,          // Bellek Tahsisi Başarısız
    
    // Daha sonra eklenecek diğer hatalar
    ERR_COUNT                   // Toplam Hata Sayısı (enum'ın boyutu için)
} BESSAMBLY_ERROR;

/**
 * @brief Bir derleme hatasını rapor eder ve derleme sürecini sonlandırır.
 * * @param error_code: Meydana gelen hatayı belirten BESSAMBLY_ERROR enum değeri.
 * @param line_number: Hatanın meydana geldiği kaynak kod satır numarası (eğer biliniyorsa, bilinmiyorsa 0).
 * @param message: Hata bağlamını açıklayan ek mesaj (örn: beklenen belirteç).
 */
void report_error(BESSAMBLY_ERROR error_code, int line_number, const char *message);

#endif // BESSAMBLY_ERROR_H