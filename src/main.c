#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "logging.h"

#define INPUT_BUF_SIZE 256

int saved_stdout = -1;
char* input_buf = NULL;

void check_platform() {
// check platform
#if defined( _WIN64 )
  LOG_MESSAGE( "Running in Win64\n" );

#elif defined( _WIN32 )
  LOG_MESSAGE( "Running in Win32\n" );

#else
  LOG_ERROR( "Only windows is supported, exiting...\n" );
  exit( EXIT_FAILURE );

#endif
}

void check_python_installed() {
  FILE* p_pipe;

  if ( ( p_pipe = _popen( "python --version", "rt" ) ) == NULL ) {
    LOG_PERROR( "Error while running python --version" );
    exit( EXIT_FAILURE );
  }

  if ( _pclose( p_pipe ) ) {
    LOG_ERROR( "Python not installed" );
    exit( EXIT_FAILURE );
  }
}

void init_input_buf() {
  input_buf = malloc( INPUT_BUF_SIZE * sizeof( char ) );

  if ( input_buf == NULL ) {
    LOG_PERROR( "Error from malloc when initializing input buffer" );
    exit( EXIT_FAILURE );
  }
}

void print_prompt() {
  printf( "yt-download-tool> " );
}

void get_input() {
  if ( fgets( input_buf, INPUT_BUF_SIZE, stdin ) == NULL &&
       ferror( stdin ) != 0 ) {
    LOG_PERROR( "Error from fgets while getting input" );
    exit( EXIT_FAILURE );
  }
}

void strip_right( char* str ) {
  int count = 0;
  for ( int i = strlen( str ) - 1; i >= 0; i-- ) {
    if ( str[i] != ' ' && str[i] != '\n' && str[i] != '\t' ) {
      break;
    }
    count++;
  }

  str[strlen( str ) - count] = '\0';
}

// returns null terminated array
char** get_tokens( char* str ) {
  int arr_size = 8;
  int arr_len = 0;
  char** tokens = calloc( arr_size, sizeof( char* ) );

  char* tok = strtok( input_buf, " " );
  while ( tok != NULL ) {
    // grow if necessary
    if ( arr_len + 1 >= arr_size ) {
      arr_size *= 2;
      tokens = realloc( tokens, arr_size * sizeof( char* ) );
    }

    // copy to tokens
    tokens[arr_len] = malloc( ( strlen( tok ) + 1 ) * sizeof( char ) );
    strcpy( tokens[arr_len], tok );
    arr_len++;

    tok = strtok( NULL, " " );
  }

  tokens[arr_len] = NULL;

  return tokens;
}

void free_null_terminated_str_arr( char** arr ) {
  for ( char** ptr = arr; *ptr != NULL; ptr++ ) {
    free( *ptr );
  }
  free( arr );
}

void download_yt_vid( char* url ) {
  FILE* p_pipe;

  char* first_part = "python yt-dlp/yt_dlp/__main__.py ";
  char* command =
      malloc( ( strlen( first_part ) + strlen( url ) + 1 ) * sizeof( char ) );

  strcpy( command, first_part );
  strcpy( command + strlen( first_part ), url );

  if ( ( p_pipe = _popen( command, "rt" ) ) == NULL ) {
    LOG_PERROR( "_popen error while running ytdlp command" );
    exit( EXIT_FAILURE );
  }

  free( command );

  if ( _pclose( p_pipe ) ) {
    LOG_ERROR( "Error while runnning ytdlp command" );
    exit( EXIT_FAILURE );
  }
}

void handle_token( char* token ) {
  if ( strcmp( token, "exit" ) == 0 ) {
    exit( EXIT_SUCCESS );
  }

  download_yt_vid( token );
}

void handle_input() {
  strip_right( input_buf );
  char** tokens = get_tokens( input_buf );

  for ( char** ptr = tokens; *ptr != NULL; ptr++ ) {
    handle_token( *ptr );
  }

  free_null_terminated_str_arr( tokens );
}

void main_shell_loop() {
  while ( 1 ) {
    print_prompt();
    get_input();

    handle_input();
  }
}

int main() {
  LOG_MESSAGE( "Started yt-download-tool!\n" );

  init_input_buf();

  // make sure python is installed
  check_python_installed();

  main_shell_loop();

  free( input_buf );

  return EXIT_SUCCESS;
}