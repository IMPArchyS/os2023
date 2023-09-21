#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main()
{
  if (argc != 2)
  {
    fprintf(2, "sleep error\n");
    exit(1);
  }

  int tick = atoi(argv[1]);
  if (tick < 0) 
  {
    fprintf(2, "sleep invalid arg");
    exit(1);
  }

  sleep(tick);
  exit(0);
}
