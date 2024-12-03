#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "my_getline.h"

// 入力データとその管理情報を保持する構造体
typedef struct {
    char* buffer;           // 読み込んだデータ（文字列）を格納するためのポインタ（動的に割り当てられる）
    size_t buffer_length;   // バッファのサイズ（現在確保されているメモリ量 - byte）
    ssize_t input_length;   // 実際に読み込まれた文字列の長さ（改行文字を除く）
} InputBuffer;

// InputBufferの初期化
InputBuffer* new_input_buffer() {
    InputBuffer* input_buffer = (InputBuffer*)malloc(sizeof(InputBuffer));
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;

    return input_buffer;
}

void print_prompt() {
    printf("db > ");
}

/**
 * read_input - 標準入力から1行を読み込み、InputBuffer構造体に格納する関数
 *
 * @input_buffer: 入力データを格納するためのInputBuffer構造体
 *
 * 説明:
 * この関数は標準入力から1行を読み込み、その内容をInputBuffer構造体に格納する。
 * 読み込んだ文字列はバッファに格納される。
 * 読み込みが正常でない場合（エラーまたはEOF）は、プログラムが終了。
 */
void read_input(InputBuffer* input_buffer) {
    // my_getline関数を呼び出して、標準入力から1行を読み込む
    ssize_t bytes_read = my_getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);

    // 入力読み込みエラーまたはEOFの場合の処理
    if (bytes_read <= 0) {
        printf("Error reading input\n");  // エラーメッセージを表示
        exit(EXIT_FAILURE);              // プログラムを終了
    }
}

// InputBuffer 構造体のメモリを解放する
void close_input_buffer(InputBuffer* input_buffer) {
    free(input_buffer->buffer);
    free(input_buffer);
}

int main(int argc, char* argv[]) {
    InputBuffer* input_buffer =  new_input_buffer();
    while (1) {
        print_prompt();
        read_input(input_buffer);

        if(strcmp(input_buffer->buffer, ".exit") == 0) {
            close_input_buffer(input_buffer);
            exit(EXIT_SUCCESS);
        } else {
            printf("Unrecognized command '%s'.\n", input_buffer->buffer);
        }
    }
}