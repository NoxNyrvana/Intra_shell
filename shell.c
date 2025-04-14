#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>
#include <time.h>
#include <openssl/evp.h>


#define SHELL_TOK_BUFSIZE 64
#define SHELL_TOK_DELIM " \t\r\n\a"
#define HISTORY_FILE ".history.txt"
#define WHITE_FILE ".whitelist.txt"
#define ALIAS_FILE ".alias.txt"
#define RESET   "\033[0m"
#define GREEN   "\033[1;32m"
#define MAX_ALIASES 100
#define MAX_LINE 1024

typedef struct {
    char name[256];
    char command[767];
} Alias;

Alias aliases[MAX_ALIASES];
int alias_count = 0;

int shell_cd(char **args);
int shell_exit(char **args);
int shell_help(char **args);
int shell_ls(char **args);
int shell_pwd(char **args);
int shell_mkdir(char **args);
int shell_cat(char **args);
int shell_rm(char **args);
int shell_history(char **args);
int shell_alias(char **args);
int shell_unalias(char **args);
int shell_fdump(char **args);
int shell_rmdir(char **args);
int shell_touch(char **args);
int shell_echo(char **args);
int shell_wait(char **args);
int shell_ls_a(char **args);
int shell_mv(char **args);
int shell_cp(char **args);

char *builtin_str[] = {
  "cd",
  "exit",
  "help",
  "ls",
  "pwd",
  "mkdir",
  "cat",
  "rm",
  "history",
  "alias",
  "unalias",
  "fdump",
  "rmdir",
  "touch",
  "echo",
  "wait",
  "ls -a",
  "mv",
  "cp"
};

int (*builtin_func[]) (char **) = {
  &shell_cd,
  &shell_exit,
  &shell_help,
  &shell_ls,
  &shell_pwd,
  &shell_mkdir,
  &shell_cat,
  &shell_rm,
  &shell_history,
  &shell_alias,
  &shell_unalias,
  &shell_fdump,
  &shell_rmdir,
  &shell_touch,
  &shell_echo,
  &shell_wait,
  &shell_mv,
  &shell_cp
};

int shell_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

int shell_mv(char **args)
{

    if (args[1] == NULL || args[2] == NULL) {
        fprintf(stderr, "usage: mv <source> <destination>\n");
        return 1;
    }

    char *source = args[1];
    char *destination = args[2];

    FILE *source_file = fopen(source, "r");
    if (source_file == NULL) {
        perror("mv: source file not found");
        return 1;
    }
    fclose(source_file);

    FILE *dest_file = fopen(destination, "w");
    if (dest_file == NULL) {
        perror("mv: unable to create destination file");
        return 1;
    }
    fclose(dest_file);

    char buffer[1024];
    size_t bytes_read;
    source_file = fopen(source, "r");
    dest_file = fopen(destination, "w");
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), source_file)) > 0) {
        fwrite(buffer, 1, bytes_read, dest_file);
    }
    fclose(source_file);
    fclose(dest_file);

    if (remove(source) != 0) {
        perror("mv: unable to remove source file");
        return 1;
    }

    return 1;
}

int shell_cp(char **args)
{

    if (args[1] == NULL || args[2] == NULL) {
        fprintf(stderr, "usage: cp <source> <destination>\n");
        return 1;
    }

    char *source = args[1];
    char *destination = args[2];

    FILE *source_file = fopen(source, "r");
    if (source_file == NULL) {
        perror("cp: source file not found");
        return 1;
    }
    FILE *dest_file = fopen(destination, "w");
    if (dest_file == NULL) {
        perror("cp: unable to create destination file");
        return 1;
    }

    char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), source_file)) > 0) {
        fwrite(buffer, 1, bytes_read, dest_file);
    }
    fclose(source_file);
    fclose(dest_file);

    return 1;
}


