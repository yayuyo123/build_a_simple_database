#include <stdio.h>
#include <stdlib.h>
#include "my_getline.h"

/**
 * my_getline - 標準入力または指定されたファイルストリームから1行を読み込む関数
 *
 * @buffer: 読み込んだ文字列を格納するためのポインタ。関数内で必要に応じてメモリが再割り当てされる。
 * @buffer_length: 読み込むために確保されたバッファのサイズ。読み込み中にバッファが不足すると、拡張される。
 * @stream: 入力元となるファイルストリーム（標準入力やファイルなど）。
 *
 * 戻り値:
 *  - 読み込んだ文字数（改行、終端文字を含まない）。エラーまたはEOFの場合は-1を返す。
 *
 * 説明:
 * この関数は、指定されたストリームから1行を読み込み、改行文字（'\n'）またはEOFに達するまで
 * 文字をバッファに格納します。バッファが不足すると自動的に拡張されます。
 * 入力が空の場合やEOFの場合、適切な処理を行い、-1を返します。
 */
ssize_t my_getline(char** buffer, size_t* buffer_length, FILE* stream) {
    if (buffer == NULL || buffer_length == NULL || stream == NULL) {
        return -1; // 無効な引数
    }

    size_t chunk_size = 128;  // バッファ拡張の単位
    size_t current_length = 0;

    // 初回割り当て
    if (*buffer == NULL) {
        *buffer = malloc(chunk_size);
        if (*buffer == NULL) {
            return -1; // メモリ確保失敗
        }
        *buffer_length = chunk_size;
    }

    int c;
    while ((c = fgetc(stream)) != '\n' && c != EOF) {
        // バッファが足りない場合は拡張
        if (current_length + 1 >= *buffer_length) {
            size_t new_length = *buffer_length + chunk_size;
            char* new_buffer = realloc(*buffer, new_length);
            if (new_buffer == NULL) {
                return -1; // メモリ再確保失敗
            }
            *buffer = new_buffer;
            *buffer_length = new_length;
        }

        // 入力文字をバッファに格納
        (*buffer)[current_length++] = (char)c;
    }

    // EOF かつ何も読み込んでいない場合は終了
    if (c == EOF && current_length == 0) {
        return -1;
    }

    // 終端文字を追加
    (*buffer)[current_length] = '\0';

    // 読み込んだ文字数を返す
    return current_length;
}
