#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    printf("=== Pi Zero ARMv6 Test ===\n");
    
    // Test basic operations
    printf("1. Basic printf works\n");
    
    // Test memory operations
    char* test = malloc(100);
    if (test) {
        strcpy(test, "Pi Zero Test");
        printf("2. Memory allocation works: %s\n", test);
        free(test);
        printf("3. Memory deallocation works\n");
    } else {
        printf("2. Memory allocation failed\n");
        return 1;
    }
    
    // Test stack operations
    int stack_array[10];
    for (int i = 0; i < 10; i++) {
        stack_array[i] = i * i;
    }
    printf("4. Stack operations work: %d\n", stack_array[5]);
    
    // Test floating point (if available)
    float test_float = 3.14159f;
    printf("5. Float operations: %.2f\n", test_float);
    
    printf("=== Pi Zero test completed successfully ===\n");
    return 0;
}
