#ifndef SERIOUSERROR_H_INCLUDED
#define SERIOUSERROR_H_INCLUDED

#define seriousError(...) (fprintf(stderr,"%s\nPress Enter to exit",__VA_ARGS__),getchar(),exit(EXIT_FAILURE))
void setColor(int color);
#endif // SERIOUSERROR_H_INCLUDED
