#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "my_getline.h"

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
typedef struct {
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE];
    char email[COLUMN_EMAIL_SIZE];
} Row;

// 構造体rowのデータを表示
void print_row(Row* row) {
    printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}

// 構造体の指定されたフィールドのバイトサイズを求める
#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)

const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

const uint32_t PAGE_SIZE = 4096;
#define TABLE_MAX_PAGES 100
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

typedef struct {
  uint32_t num_rows;
  void* pages[TABLE_MAX_PAGES]; // 各ページの先頭アドレス
} Table;

// テーブルの初期化
Table* new_table() {
    Table* table = (Table*)malloc(sizeof(Table));
    table->num_rows = 0;
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        table->pages[i] = NULL;
    }
    return table;
}

// テーブルの解放
void free_table(Table* table) {
    for(int i = 0; table->pages[i]; i++) {
        free(table->pages[i]);
    }
    free(table);
}

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

// InputBuffer 構造体のメモリを解放する
void close_input_buffer(InputBuffer* input_buffer) {
    free(input_buffer->buffer);
    free(input_buffer);
}

/**
 * メタコマンドはプログラム自体を制御するコマンドでここではドット(.)で始まる
 */
typedef enum {
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

MetaCommandResult do_meta_command(InputBuffer* input_buffer, Table* table) {
    if(strcmp(input_buffer->buffer, ".exit") == 0) {
        close_input_buffer(input_buffer);
        free_table(table);
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
    PREPARE_SYNTAX_ERROR, // 文法的な誤り
    PREPARE_UNRECOGNIZED_STATEMENT  // 認識できないステートメント
} PrepareResult;

typedef struct {
  StatementType type;
  Row row_to_insert;
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
        int args_assigned = sscanf(
            input_buffer->buffer, "insert %d %s %s",
            &(statement->row_to_insert.id), statement->row_to_insert.username, statement->row_to_insert.email
        );
        if(args_assigned < 3) {
            return PREPARE_SYNTAX_ERROR;
        }
        return PREPARE_SUCCESS; // 解析成功を返す
    }
    if(strcmp(input_buffer->buffer, "select") == 0) {
        statement->type = STATEMENT_SELECT; // ステートメントタイプを SELECT に設定
        return PREPARE_SUCCESS; // 解析成功を返す
    }

    // 上記以外の場合、認識できないステートメントとしてエラーを返す
    return PREPARE_UNRECOGNIZED_STATEMENT;
}

/**
 * シリアライズ（直列化）
 * Row構造体のデータを指定されたメモリ領域にコピーして、直列化する。
 * 
 * @param source   シリアライズする元となるRow構造体へのポインタ
 * @param destination  シリアライズされたデータを格納するメモリ領域へのポインタ
 *                    - destinationは、シリアライズされたデータを格納する場所を指すポインタ。 
 *
 * この関数は、Row構造体のフィールドを順番にコピーし、指定されたメモリ領域に格納する。
 * コピー先のメモリ領域は、ID_OFFSET, USERNAME_OFFSET, EMAIL_OFFSETなどのオフセットを使って
 * 各フィールドを配置する。
 */
void serialize_row(Row* source, void* destination) {
    // Row構造体のidフィールドをdestinationのID_OFFSET位置にコピー
    memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
    // Row構造体のusernameフィールドをdestinationのUSERNAME_OFFSET位置にコピー
    memcpy(destination + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
    // Row構造体のemailフィールドをdestinationのEMAIL_OFFSET位置にコピー
    memcpy(destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

/**
 * デシリアライズ（逆）
 */
void deserialize_row(void* source, Row* destination) {
  memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
  memcpy(&(destination->username), source + USERNAME_OFFSET, USERNAME_SIZE);
  memcpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
}

// テーブル内の特定の行にアクセスする関数
void* row_slot(Table* table, uint32_t row_num) {
    // 行番号からページ番号を計算
    uint32_t page_num = row_num / ROWS_PER_PAGE;

    // 指定されたページを取得
    void* page = table->pages[page_num];

    // ページがまだ割り当てられていない場合は、新たに割り当てる
    if (page == NULL) {
        page = table->pages[page_num] = malloc(PAGE_SIZE); // ページサイズ分のメモリを確保
    }

    // ページ内での行のオフセット（ページ内のどこに行が格納されるか）を計算
    uint32_t row_offset = row_num % ROWS_PER_PAGE;
    // 行のバイトオフセットを計算（行のデータがメモリ内でどこに格納されるか）
    uint32_t byte_offset = row_offset * ROW_SIZE;
    // 行のデータが格納されている位置を返す
    return page + byte_offset;
}

typedef enum {
    EXECUTE_SUCCESS,
    EXECUTE_TABLE_FULL
} ExecuteResult;

ExecuteResult execute_insert(Statement* statement, Table* table) {
    // テーブルの行数が最大値に達している場合、挿入できない
    if(table->num_rows >= TABLE_MAX_ROWS) {
        return EXECUTE_TABLE_FULL;
    }

    // 挿入する行のデータを取得
    Row* row_to_insert = &(statement->row_to_insert);

    // 行をシリアライズし、テーブル内の適切な場所に書き込む
    serialize_row(row_to_insert, row_slot(table, table->num_rows));
    // テーブルの行数を1増加
    table->num_rows += 1;

    return EXECUTE_SUCCESS;
}

ExecuteResult execute_select(Statement* statement, Table* table) {
  Row row;
  for (uint32_t i = 0; i < table->num_rows; i++) {
    deserialize_row(row_slot(table, i), &row);
    print_row(&row);
  }
  return EXECUTE_SUCCESS;
}

ExecuteResult execute_statement(Statement* statement, Table* table) {


    switch (statement->type) {
        case (STATEMENT_INSERT):
            return execute_insert(statement, table);
        case (STATEMENT_SELECT):
            return execute_select(statement, table);
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


int main(int argc, char* argv[]) {
    Table* table = new_table();
    InputBuffer* input_buffer =  new_input_buffer();
    while (1) {
        print_prompt();
        read_input(input_buffer);

        // メタコマンド(ドットで始まるプログラムを制御するコマンド)
        if (input_buffer->buffer[0] == '.') {
          switch (do_meta_command(input_buffer, table)) {
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
            case (PREPARE_SYNTAX_ERROR):
                printf("Syntax error. Could not parse statement.\n");
                continue;
            case (PREPARE_UNRECOGNIZED_STATEMENT):
                printf("Unrecognized keyword at start of'%s'.\n", input_buffer->buffer);
                continue;
        }

        // ステートメントを実行
        switch (execute_statement(&statement, table)) {
            case (EXECUTE_SUCCESS):
                printf("Executed.\n");
                break;
            case (EXECUTE_TABLE_FULL):
                printf("Error: Table full.\n");
                break;
        }

    }
}