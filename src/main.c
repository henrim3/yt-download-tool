#include <assert.h>
#include <fcntl.h>
#include <fileapi.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <windows.h>

#include "logging.h"
#include "string_vector.h"

/* --------------- enums --------------- */

/* ------------- constants ------------- */

#define INPUT_BUF_SIZE 256
#define TOKENS_INITIAL_SIZE 8
#define OUTPUT_LOCATIONS_INITIAL_SIZE 8
#define TEMP_VIDS_DIR "tmp\\vids"
#define TEMP_WAVS_DIR "tmp\\wavs"

/* ------------ global vars ------------ */

// buffer to hold user input
char* input_buf = NULL;
int input_buf_len = 0;

// input tokens
char** tokens = NULL;
int tokens_len = 0;
int tokens_size = 0;

// places to output to
char** output_locations = NULL;
int output_locations_len = 0;
int output_locations_size = 0;

/* ------------------------------------- */

int str_eq( const char* s1, const char* s2 ) {
  return strcmp( s1, s2 ) == 0;
}

// ss is null terminated
int str_eq_any( const char* s, const char** ss ) {
  for ( const char** ptr = ss; *ptr != NULL; ptr++ ) {
    if ( str_eq( s, *ptr ) ) {
      return 1;
    }
  }
  return 0;
}

int char_in_str( char c, const char* s ) {
  int i = 0;
  while ( s[i] != '\0' ) {
    if ( s[i] == c ) {
      return 1;
    }
    i++;
  }

  // for ( char* ptr = s; *ptr != '\0'; ptr++ ) {
  //   if ( *ptr == c ) {
  //     return 1;
  //   }
  // }
  return 0;
}

int is_whitespace( char c ) {
  return char_in_str( c, " \n\t" );
}

int to_power( int x, int e ) {
  int res = 1;

  for ( int i = 0; i < e; i++ ) {
    res *= x;
  }

  return res;
}

// returns negative if invalid
int str_to_int( const char* s ) {
  int len = strlen( s );
  if ( len == 0 ) {
    return -1;
  }

  int res = 0;

  for ( int i = len - 1; i >= 0; i-- ) {
    char c = s[i];
    if ( c < '0' || c > '9' ) {
      return -1;
    }

    int val = c - '0';

    res += to_power( 10, len - ( i + 1 ) ) * val;
  }

  return res;
}

void free_null_terminated_str_arr( char** arr ) {
  for ( char** ptr = arr; *ptr != NULL; ptr++ ) {
    free( *ptr );
  }
  free( arr );
}

void free_str_arr( char** arr, int n ) {
  for ( int i = 0; i < n; i++ ) {
    if ( arr[i] == NULL ) {
      LOG_ERROR( "ruh roh" );
    }
    free( arr[i] );
  }
  free( arr );
  arr = NULL;
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
  memset( input_buf, 0, INPUT_BUF_SIZE );
  if ( fgets( input_buf, INPUT_BUF_SIZE, stdin ) == NULL &&
       ferror( stdin ) != 0 ) {
    LOG_PERROR( "Error from fgets while getting input" );
    exit( EXIT_FAILURE );
  }
  input_buf_len = strlen( input_buf );

  // get rid of newline
  input_buf[input_buf_len - 1] = '\0';
  input_buf_len--;
}

char* substr( char* s, int start, int end ) {
  assert( s != NULL && "substring source can't be null" );
  assert( start >= 0 && "substring start should be >= 0" );
  assert( end >= start && "end should come after start" );

  int substr_len = end - start;

  char* res = malloc( ( substr_len + 1 ) * sizeof( char ) );
  res[substr_len] = '\0';

  for ( int i = 0; i < substr_len; i++ ) {
    res[i] = s[start + i];
  }

  return res;
}

