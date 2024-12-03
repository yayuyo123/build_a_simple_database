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

/**
 * メタコマンドはプログラム自体を制御するコマンドでここではドット(.)で始まる
 */
typedef enum {
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

MetaCommandResult do_meta_command(InputBuffer* input_buffer) {
    if(strcmp(input_buffer->buffer, ".exit") == 0) {
        exit(EXIT_SUCCESS);
    } else {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT
} StatementType;

// ステートメント解析の結果を表す列挙型
typedef enum {
    PREPARE_SUCCESS,                // ステートメント解析が成功
    PREPARE_UNRECOGNIZED_STATEMENT  // 認識できないステートメント
} PrepareResult;

typedef struct {
  StatementType type;
} Statement;

/**
 * 入力された文字列を解析し、対応するステートメントを設定する関数
 * 
 * @param input_buffer ユーザー入力が格納されたバッファ
 * @param statement    解析結果を格納するステートメント構造体
 * @return PrepareResult ステートメント解析の結果
 */
PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement) {
    if(strncmp(input_buffer->buffer, "insert", 6) == 0) {
        statement->type = STATEMENT_INSERT; // ステートメントタイプを INSERT に設定
        return PREPARE_SUCCESS; // 解析成功を返す
    }
    if(strcmp(input_buffer->buffer, "select") == 0) {
        statement->type = STATEMENT_SELECT; // ステートメントタイプを SELECT に設定
        return PREPARE_SUCCESS; // 解析成功を返す
    }

    // 上記以外の場合、認識できないステートメントとしてエラーを返す
    return PREPARE_UNRECOGNIZED_STATEMENT;
}

void execute_statement(Statement* statement) {
    switch (statement->type) {
        case (STATEMENT_INSERT):
            printf("This is where we would do an insert.\n");
            break;
        case (STATEMENT_SELECT):
            printf("This is where we would do a select.\n");
            break;
    }
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

        // メタコマンド(ドットで始まるプログラムを制御するコマンド)
        if(input_buffer->buffer[0] == '.') {
            switch (do_meta_command(input_buffer)) {
                case (META_COMMAND_SUCCESS):
                    continue;
                case (META_COMMAND_UNRECOGNIZED_COMMAND):
                    printf("Unrecognized command '%s'\n", input_buffer->buffer);
                    continue;
            }
        }

        // ステートメントの解析結果を保持
        Statement statement;
        // 入力を解析し、結果に応じて処理を分岐
        switch (prepare_statement(input_buffer, &statement)) {
            case (PREPARE_SUCCESS):
                break;
            case (PREPARE_UNRECOGNIZED_STATEMENT):
                printf("Unrecognized keyword at start of'%s'.\n", input_buffer->buffer);
                continue;
        }

        // ステートメントを実行
        execute_statement(&statement);
        printf("Executed.\n");

    }
}