#include <unistd.h>
#include <sys/syscall.h>

int main() {
    // Use only system calls, no stdio
    const char* msg1 = "=== Syscall Test ===\n";
    const char* msg2 = "1. Basic syscall works\n";
    const char* msg3 = "2. Memory operations work\n";
    const char* msg4 = "=== Syscall test completed successfully ===\n";
    
    // Write to stdout using syscall
    syscall(SYS_write, 1, msg1, 21);
    syscall(SYS_write, 1, msg2, 25);
    syscall(SYS_write, 1, msg3, 30);
    syscall(SYS_write, 1, msg4, 45);
    
    return 0;
}
