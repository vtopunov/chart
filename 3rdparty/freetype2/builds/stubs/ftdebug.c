#include <freetype/internal/ftdebug.h>

FT_BASE_DEF( void )
ft_debug_init( void )
{}


FT_BASE_DEF( FT_Int )
FT_Trace_Get_Count( void )
{
  return 0;
}


FT_BASE_DEF( const char * )
FT_Trace_Get_Name( FT_Int  idx )
{
  FT_UNUSED( idx );
  return NULL;
}


FT_BASE_DEF( void )
FT_Trace_Disable( void )
{}


FT_BASE_DEF( void )
FT_Trace_Enable( void )
{}