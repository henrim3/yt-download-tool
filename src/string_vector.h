typedef struct StringVector {
    char** arr;
    int length;
    int capacity;
} StringVector;

void StringVector_init( StringVector* vec, int initial_capacity );
void StringVector_free( StringVector* vec );
void StringVector_append( StringVector* vec, char* s );
void StringVector_remove( StringVector* vec, int i );
char* StringVector_get( const StringVector* vec, int i );
void StringVector_print( const StringVector* vec );