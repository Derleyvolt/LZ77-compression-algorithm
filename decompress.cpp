#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef unsigned char byte;

typedef struct tuple {
    unsigned short backward;
    byte len;
    byte symbol;
} tuple;

typedef struct decompressed_bytes {
    byte* buf;
    int   capacity;
    int   len;
} decompressed_bytes;

decompressed_bytes inicialize(int capacity) {
    decompressed_bytes aux;
    aux.buf       = (byte*)malloc(sizeof(byte) * capacity);
    aux.len       = 0;
    aux.capacity  = capacity;
    return aux;
}

void insert(decompressed_bytes* arr, byte data) {
    if(arr->len < arr->capacity) {
        arr->buf[arr->len] = data;
    } else {
        // double capacity
        byte* resized_buf = (byte*)malloc(sizeof(byte) * arr->capacity * 2);

        // copy data to new buff
        for(int i = 0; i < arr->capacity; i++) {
            resized_buf[i] = arr->buf[i];
        }

        // free old data buf
        free(arr->buf);

        // updating new capacity
        arr->capacity = arr->capacity * 2;
        arr->buf      = resized_buf;
    }

    arr->len++;
}

void write_binary(byte* buf, int len, const char* out);

// len is a multiple of 4
void decompress(byte* buf, int len, const char* out) {
    decompressed_bytes arr;
    arr = inicialize(len * 10);
    tuple aux;
    for(int i = 0; i < len; i += 4) {
        memcpy(&aux, buf+i, sizeof(tuple));
        if(aux.len > 100) {
            printf("tamanho: %d\n", aux.len);
        }
        if(aux.backward == 0) {
            insert(&arr, aux.symbol);
        } else {
            int ax = arr.len;
            for(int i = 0; i < aux.len; i++) {
                insert(&arr, arr.buf[ax-1-(aux.backward-1)+i]);
            }

            //if(!(i+4 >= len && aux.len > 0))
            insert(&arr, aux.symbol);
        }
    }

    write_binary(arr.buf, arr.len, out);
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

void write_binary(byte* buf, int len, const char* out) {
    FILE* fp = fopen(out, "wb");

    if(fp == NULL) {
        printf("Erro na alocacao de memoria\n");
        return;
    }

    fwrite(buf, len, 1, fp);
    fclose(fp);
}

int main() {
    byte* buf;
    int len = read_binary(&buf, "saida.out");

    decompress(buf, len, "decompressed.csv");
    return 0;
}