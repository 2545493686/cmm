#include <stdio.h>

int main(int argc, char const *argv[])
{
    int i;
    printf("%d", (({ i = 6;}), i));
    return 0;
}
