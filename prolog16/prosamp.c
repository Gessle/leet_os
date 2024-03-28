int bar( int a )
{
    return( a - 33 );
}

void foo( int a )
{
    bar( a + 57 );
}

int main( int argc, char *argv[] )
{
    foo( argc );

    return( 0 );
}
