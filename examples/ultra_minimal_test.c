#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("=== Ultra Minimal Test ===\n");
    printf("1. Basic printf works\n");
    
    char* test = malloc(100);
    if (test) {
        printf("2. Memory allocation works\n");
        free(test);
        printf("3. Memory deallocation works\n");
    }
    
    printf("=== Ultra minimal test completed successfully ===\n");
    return 0;
}
