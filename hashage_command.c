#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

void hash_function(const char *filepath, const char *output_file) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int length = 0;
    FILE *fp = fopen(filepath, "rb");
    if (!fp) return;

    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        fclose(fp);
        return;
    }

    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) != 1) {
        EVP_MD_CTX_free(mdctx);
        fclose(fp);
        return;
    }

    char buffer[1024];
    size_t read_bytes;
    while ((read_bytes = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        if (EVP_DigestUpdate(mdctx, buffer, read_bytes) != 1) {
            EVP_MD_CTX_free(mdctx);
            fclose(fp);
            return;
        }
    }

    if (EVP_DigestFinal_ex(mdctx, hash, &length) != 1) {
        EVP_MD_CTX_free(mdctx);
        fclose(fp);
        return;
    }

    remove(output_file);

    FILE *hash_file = fopen(output_file, "w");
    if (hash_file) {
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
    struct stat st = {0};
    if (stat(".hash", &st) == -1) {
        mkdir(".hash", 0700);
    }

    char *path = getenv("PATH");
    if (!path) return 1;

    char *paths = strdup(path);
    char *dir = strtok(paths, ":");

    while (dir) {
        DIR *dp = opendir(dir);
        if (dp) {
            struct dirent *entry;
            while ((entry = readdir(dp)) != NULL) {
                if (entry->d_type == DT_REG || entry->d_type == DT_LNK || entry->d_type == DT_UNKNOWN) {
                    char fullpath[512];
                    snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, entry->d_name);

                    char output_file[512];
                    snprintf(output_file, sizeof(output_file), ".hash/%s_hash.txt", entry->d_name);

                    hash_function(fullpath, output_file);
                }
            }
            closedir(dp);
        }
        dir = strtok(NULL, ":");
    }

    free(paths);
    return 0;
}
