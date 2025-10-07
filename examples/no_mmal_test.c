#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
    printf("=== NO MMAL TEST ===\n");
    
    // Test basic operations
    printf("1. Basic printf works\n");
    
    // Test memory operations
    char* test = malloc(100);
    if (test) {
        strcpy(test, "No MMAL Test");
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
    
    // Test integer operations (no floating point)
    int test_int = 314159;
    printf("5. Integer operations: %d\n", test_int);
    
    // Test system calls
    printf("6. System call test: PID = %d\n", getpid());
    
    printf("=== NO MMAL test completed successfully ===\n");
    return 0;
}