int shell_rm(char **args)
{

  if (args[1] == NULL) {
    fprintf(stderr, "rien");
  } else {
    if (remove(args[1]) != 0) {
      perror("rm");
    }
  }
  return 1;
}
int shell_touch(char **args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "usage: touch <fichier>\n");
        return 1;
    }

    FILE *file = fopen(args[1], "a");
    if (file == NULL) {
        perror("touch");
        return 1;
    }
    fclose(file);
    return 1;
}


int shell_fdump(char **args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "usage: fdump <fichier>\n");
        return 1;
    }

    FILE *file = fopen(args[1], "r");
    if (file == NULL) {
        perror("erreur ouverture fichier");
        return 1;
    }

    int c, offset = 0;
    while ((c = fgetc(file)) != EOF) {
        printf("%02X ", c);
        offset++;
        if (offset % 8 == 0) {
            printf(" %01X\n", offset);
        }
    }
    if (offset % 8 != 0) {
        while (offset % 8 != 0) {
            printf("   ");
            offset++;
        }
        printf(" %03X\n", offset);
    }

    fclose(file);
    return 1;
}

int shell_rmdir(char **args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "usage: rmdir <dossier>\n");
        return 1;
    }

    if (rmdir(args[1]) != 0) {
        perror("rmdir");
    }
    return 1;
}

int shell_unalias(char **args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "usage: unalias nom\n");
        return 1;
    }

    FILE *file = fopen(ALIAS_FILE, "r");
    if (file == NULL) {
        perror("erreur d'ouverture du fichier alias");
        return 1;
    }

    char temp_file[256];
    sprintf(temp_file, "%s.tmp", ALIAS_FILE);
    FILE *temp = fopen(temp_file, "w");
    if (temp == NULL) {
        perror("erreur d'ouverture du fichier temporaire");
        fclose(file);
        return 1;
    }

    char line[1024];
    char alias_name[256];
    while (fgets(line, sizeof(line), file) != NULL) {
        if (sscanf(line, "%255s", alias_name) == 1) {
            if (strcmp(alias_name, args[1]) != 0) {
                fprintf(temp, "%s", line);
            }
        }
    }

    fclose(file);
    fclose(temp);

    if (remove(ALIAS_FILE) != 0) {
        perror("erreur de suppression du fichier alias");
        remove(temp_file);
        return 1;
    }

    if (rename(temp_file, ALIAS_FILE) != 0) {
        perror("erreur de renommage du fichier temporaire");
        return 1;
    }

    printf("Alias supprimé : %s\n", args[1]);
    return 1;
}

int shell_alias(char **args)
{
    if (args[1] == NULL) {
        FILE *file = fopen(ALIAS_FILE, "r");
        if (file == NULL) {
            perror("erreur d'ouverture du fichier alias");
            return 1;
        }

        char line[1024];
        while (fgets(line, sizeof(line), file) != NULL) {
            printf("%s", line);
        }
        fclose(file);
    } else {
        char alias_name[256], alias_command[768];
        if (sscanf(args[1], "%255[^=]='%767[^']'", alias_name, alias_command) != 2) {
            fprintf(stderr, "usage: alias nom='commande'\n");
            return 1;
        }

        FILE *file = fopen(ALIAS_FILE, "a");
        if (file == NULL) {
            perror("erreur ouverture fichier alias");
            return 1;
        }

        fprintf(file, "%s %s\n", alias_name, alias_command);
        fflush(file); 
        fclose(file);

        printf("Alias ajouté : %s -> %s\n", alias_name, alias_command);
    }
    return 1;
}

void check_alias_file() {
    FILE *file = fopen(ALIAS_FILE, "r");
    if (file == NULL) {
        file = fopen(ALIAS_FILE, "w");
        if (file == NULL) {
            perror("erreur lors de la création du fichier alias");
            exit(EXIT_FAILURE);
        }
        fclose(file);
    } else {
        fclose(file);
    }
}

int shell_wait(char **args)
{
    int status;
    if (wait(&status) == -1) {
        perror("wait");
        return 1;
    }
    return 1;
}

