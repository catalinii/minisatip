#include "utils/uuid.h"
#include <stdlib.h>
#include <time.h>

// https://stackoverflow.com/a/71826534
void uuid4_generate(char *uuid) {
    srand(time(NULL));

    char v[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    for (int i = 0; i < 36; ++i) {
        uuid[i] = v[rand() % 16];
    }

    uuid[8] = '-';
    uuid[13] = '-';
    uuid[18] = '-';
    uuid[23] = '-';
    uuid[36] = '\0';
}
