#ifndef __ARGS_H
#define __ARGS_H

char *shift_args(int *argc, char **argv[])
{
    *argc -= 1;
    char *arg = **argv;
    *argv += 1;
    return arg;
}


#endif // __ARGS_H