void load_aliases() {
    alias_count = 0;
    FILE *file = fopen(ALIAS_FILE, "r");
    if (file == NULL) return;
    while (fscanf(file, "%255s %767[^\n]", aliases[alias_count].name, aliases[alias_count].command) == 2) {
        alias_count++;
        if (alias_count >= MAX_ALIASES) break;
    }
    fclose(file);
}

char* get_alias_command(char *input) {
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(input, aliases[i].name) == 0) {
            return aliases[i].command;
        }
    }
    return NULL;
}

void add_to_history(const char *command) {
    FILE *history_file = fopen(HISTORY_FILE, "a");
    if (history_file == NULL) {
        perror("erreur d'ouverture du fichier d'historique");
        return;
    }

    time_t now = time(0);
    struct tm *local = localtime(&now);
    char time_str[10];
    strftime(time_str, sizeof(time_str), "%H:%M:%S", local);
    char date_str[11];
    strftime(date_str, sizeof(date_str), "%Y-%m-%d", local);
    char *user = getenv("USER");

    fprintf(history_file, "%s %s %s %s\n", date_str, time_str, user, command);
    fclose(history_file);
}

int shell_history(char **args) {
    FILE *history_file = fopen(HISTORY_FILE, "r");
    if (history_file == NULL) {
        perror("erreur d'ouverture du fichier d'historique");
        return 1;
    }

    char line[1024];
    int line_number = 1;
    while (fgets(line, sizeof(line), history_file)) {
        printf("%d %s", line_number++, line);
    }

    fclose(history_file);
    return 1;
}



void sigint_handler(int sig) {
    printf("\n");
    char *user = getenv("USER");
    printf(GREEN "%s_> " RESET, user);
    fflush(stdout);
}

int shell_cat(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "usage: cat <fichier>\n");
        return 1;
    }

    FILE *file = fopen(args[1], "r");
    if (file == NULL) {
        perror("erreur ouverture fichier");
        return 1;
    }

    char line[1024];
    while (fgets(line, sizeof(line), file) != NULL) {
        printf("%s", line);
    }
    printf("\n");
    fclose(file);
    return 1;
}

int shell_pwd(char **args)
{
  char cwd[1024];
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    printf("%s\n", cwd);
  }
}

int shell_mkdir(char **args)
{

  if (args[1] == NULL) {
    fprintf(stderr, "rien");
  } else {
    if (mkdir(args[1], 0777) != 0) {
      perror("mkdir");
    }
  }
  return 1;
}

int shell_ls(char **args) {
    DIR *dir;
    struct dirent *entry;
    char *directory = (args[1] != NULL) ? args[1] : ".";

    dir = opendir(directory);
    if (dir == NULL) {
        perror("ls");
        return 1;
    }

    if (args[2] != NULL && strcmp(args[2], "-a") == 0) {
        while ((entry = readdir(dir)) != NULL) {
            printf("%s\n", entry->d_name);
        }
    } else {
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] != '.') { 
                printf("%s\n", entry->d_name);
            }
        }
    }

    closedir(dir);
    return 1;
}

int shell_ls_a(char **args) {
    DIR *dir;
    struct dirent *entry;
    char *directory = (args[1] != NULL) ? args[1] : ".";

    dir = opendir(directory);
    if (dir == NULL) {
        perror("ls");
        return 1;
    }

    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n", entry->d_name);
    }

    closedir(dir);
    return 1;
}


int shell_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "rien");
  } else {
    if (chdir(args[1]) != 0) {
      perror("1");
    }
  }
  return 1;
}

int shell_help(char **args)
{
  FILE *white_file = fopen(WHITE_FILE, "r");
  int i;
  for (i = 0; i < shell_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }
  char line[1024];
    int line_number = 1;
    printf(" \n ___________\n WHITELIST \n ___________\n\n");
    while (fgets(line, sizeof(line), white_file)) {
        printf("  %s",line);
    }
  return 1;
}

int shell_exit(char **args)
{
  return 0;
}

