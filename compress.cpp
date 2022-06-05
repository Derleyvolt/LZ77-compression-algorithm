#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define min(a, b) (a < b ? a : b)

const int search_buf    = 1024;
const int lookahead_buf = 100;

typedef struct tuple {
    unsigned short backward;
    unsigned char  len;
    unsigned char  symbol;
} tuple;

typedef struct dynamic_list {
    tuple* buf;
    int    capacity;
    int    len;
} dynamic_list;

dynamic_list inicialize(int capacity) {
    dynamic_list aux;

    aux.capacity  = capacity;
    aux.buf       = (tuple*)malloc(sizeof(tuple) * capacity);

    if(aux.buf == NULL) {
        printf("Erro de alocacao\n");
        exit(0);
    }

    aux.len = 0;
    return aux;
}

tuple make_tuple(int arg1, int arg2, unsigned char arg3) {
    tuple ret;
    ret.backward = arg1;
    ret.len      = arg2;
    ret.symbol        = arg3;
    return ret;
}

void insert(dynamic_list* list, tuple arg) {
    if(list->len < list->capacity) {
        list->buf[list->len] = arg;
    } else {
        // double capacity
        tuple* resized_buf = (tuple*)malloc(sizeof(tuple) * list->capacity * 2);

        // copy data to new buff
        for(int i = 0; i < list->capacity; i++) {
            resized_buf[i] = list->buf[i];
        }

        // free old data buf
        free(list->buf);

        // updating new capacity
        list->capacity = list->capacity * 2;
        list->buf      = resized_buf;
    }

    list->len++;
}

int traverse_list(dynamic_list list) {
    int cnt_bytes = 0;
    for(int i = 0; i < list.len; i++) {
        printf("(%d, %d, %c) ", list.buf[i].backward, list.buf[i].len, list.buf[i].symbol);
        cnt_bytes += 4;
    }
    printf("\n");

    return cnt_bytes;
}

dynamic_list compression(unsigned char buf[], int len) {
    dynamic_list list;

    list = inicialize(1024);

    insert(&list, make_tuple(0, 0, buf[0]));

    for(int i = 1; i < len; i++) {
        tuple temp = make_tuple(0, 0, buf[i]);
        for(int j = i-1; j >= 0 && j >= i-search_buf; j--) {
            int cx = j;
            for(int p = i; p < min(i+lookahead_buf, len) && cx < i; p++) {
                if(buf[cx] == buf[p]) {
                    if(temp.len < p-i+1) {
                        temp = make_tuple(i-j, p-i+1, buf[p+1 == len ? p : p+1]);
                    }
                    cx++;
                } else {
                    break;
                }
            }
        }

        insert(&list, temp);
        i += temp.len ? temp.len : 0;
    }

    return list;
}

int read_binary(unsigned char** buf, const char* in) {
    FILE* fp = fopen(in, "rb");

    if(fp == NULL) {
        printf("Erro na alocacao de memoria\n");
        return -1;
    }

    fseek(fp, 0L, SEEK_END);
    int len = ftell(fp);
    rewind(fp);

    *buf = (unsigned char*)malloc(sizeof(unsigned char) * len);

    fread(*buf, len, 1, fp);
    fclose(fp);
    return len;
}

void write_binary(dynamic_list list, const char* out) {
    FILE* fp = fopen(out, "wb");

    if(fp == NULL) {
        printf("Erro na alocacao de memoria\n");
        return;
    }

    fwrite(list.buf, sizeof(tuple) * list.len, 1, fp);
    fclose(fp);
}

int main() {
    unsigned char* buf;

    int n = read_binary(&buf, "vinho.csv");

    printf("%d\n", n);

    dynamic_list list = compression(buf, n);
    int bt = traverse_list(list);

    write_binary(list, "saida.out");

    printf("%d compressed bytes vs %d uncompressed bytes\n", bt, n);
    return 0;
}

// a b a b c b a b a b a a b

// (0, 0, 'a'), (0, 0, 'b'), (2, 2, '*'), (0, 0, 'c'), (4, 3, '*'), (8, 3, '*'), (1, 1, '*')
// (0, 0, 'a'), (0, 0, 'b'), (2, 2, 'c'), (4, 3, 'a'), (2, 2, 'a')