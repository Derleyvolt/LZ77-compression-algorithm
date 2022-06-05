#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define min(a, b) (a < b ? a : b)

typedef unsigned char byte;

const int search_buf    = 1024;
const int lookahead_buf = 100;

typedef struct tuple {
    unsigned short backward;
    byte len;
    byte symbol;
} tuple;

typedef struct dynamic_list {
    tuple* buf;
    int    capacity;
    int    len;
} dynamic_list;

void free_list(dynamic_list list) {
	free(list.buf);
}

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

tuple make_tuple(unsigned short arg1, byte arg2, byte arg3) {
    tuple ret;
    ret.backward = arg1;
    ret.len      = arg2;
    ret.symbol   = arg3;
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
        list->capacity       = list->capacity * 2;
        list->buf            = resized_buf;
		list->buf[list->len] = arg;
    }

    list->len++;
}

void list_tuples(dynamic_list list) {
    for(int i = 0; i < list.len; i++) {
        printf("(%d, %c, %c) ", list.buf[i].backward, list.buf[i].len, list.buf[i].symbol);
    }
    printf("\n");
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

int read_binary(byte** buf, const char* in) {
    FILE* fp = fopen(in, "rb");

    if(fp == NULL) {
        printf("Erro na alocacao de memoria\n");
        return -1;
    }

    fseek(fp, 0L, SEEK_END);
    int len = ftell(fp);
    rewind(fp);

    *buf = (byte*)malloc(sizeof(byte) * len);

    fread(*buf, len, 1, fp);
    fclose(fp);
    return len;
}

int compression(byte buf[], int len, const char* out) {
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
                        temp = make_tuple(i-j, p-i+1, buf[p+1 >= len ? p : p+1]);
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

    write_binary(list, out);
    free_list(list);
    return list.len * sizeof(tuple);
}

int main() {	
    byte* buf;

    int n = read_binary(&buf, "dataset.csv");

    printf("%d uncompressed bytes\n", n);

    int compressed_len = compression(buf, n, "saida.out");

    double compressed_rate = 100 - (compressed_len/(double)n) * 100;

    printf("%lf percent smaller\n", compressed_rate);
    return 0;
}
