#include <stdio.h>
#include <stdlib.h>
#include "my_getline.h"

void test_my_getline() {
    char* buffer = NULL;
    size_t buffer_length = 0;
    ssize_t result;

    printf("Test 1: Normal input\n");
    printf("Enter 'Hello World': ");
    result = my_getline(&buffer, &buffer_length, stdin);
    if (result != -1) {
        printf("Buffer: '%s', Length: %zd\n", buffer, result);
    } else {
        printf("Error or EOF encountered\n");
    }

    printf("\nTest 2: Empty input\n");
    printf("Press Enter without typing: ");
    result = my_getline(&buffer, &buffer_length, stdin);
    if (result != -1) {
        printf("Buffer: '%s', Length: %zd\n", buffer, result);
    } else {
        printf("Error or EOF encountered\n");
    }

    printf("\nTest 3: Long input (more than 128 characters)\n");
    printf("Enter a long string: ");
    result = my_getline(&buffer, &buffer_length, stdin);
    if (result != -1) {
        printf("Buffer: '%s', Length: %zd\n", buffer, result);
    } else {
        printf("Error or EOF encountered\n");
    }

    printf("\nTest 4: EOF before newline\n");
    printf("Simulate EOF by pressing Ctrl+D (Linux/Mac) or Ctrl+Z (Windows): ");
    result = my_getline(&buffer, &buffer_length, stdin);
    if (result != -1) {
        printf("Buffer: '%s', Length: %zd\n", buffer, result);
    } else {
        printf("Error or EOF encountered\n");
    }

    // 解放
    free(buffer);
}

int main() {
    test_my_getline();
    return 0;
}