int shell_echo(char **args)
{
    if (args[1] == NULL) {
        printf("\n");
    } else {
        for (int i = 1; args[i] != NULL; i++) {
            if (strcmp(args[i], "$") == 0 && args[i + 1] != NULL) {
                if (strcmp(args[i + 1], "Date") == 0) {
                    time_t now = time(0);
                    char *dt = ctime(&now);
                    printf("%s ", dt);
                    i++;
                } else if (strcmp(args[i + 1], "Hour") == 0) {
                    time_t now = time(0);
                    struct tm *local = localtime(&now);
                    char time_str[10];
                    strftime(time_str, sizeof(time_str), "%H:%M:%S", local);
                    printf("%s ", time_str);
                    i++;
                } else {
                    char *env_var = getenv(args[i + 1]);
                    if (env_var != NULL) {
                        printf("%s ", env_var);
                        i++;
                    } else {
                        printf("$%s ", args[i + 1]);
                        i++;
                    }
                }
            } else {
                printf("%s ", args[i]);
            }
        }
        printf("\n");
    }
    return 1;
}

int shell_launch(char **args)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("shell");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("shell");
  } else {
    // Parent process
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}


int compare_hashes(const char *cmd) {
    char ref_path[256], temp_path[256];
    snprintf(ref_path, sizeof(ref_path), ".hash/%s_hash.txt", cmd);
    snprintf(temp_path, sizeof(temp_path), ".hash/temp/%s", cmd);

    FILE *ref = fopen(ref_path, "r");
    FILE *temp = fopen(temp_path, "r");

    if (!ref || !temp) {
        perror("Erreur ouverture fichiers hash");
        if (ref) fclose(ref);
        if (temp) fclose(temp);
        return 0; // échec
    }

    char hash_ref[EVP_MAX_MD_SIZE * 2 + 1];
    char hash_temp[EVP_MAX_MD_SIZE * 2 + 1];

    fgets(hash_ref, sizeof(hash_ref), ref);
    fgets(hash_temp, sizeof(hash_temp), temp);

    fclose(ref);
    fclose(temp);

    // Supprimer \n éventuel
    hash_ref[strcspn(hash_ref, "\n")] = 0;
    hash_temp[strcspn(hash_temp, "\n")] = 0;

    return strcmp(hash_ref, hash_temp) == 0;
}





