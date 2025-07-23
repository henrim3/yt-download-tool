#ifndef __MIM_LOGGING_H__
#define __MIM_LOGGING_H__

#define COLOR_RESET "\033[0m"
#define BLACK "\033[30m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"
#define BOLDBLACK "\033[1m\033[30m"
#define BOLDRED "\033[1m\033[31m"
#define BOLDGREEN "\033[1m\033[32m"
#define BOLDYELLOW "\033[1m\033[33m"
#define BOLDBLUE "\033[1m\033[34m"
#define BOLDMAGENTA "\033[1m\033[35m"
#define BOLDCYAN "\033[1m\033[36m"
#define BOLDWHITE "\033[1m\033[37m"

#include <stdio.h>

#define LOG_IMPORTANT( msg, ... )                                              \
  do {                                                                         \
    printf( "%s%s:%d: ", BOLDCYAN, __FILE__, __LINE__ );                       \
    printf( msg, ##__VA_ARGS__ );                                              \
    printf( "%s\n", COLOR_RESET );                                             \
  } while ( 0 )

#define LOG_MESSAGE( msg, ... )                                                \
  do {                                                                         \
    printf( "%s%s:%d:%s ", YELLOW, __FILE__, __LINE__, COLOR_RESET );          \
    printf( msg, ##__VA_ARGS__ );                                              \
    printf( "\n" );                                                            \
  } while ( 0 )

#define LOG_ERROR( msg, ... )                                                  \
  do {                                                                         \
    fprintf( stderr, "%s%s:%d:%s ERROR: %s", RED, __FILE__, __LINE__, BOLDRED, \
             COLOR_RESET );                                                    \
    fprintf( stderr, msg, ##__VA_ARGS__ );                                     \
    fprintf( stderr, "\n" );                                                   \
  } while ( 0 )

#define LOG_PERROR( msg )                                                      \
  do {                                                                         \
    fprintf( stderr, "%s%s:%d:%s ", RED, __FILE__, __LINE__, COLOR_RESET );    \
    perror( msg );                                                             \
  } while ( 0 )

#endif // __MIM_LOGGING_H__