void get_tokens() {
  tokens_size = TOKENS_INITIAL_SIZE;
  tokens_len = 0;

  tokens = calloc( tokens_size, sizeof( char* ) );

  if ( tokens == NULL ) {
    LOG_PERROR( "calloc error while allocating tokens" );
    exit( EXIT_FAILURE );
  }

  int i = 0;
  while ( i < input_buf_len ) {
    // skip whitespace
    while ( i < input_buf_len && is_whitespace( input_buf[i] ) ) {
      i++;
    }

    if ( i == input_buf_len ) {
      break;
    }

    char* token = NULL;
    int start = i;
    int is_quote = 0;

    while ( i < input_buf_len ) {
      if ( i == input_buf_len - 1 ) {
        if ( is_quote && input_buf[i] == '"' ) {
          token = substr( input_buf, start + 1, i );
        } else {
          token = substr( input_buf, start, i + 1 );
        }
        break;
      }

      if ( !is_quote && is_whitespace( input_buf[i] ) ) {
        token = substr( input_buf, start, i );
        break;
      }

      if ( input_buf[i] == '"' ) {
        if ( !is_quote ) {
          is_quote = 1;
        } else {
          // empty quotes
          if ( i - start - 1 == 0 ) {
            i++;
            continue;
          }
          token = substr( input_buf, start + 1, i );
          break;
        }
      }

      i++;
    }
    i++;

    // grow if necessary, leave empty at end for NULL
    if ( tokens_len + 1 >= tokens_size ) {
      tokens_size *= 2;
      tokens = realloc( tokens, tokens_size * sizeof( char* ) );
    }

    // copy to tokens
    tokens[tokens_len] = malloc( ( strlen( token ) + 1 ) * sizeof( char ) );
    strcpy( tokens[tokens_len], token );
    tokens_len++;
  }
}

int is_input_safe( const char* input ) {
  if ( strstr( input, "rm" ) ) {
    return 0;
  }

  if ( char_in_str( ';', input ) ) {
    return 0;
  }

  if ( strstr( input, "&&" ) ) {
    return 0;
  }

  if ( strstr( input, "||" ) ) {
    return 0;
  }

  return 1;
}

StringVector* get_files_in_dir( const char* dir_path ) {
  WIN32_FIND_DATA find_data;
  HANDLE handle;

  char search_path[2048];

  // get all files
  sprintf( search_path, "%s\\*.*", dir_path );

  handle = FindFirstFile( search_path, &find_data );
  if ( handle == INVALID_HANDLE_VALUE ) {
    LOG_ERROR( "FindFirstFile failed" );
    exit( EXIT_FAILURE );
  }

  StringVector* file_names = malloc( sizeof( StringVector ) );
  StringVector_init( file_names, 32 );

  do {
    // skip . and ..
    if ( str_eq_any( find_data.cFileName,
                     (const char*[]){ ".", "..", NULL } ) ) {
      continue;
    }

    StringVector_append( file_names, find_data.cFileName );
  } while ( FindNextFile( handle, &find_data ) );

  FindClose( handle );

  return file_names;
}

void download_yt_video( const char* url ) {
  printf( "Downloading %s...\n", url );

  if ( !is_input_safe( url ) ) {
    printf( "Couldn't download %s due to unsafe input\n", url );
    return;
  }

  FILE* p_pipe;
  char buf[128];

  char command[512];
  snprintf( command, sizeof( command ),
            "python yt-dlp/yt_dlp/__main__.py \"%s\" -P .\\%s > $nul", url,
            TEMP_VIDS_DIR );

  printf( "Running command: %s\n", command );

  if ( ( p_pipe = _popen( command, "rt" ) ) == NULL ) {
    LOG_PERROR( "_popen error while running ytdlp command" );
    exit( EXIT_FAILURE );
  }

  // need to flush for whatever reason
  while ( fgets( buf, sizeof( buf ), p_pipe ) ) {
  }

  if ( _pclose( p_pipe ) ) {
    LOG_ERROR( "Error while running ytdlp command" );
    exit( EXIT_FAILURE );
  }

  printf( "Successfully downloaded %s\n", url );
}

void download_yt_videos( const char** urls, int n ) {
  for ( int i = 0; i < n; i++ ) {
    download_yt_video( urls[i] );
  }
}

