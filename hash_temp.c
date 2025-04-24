#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

void hash_function(const char *filepath, const char *cmdname) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int length = 0;
    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        perror("Erreur ouverture du fichier à hasher");
        return;
    }

    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        fclose(fp);
        fprintf(stderr, "Erreur EVP_MD_CTX_new\n");
        return;
    }

    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) != 1) {
        EVP_MD_CTX_free(mdctx);
        fclose(fp);
        fprintf(stderr, "Erreur EVP_DigestInit_ex\n");
        return;
    }

    char buffer[1024];
    size_t read_bytes;
    while ((read_bytes = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        if (EVP_DigestUpdate(mdctx, buffer, read_bytes) != 1) {
            EVP_MD_CTX_free(mdctx);
            fclose(fp);
            fprintf(stderr, "Erreur EVP_DigestUpdate\n");
            return;
        }
    }

    if (EVP_DigestFinal_ex(mdctx, hash, &length) != 1) {
        EVP_MD_CTX_free(mdctx);
        fclose(fp);
        fprintf(stderr, "Erreur EVP_DigestFinal_ex\n");
        return;
    }

    fclose(fp);
    EVP_MD_CTX_free(mdctx);

    char filename[256];
    snprintf(filename, sizeof(filename), "/home/%s/.hash/temp/%s", getenv("USER"), cmdname);

    FILE *hash_file = fopen(filename, "w");
    if (!hash_file) {
        perror("Erreur création du fichier hash");
        return;
    }

    for (unsigned int i = 0; i < length; i++) {
        fprintf(hash_file, "%02x", hash[i]);
    }
    fprintf(hash_file, "\n");
    fclose(hash_file);

    printf("[+] Hash de %s écrit dans %s\n", cmdname, filename);
}

char *find_command_path(const char *command) {
    char *path = getenv("PATH");
    if (!path) return NULL;

    char *paths = strdup(path);
    char *dir = strtok(paths, ":");
    static char fullpath[512];

    while (dir) {
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, command);
        if (access(fullpath, X_OK) == 0) {
            free(paths);
            return fullpath;
        }
        dir = strtok(NULL, ":");
    }

    free(paths);
    return NULL;
}

int main() {
    char *user = getenv("USER");
    if (!user) {
        fprintf(stderr, "Erreur : variable d'environnement USER non définie\n");
        return 1;
    }

    char home_hash_path[512];
    snprintf(home_hash_path, sizeof(home_hash_path), "/home/%s/.hash", user);

    struct stat st = {0};
    if (stat(home_hash_path, &st) == -1) {
        if (mkdir(home_hash_path, 0700) == 0) {
            printf("[+] Dossier .hash créé pour l'utilisateur %s\n", user);
        } else {
            perror("Erreur mkdir .hash");
            return 1;
        }
    }

    char temp_hash_path[512];
    snprintf(temp_hash_path, sizeof(temp_hash_path), "/home/%s/.hash/temp", user);

    struct stat st_temp = {0};
    if (stat(temp_hash_path, &st_temp) == -1) {
        if (mkdir(temp_hash_path, 0700) == 0) {
            printf("[+] Dossier .hash/temp créé pour l'utilisateur %s\n", user);
        } else {
            perror("Erreur mkdir .hash/temp");
            return 1;
        }
    }

    char *home = getenv("HOME");
    if (!home) {
        fprintf(stderr, "Erreur : $HOME non défini\n");
        return 1;
    }

    char whitelist_path[512];
    snprintf(whitelist_path, sizeof(whitelist_path), "%s/.whitelist.txt", home);

    FILE *whitelist = fopen(whitelist_path, "r");
    if (!whitelist) {
        perror("Erreur ouverture ~/.whitelist.txt");
        return 1;
    }

    char line[256];
    while (fgets(line, sizeof(line), whitelist)) {
        line[strcspn(line, "\n")] = 0;
        if (strlen(line) == 0) continue;

        char *command_path = find_command_path(line);
        if (command_path) {
            printf("[+] %s trouvé à %s\n", line, command_path);
            hash_function(command_path, line);
        } else {
            printf("[-] Commande introuvable : %s\n", line);
        }
    }

    fclose(whitelist);
    return 0;
}
