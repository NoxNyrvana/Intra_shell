#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void hash_function(const char *input_file, const char *output_file) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int length = 0;
    char command[256];
    FILE *fp;

    snprintf(command, sizeof(command), "cat %s", input_file);
    fp = popen(command, "r");
    if (fp == NULL) return;

    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (mdctx == NULL) return;

    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) != 1) return;

    while (fgets(command, sizeof(command), fp) != NULL) {
        if (EVP_DigestUpdate(mdctx, command, strlen(command)) != 1) return;
    }

    if (EVP_DigestFinal_ex(mdctx, hash, &length) != 1) return;

    remove(output_file);

    FILE *hash_file = fopen(output_file, "a");
    if (hash_file != NULL) {
        for (unsigned int i = 0; i < length; i++) {
            fprintf(hash_file, "%02x", hash[i]);
        }
        fprintf(hash_file, "\n");
        fclose(hash_file);
    }

    EVP_MD_CTX_free(mdctx);
    fclose(fp);
}

int main() {
    char output_file[512] = ".hash/.whitelist_hash.txt";

    struct stat st = {0};
    if (stat(".hash", &st) == -1) {
        mkdir(".hash", 0700);
    }

    hash_function(".whitelist.txt", output_file);

    return 0;
}
