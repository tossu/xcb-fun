#include <stdio.h>

int main (void)
{
    unsigned int ch;

    while ((ch = getchar()) != EOF)
    {
        putchar( ch );
        putchar( ch );
    }
    return 0;
}