int shell_execute(char **args)
{
    if (args[0] == NULL) {
        return 1;
    }
    if (strcmp(args[0], "history") != 0) {
        add_to_history(args[0]);
    }
    if (strcmp(args[0], "cd") == 0) {
        return shell_cd(args);
    }
    if (strcmp(args[0], "exit") == 0) {
        return shell_exit(args);
    }
    if (strcmp(args[0], "help") == 0) {
        return shell_help(args);
    }
    if (strcmp(args[0], "ls") == 0) {
        return shell_ls(args);
    }
    if (strcmp(args[0], "pwd") == 0) {
        return shell_pwd(args);
    }
    if (strcmp(args[0], "mkdir") == 0) {
        return shell_mkdir(args);
    }
    if (strcmp(args[0], "cat") == 0) {
        return shell_cat(args);
    }
    if (strcmp(args[0], "rm") == 0) {
        return shell_rm(args);
    }
    if (strcmp(args[0], "history") == 0) {
        return shell_history(args);
    }
    if (strcmp(args[0], "alias") == 0) {
        return shell_alias(args);
    }
    if (strcmp(args[0], "unalias") == 0){
        return shell_unalias(args);
    }
    if (strcmp(args[0], "fdump") == 0) {
        return shell_fdump(args);
    }
    if (strcmp(args[0], "rmdir") == 0) {
        return shell_rmdir(args);
    }
    if (strcmp(args[0], "touch") == 0) {
        return shell_touch(args);
    }
    if (strcmp(args[0], "echo") == 0) {
        return shell_echo(args);
    }
    if (strcmp(args[0], "wait") == 0) {
        return shell_wait(args);
    }
    if (strcmp(args[0], "ls_-a") == 0) {
        return shell_ls_a(args);
    }
    if (strcmp(args[0], "mv") == 0) {
        return shell_mv(args);
    }
    if (strcmp(args[0], "cp") == 0) {
        return shell_cp(args);
    }
    if (args[0] == NULL) return 1;
    
load_aliases();
    char *alias_cmd = get_alias_command(args[0]);
    if (alias_cmd) {
        char *new_args[] = { alias_cmd, NULL };
        return shell_launch(new_args);
    }

    FILE *fichier = fopen(WHITE_FILE, "r");
    if (!fichier) {
        perror("Erreur ouverture whitelist");
        return 1;
    }

    char ligne[MAX_LINE];
    int trouve = 0;

    while (fgets(ligne, MAX_LINE, fichier)) {
        ligne[strcspn(ligne, "\n")] = 0; 
        if (strcmp(ligne, args[0]) == 0) {
            trouve = 1;
            break;
        }
    }
    fclose(fichier);

    if (!trouve) {
        printf("Commande non whitelistée\n");
        return 1;
    }


    pid_t pid = fork();
    if (pid == 0) {
        execlp("gcc", "gcc", "-o", "hash_temp", "hash_temp.c", "-lssl", "-lcrypto", NULL);
        perror("Erreur compilation");
        exit(1);
    }
    int status;
    waitpid(pid, &status, 0);

  
    pid_t pid2 = fork();
    if (pid2 == 0) {
        execl("./hash_temp", "./hash_temp", args[0], NULL);
        perror("Erreur execl");
        exit(1);
    }
    waitpid(pid2, &status, 0);
    
    if (compare_hashes(args[0])) {
    printf("Hash OK, exécution de la commande : %s\n", args[0]);
    return shell_launch(args);
} else {
    printf("Hash non correspondant ou erreur\n");
    return 1;
}
}



char *shell_read_line(void)
{
  char *line = NULL;
  ssize_t bufsize = 0;
  getline(&line, &bufsize, stdin);
  return line;
}


char **shell_split_line(char *line) {
  int bufsize = SHELL_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char *));
  char *token;

  if (!tokens) {
    fprintf(stderr, "erreur d'allocation mémoire\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, SHELL_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += SHELL_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char *));
      if (!tokens) {
        fprintf(stderr, "erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, SHELL_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

/**
   @brief Loop getting input and executing it.
 */
void shell_loop(void)
{
  char *line;
  char **args;
  int status;

  do {
    char *user = getenv("USER");
    printf(GREEN "%s_> " RESET, user);
    line   = shell_read_line();
    args   = shell_split_line(line);
    status = shell_execute(args);

    free(line);
    free(args);
  } while (status);
}

int main(int argc, char **argv)
{
  FILE *file = fopen(ALIAS_FILE, "r");
  if (file == NULL) {
      file = fopen(ALIAS_FILE, "w");
      if (file == NULL) {
          perror("error missing alias file");
          exit(EXIT_FAILURE);
      }
      fclose(file);
  } else {
      fclose(file);
  }
FILE *filew = fopen(WHITE_FILE, "r");
   if (filew == NULL) {
       filew = fopen(WHITE_FILE, "w");
       if (filew == NULL) {
           perror("error missing whitelist file");
           exit(EXIT_FAILURE);
       }
       fclose(filew);
   } else {
       fclose(filew);
   }
  if (remove(HISTORY_FILE) != 0) {
    perror("error history still here");
  }
  FILE *history_file = fopen(HISTORY_FILE, "w");
  if (history_file == NULL) {
    perror("error no history created");
    return EXIT_FAILURE;
  }
  fclose(history_file);
  signal(SIGINT, sigint_handler);
  printf("Mini SHELL - exit pour Quitter \n");
  shell_loop();
  printf("Hasta la vista baby !\n");
  return EXIT_SUCCESS;
}
