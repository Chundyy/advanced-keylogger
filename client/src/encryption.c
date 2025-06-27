#include "encryption.h"
#include <wincrypt.h>
#include <stdio.h>
#include <string.h>

#define KEY_LENGTH 32

void encrypt_data(const char* input, char* output, size_t len) {
    HCRYPTPROV hProv;
    HCRYPTHASH hHash;
    HCRYPTKEY hKey;
    
    // Obter contexto criptográfico
    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        printf("[!] Erro ao obter contexto criptográfico\n");
        return;
    }

    // Criar hash SHA-256
    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
        printf("[!] Erro ao criar hash\n");
        CryptReleaseContext(hProv, 0);
        return;
    }

    // Hash da chave
    if (!CryptHashData(hHash, (BYTE*)ENCRYPTION_KEY, strlen(ENCRYPTION_KEY), 0)) {
        printf("[!] Erro ao processar hash da chave\n");
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return;
    }

    // Derivar chave AES
    if (!CryptDeriveKey(hProv, CALG_AES_256, hHash, 0, &hKey)) {
        printf("[!] Erro ao derivar chave\n");
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return;
    }

    // Criptografar dados
    strncpy(output, input, len);
    DWORD dwLen = (DWORD)strlen(output);
    
    if (!CryptEncrypt(hKey, 0, TRUE, 0, (BYTE*)output, &dwLen, (DWORD)len)) {
        printf("[!] Erro na criptografia\n");
    }

    // Limpeza
    CryptDestroyKey(hKey);
    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
}
