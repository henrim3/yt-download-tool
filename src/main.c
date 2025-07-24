#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "logging.h"

/* ------------- constants ------------- */

#define INPUT_BUF_SIZE 256
#define TOKENS_INITIAL_SIZE 8
#define OUTPUT_LOCATIONS_INITIAL_SIZE 4

/* ------------------------------------- */

/* ------------ global vars ------------ */

// buffer to hold input
char* input_buf = NULL;

char** tokens = NULL;
int tokens_len = 0;
int tokens_size = 0;

// places to output to
char** output_locations = NULL;
int output_locations_len = 0;
int output_locations_size = 0;

/* ------------------------------------- */

int str_eq( char* s1, char* s2 ) {
  return strcmp( s1, s2 ) == 0;
}

// ss is null terminated
int str_eq_any( char* s, char** ss ) {
  for ( char** ptr = ss; *ptr != NULL; ptr++ ) {
    if ( str_eq( s, *ptr ) ) {
      return 1;
    }
  }
  return 0;
}

int char_in_str( char c, char* s ) {
  for ( char* ptr = s; *ptr != '\0'; ptr++ ) {
    if ( *ptr == c ) {
      return 1;
    }
  }
  return 0;
}

void free_null_terminated_str_arr( char** arr ) {
  for ( char** ptr = arr; *ptr != NULL; ptr++ ) {
    free( *ptr );
  }
  free( arr );
}

void free_str_arr( char** arr, int n ) {
  for ( int i = 0; i < n; i++ ) {
    free( arr[i] );
  }
  free( arr );
}

void check_platform() {
// check platform
#if defined( _WIN64 )
  printf( "Running in Win64\n" );

#elif defined( _WIN32 )
  printf( "Running in Win32\n" );

#else
  LOG_ERROR( "Only windows is supported, exiting...\n" );
  exit( EXIT_FAILURE );

#endif
}

void check_python_installed() {
  FILE* p_pipe;

  if ( ( p_pipe = _popen( "python --version > $nul", "rt" ) ) == NULL ) {
    LOG_PERROR( "Error while running python --version" );
    exit( EXIT_FAILURE );
  }

  if ( _pclose( p_pipe ) ) {
    printf( "Python not installed\n" );
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
    if ( !char_in_str( str[i], " \n\t" ) ) {
      break;
    }
    count++;
  }

  str[strlen( str ) - count] = '\0';
}

// tokens is null terminated
void get_tokens() {
  tokens_size = TOKENS_INITIAL_SIZE;
  tokens_len = 0;

  if ( tokens != NULL ) {
    free_str_arr( tokens, tokens_size );
    tokens = NULL;
  }
  tokens = calloc( tokens_size, sizeof( char* ) );

  char* tok = strtok( input_buf, " " );
  while ( tok != NULL ) {
    // grow if necessary, leave empty at end for NULL
    if ( tokens_len + 1 >= tokens_size ) {
      tokens_size *= 2;
      tokens = realloc( tokens, tokens_size * sizeof( char* ) );
    }

    // copy to tokens
    tokens[tokens_len] = malloc( ( strlen( tok ) + 1 ) * sizeof( char ) );
    strcpy( tokens[tokens_len], tok );
    tokens_len++;

    tok = strtok( NULL, " " );
  }

  tokens[tokens_len] = NULL;
}

void download_yt_vid( char* url ) {
  printf( "Downloading %s\n", url );

  FILE* p_pipe;
  char buf[128];

  char command[512];
  snprintf( command, sizeof( command ),
            "python yt-dlp/yt_dlp/__main__.py -x \"%s\" > $nul", url );

  printf( "Running command: %s\n", command );

  if ( ( p_pipe = _popen( command, "rt" ) ) == NULL ) {
    LOG_PERROR( "_popen error while running ytdlp command" );
    exit( EXIT_FAILURE );
  }

  // need to flush for whatever reason
  while ( fgets( buf, sizeof( buf ), p_pipe ) ) {
  }

  if ( _pclose( p_pipe ) ) {
    LOG_ERROR( "Error while runnning ytdlp command" );
    exit( EXIT_FAILURE );
  }

  printf( "Successfully downloaded %s\n", url );
}

void download_yt_vids( char** urls ) {
  for ( char** ptr = urls; *ptr != NULL; ptr++ ) {
    download_yt_vid( *ptr );
  }
}

void print_help() {
  printf( "Help not available\n" );
}

void print_add_command_help() {
  printf( "try add output...\n" );
}

void ensure_output_locations_initialized() {
  if ( output_locations == NULL ) {
    output_locations_size = OUTPUT_LOCATIONS_INITIAL_SIZE;
    output_locations = malloc( output_locations_size * sizeof( char* ) );
  }
}

void add_output_location( char* output_path ) {
  ensure_output_locations_initialized();

  if ( output_locations_len + 1 >= output_locations_size ) {
    output_locations_size *= 2;
    output_locations =
        realloc( output_locations, output_locations_size * sizeof( char* ) );
  }

  output_locations[output_locations_len] =
      malloc( strlen( output_path ) * sizeof( char ) );
  strcpy( output_locations[output_locations_len], output_path );

  output_locations_len++;

  printf( "Added output: %s\n", output_path );
}

void list_output_locations() {
  if ( output_locations_len == 0 ) {
    printf( "No outputs\n" );
    return;
  }

  printf( "Output locations:\n" );
  for ( int i = 0; i < output_locations_len; i++ ) {
    printf( "  %d. %s\n", i + 1, output_locations[i] );
  }
}

void handle_add_command() {
  if ( str_eq( tokens[1], "output" ) ) {
    if ( tokens_len < 3 ) {
      printf( "You must specify an output path to add\n" );
      return;
    }
    add_output_location( tokens[2] );
  } else {
    printf( "Add command not recognized\n" );
    print_add_command_help();
  }
}

void handle_list_command() {
  if ( str_eq_any( tokens[1], (char*[]){ "output", "outputs", NULL } ) ) {
    if ( tokens[0] == NULL ) {
      printf( "Listing outputs\n" );
      return;
    }
    list_output_locations();
  } else {
    printf( "Add command not recognized\n" );
    print_add_command_help();
  }
}

void handle_input() {
  strip_right( input_buf );
  get_tokens();

  if ( tokens_len == 0 || tokens[0] == NULL ) {
    printf( "No input provided...\n" );
    return;
  }

  if ( str_eq( tokens[0], "exit" ) ) {
    printf( "Closing... Bye!\n" );
    exit( EXIT_SUCCESS );
    return;
  }

  if ( str_eq( tokens[0], "dl" ) ) {
    download_yt_vids( &( tokens[1] ) );
    return;
  }

  if ( str_eq( tokens[0], "add" ) ) {
    handle_add_command();
    return;
  }

  if ( str_eq( tokens[0], "list" ) ) {
    handle_list_command();
    return;
  }

  if ( str_eq( tokens[0], "help" ) ) {
    print_help();
    return;
  }

  printf( "Command not recognized\n" );
  print_help();
}

void main_shell_loop() {
  while ( 1 ) {
    print_prompt();
    get_input();
    handle_input();
  }
}

int main() {
  printf( "Started yt-download-tool!\n" );

  check_platform();

  init_input_buf();

  // make sure python is installed
  check_python_installed();

  main_shell_loop();

  // free globals
  free( input_buf );
  free_str_arr( tokens, tokens_size );
  free_str_arr( output_locations, output_locations_size );

  return EXIT_SUCCESS;
}
