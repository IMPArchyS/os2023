#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int isPrime(int n)
{
    if (n <= 1) return 0;
    if (n <= 3) return 1;
    if (n % 2 == 0 || n % 3 == 0) return 0;

    for (int i = 5; i * i <= n; i += 6) 
        if (n % i == 0 || n % (i + 2) == 0) return 0;

    return 1;    
}
int main(int argc, char** argv)
{
    if (argc != 1)
    {
        fprintf(2, "primes error!\n");
        exit(1);
    }

    int p[2];
    pipe(p);

    if (fork() == 0) // child
    {
        close(p[0]);

        for (int i = 2; i <= 35; i++)
        {
            if (isPrime(i))
                write(p[1], &i, sizeof(i));
        }

        close(p[1]); 
        exit(0);
    }
    else // Parent
    {
        close(p[1]);

        int prime;
        while (read(p[0], &prime, sizeof(prime)) > 0)
            printf("prime %d\n", prime);

        close(p[0]); 
        wait(0);
        exit(0);
    }
}