void convert_video_to_wav( const char* file ) {
  printf( "Converting %s...\n", file );

  // create temp wavs directory
  if ( CreateDirectory( TEMP_WAVS_DIR, NULL ) == 0 ) {
    LOG_ERROR( "Error while creating temp wavs directory" );
    exit( EXIT_FAILURE );
  }

  if ( !is_input_safe( file ) ) {
    printf( "Couldn't download %s due to unsafe input\n", file );
    return;
  }

  FILE* p_pipe;
  char buf[128];

  char command[512];
  snprintf( command, sizeof( command ),
            "ffmpeg -i \"%s\\%s\" \"%s\\%s.wav\" > $nul", TEMP_VIDS_DIR, file,
            TEMP_WAVS_DIR, file );

  printf( "Running command: %s\n", command );

  if ( ( p_pipe = _popen( command, "rt" ) ) == NULL ) {
    LOG_PERROR( "_popen error while running ffmpeg command" );
    exit( EXIT_FAILURE );
  }

  // need to flush for whatever reason
  while ( fgets( buf, sizeof( buf ), p_pipe ) ) {
  }

  if ( _pclose( p_pipe ) ) {
    LOG_ERROR( "Error while running ffmpeg command" );
    exit( EXIT_FAILURE );
  }

  printf( "Successfully converted %s\n", file );
}

void convert_videos_to_wavs() {
  StringVector* video_files;
  video_files = get_files_in_dir( ".\\tmp\\vids" );

  for ( int i = 0; i < video_files->length; i++ ) {
    convert_video_to_wav( StringVector_get( video_files, i ) );
  }

  StringVector_free( video_files );
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

void add_output_location( const char* output_path ) {
  ensure_output_locations_initialized();

  if ( output_locations_len + 1 >= output_locations_size ) {
    output_locations_size *= 2;
    output_locations =
        realloc( output_locations, output_locations_size * sizeof( char* ) );
  }

  output_locations[output_locations_len] =
      malloc( ( strlen( output_path ) + 1 ) * sizeof( char ) );
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

// 1-indexed
void delete_output_location( int num ) {
  if ( num <= 0 || num > output_locations_len ) {
    printf( "Invalid delete index %d\n", num );
    return;
  }

  printf( "Deleting output: %s\n", output_locations[num - 1] );

  output_locations_len--;

  free( output_locations[num - 1] );

  // shift everything down
  for ( int i = num - 1; i < output_locations_len; i++ ) {
    output_locations[i] = output_locations[i + 1];
  }

  // shrink if possible
  if ( output_locations_len < output_locations_size / 2 &&
       output_locations_size / 2 >= OUTPUT_LOCATIONS_INITIAL_SIZE ) {
    output_locations_size /= 2;
    output_locations =
        realloc( output_locations, output_locations_size * sizeof( char ) );
  }
}

void handle_download_command() {
  download_yt_videos( (const char**)tokens + 1, tokens_len - 1 );
  convert_videos_to_wavs();
}

void handle_output_command() {
  if ( str_eq( tokens[1], "add" ) ) {
    if ( tokens_len == 3 ) {
      add_output_location( tokens[2] );
    } else {
      printf( "output add command requires 3 tokens\n" );
    }
    return;
  }

  if ( str_eq( tokens[1], "del" ) ) {
    if ( tokens_len == 3 ) {
      delete_output_location( str_to_int( tokens[2] ) );
    } else {
      printf( "output del requires 3 tokens\n" );
    }
    return;
  }

  if ( str_eq( tokens[1], "list" ) ) {
    if ( tokens_len == 2 ) {
      list_output_locations();
    } else {
      printf( "output list requires 2 tokens\n" );
    }
    return;
  }

  printf( "output command not found\n" );
  print_help();
}

// returns 1 if time to exit
int handle_input() {
  get_tokens();

  if ( tokens_len == 0 || tokens[0] == NULL ) {
    printf( "No input provided...\n" );
    return 0;
  }

  if ( str_eq_any( tokens[0], (const char*[]){ "exit", "quit", NULL } ) ) {
    printf( "Closing... Bye!\n" );
    return 1;
  }

  if ( str_eq( tokens[0], "dl" ) ) {
    handle_download_command();
    return 0;
  }

  if ( str_eq( tokens[0], "output" ) ) {
    handle_output_command();
    return 0;
  }

  if ( str_eq( tokens[0], "help" ) ) {
    print_help();
    return 0;
  }

  printf( "Command not recognized\n" );
  print_help();

  return 0;
}

void main_shell_loop() {
  while ( 1 ) {
    print_prompt();
    get_input();
    if ( handle_input() == 1 ) {
      return;
    }
    free_str_arr( tokens, tokens_len );
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
  free_str_arr( output_locations, output_locations_len );

  return EXIT_SUCCESS;
}
