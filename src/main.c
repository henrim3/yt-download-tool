#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "logging.h"

int saved_stdout = -1;

void disable_stdout() {
  LOG_MESSAGE( "TODO: implement disable_stdout" );

  // // flush any buffered output
  // fflush( stdout );

  // // open nul file
  // int nul_fd = _open( "NUL", _O_WRONLY );
  // if ( nul_fd == -1 ) {
  //   LOG_PERROR( "Error while opening NUL" );
  //   exit( EXIT_FAILURE );
  // }

  // // save stdout
  // saved_stdout = _dup( _fileno( stdout ) );
  // if ( saved_stdout == -1 ) {
  //   LOG_PERROR( "Error whlie saving stdout" );
  //   exit( EXIT_FAILURE );
  // }

  // if ( _dup2( nul_fd, _fileno( stdout ) ) == -1 ) {
  //   LOG_PERROR( "Error while redirecting stdout to NUL" );
  //   exit( EXIT_FAILURE );
  // }

  // _close( nul_fd );
}

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

  disable_stdout();

  if ( ( p_pipe = _popen( "python --version", "rt" ) ) == NULL ) {
    LOG_PERROR( "Error while running python --version" );
    exit( 1 );
  }

  if ( _pclose( p_pipe ) ) {
    LOG_ERROR( "Python not installed" );
    exit( 1 );
  }
}

int main() {
  LOG_MESSAGE( "Started yt-download-tool!\n" );

  // make sure python is installed
  check_python_installed();

  return EXIT_SUCCESS;
}