#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    printf("=== Minimal Test ===\n");
    printf("Basic printf works\n");
    
    // Test basic memory operations
    char* test = malloc(100);
    if (test) {
        strcpy(test, "Hello World");
        printf("Memory allocation works: %s\n", test);
        free(test);
        printf("Memory deallocation works\n");
    } else {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    printf("=== Minimal test completed successfully ===\n");
    return 0;
}
