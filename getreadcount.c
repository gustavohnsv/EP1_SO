// Test new system call getreadcount()

#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
    int n = getreadcount();
    printf(1, "read count: %d\n", n);
    exit();
}
