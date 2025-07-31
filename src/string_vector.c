#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logging.h"
#include "string_vector.h"

void StringVector_init( StringVector* vec, int initial_capacity ) {
  assert( vec != NULL );
  assert( initial_capacity > 0 );
  vec->length = 0;
  vec->capacity = initial_capacity;
  vec->arr = malloc( initial_capacity * sizeof( char* ) );
}

void StringVector_free( StringVector* vec ) {
  assert( vec != NULL && "Can't free null vector" );
  
  for ( int i = 0; i < vec->length; i++ ) {
    free( vec->arr[i] );
  }

  free( vec->arr );
}

void StringVector_append( StringVector* vec, char* s ) {
  assert( vec != NULL );
  // grow if necessary
  if ( vec->length + 1 == vec->capacity ) {
    vec->capacity *= 2;
    vec->arr = realloc( vec->arr, vec->capacity * sizeof( char* ) );
  }

  vec->arr[vec->length] = malloc( ( strlen( s ) + 1 ) * sizeof( char ) );
  strcpy( vec->arr[vec->length], s );
  
  vec->length++;
}

void StringVector_remove( StringVector* vec, int i ) {
  assert( vec != NULL );
  assert( i >= 0 );
  assert( i < vec->length );
  
  vec->length--;
  free( vec->arr[i] );

  // shift everything down
  for ( int j = i; j < vec->length - 1; j++ ) {
    vec->arr[j] = vec->arr[j + 1];
  }
}

char* StringVector_get( const StringVector* vec, int i ) {
  assert( i >= 0 && i < vec->length );
  return vec->arr[i];
}

void StringVector_print( const StringVector* vec ) {
  printf( "StringVector at %p\n", vec );
  for ( int i = 0; i < vec->length; i++ ) {
    printf( "  %d: %s\n", i, vec->arr[i] );
  }
}