#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "pool.h"

#define MAX_COMMANDS 256
#define MAX_LENGTH 256
#define COMMAND_DELIM ";\n"
#define WORD_DELIM " \n"


#define APPS_X(X) \
        X(echo) \
        X(retcode) \
        X(pooltest) \


#define DECLARE(X) static int X(int, char *[]);
APPS_X(DECLARE)
#undef DECLARE

static const struct app {
    const char *name;
    int (*fn)(int, char *[]);
} app_list[] = {
#define ELEM(X) { # X, X },
        APPS_X(ELEM)
#undef ELEM
};

int echo(int argc, char *argv[]) {
    for (int i = 1; i < argc; ++i) {
        printf("%s%c", argv[i], i == argc - 1 ? '\n' : ' ');
    }
    return argc - 1;
}

int retcode(int argc, char *argv[]) {
    printf("%s\n", argv[1]);
    return 0;
}

static int pooltest(int argc, char *argv[]) {
    struct obj {
        void *field1;
        void *field2;
    };
    static struct obj objmem[4];
    static struct pool objpool = POOL_INITIALIZER_ARRAY(objmem);

    if (!strcmp(argv[1], "alloc")) {
        struct obj *o = pool_alloc(&objpool);
        printf("alloc %ld\n", o ? (o - objmem) : -1);
        return 0;
    } else if (!strcmp(argv[1], "free")) {
        int iobj = atoi(argv[2]);
        printf("free %d\n", iobj);
        pool_free(&objpool, objmem + iobj);
        return 0;
    }
}

int cmd() {
    int return_code = 0;
    while(1) {
        char string[MAX_LENGTH] = "";
        if (fgets(string, MAX_LENGTH, stdin) == NULL) {
            break;
        }
        char * commands[MAX_COMMANDS];
        char * command = strtok(string, COMMAND_DELIM);
        int count_of_commands = 0;
        while(command != NULL) {
            commands[count_of_commands++] = command;
            command = strtok(NULL, COMMAND_DELIM);
        }
        for (int j = 0; j < count_of_commands; j++) {
            char * words[MAX_COMMANDS];
            char * word = strtok(commands[j], WORD_DELIM);
            int count_of_words = 0;
            while (word != NULL) {
                words[count_of_words++] = word;
                word = strtok(NULL, WORD_DELIM);
            }
            if (strcmp(words[0], "echo") == 0) {
                return_code = echo(count_of_words, words);
            } else if (strcmp(words[0], "pooltest") == 0) {
                return_code = pooltest(count_of_words, words);
            }

            else if (strcmp(words[0], "retcode") == 0) {
                char * argv[128];
                char buffer[MAX_LENGTH];
                sprintf(buffer, "%d", return_code);
                argv[0] = words[0];
                argv[1] = buffer;
                return_code = retcode(2, argv);
            } else {
                return_code = -1;
                printf("Unknown command '%s'\n", words[0]);
            }
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    cmd();
    return 0;
}
