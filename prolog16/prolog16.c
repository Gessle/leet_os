#include <stdio.h>
#include <string.h>
#include <inttypes.h>


typedef struct {
    uint16_t    caller_ip;
    uint16_t    caller_bp;
} near_args;

typedef struct {
    uint16_t    caller_ip;
    uint16_t    caller_cs;
    uint16_t    caller_bp;
} far_args;


uint16_t    get_cs( void );
#pragma aux get_cs = "mov ax, cs" modify nomemory exact [];

uint16_t    get_bp( void );
#pragma aux get_bp = value [bp] modify nomemory exact [];

/* Explanation of #pragma aux usage:
 *  #pragma aux _PRO -- refers to function named _PRO
 *  "_*" -- prepend an underscore to the name (_PRO --> __PRO)
 *  parm [ax] caller -- first argument passed in AX, caller pushes
 *    remainding arguments on stack; fake argument in AX used to
 *    force saving AX if __STK is called from prologue
 *  value [sp] -- convince compiler that AX is not used to pass the
 *    return value and therefore needs saving
 *  modify exact [sp] -- force compiler to preserve all registers
 *    except SP (which in behaves normally)
 */
#pragma aux _PRO "_*" parm [ax] caller value [sp] modify exact [sp];

void _PRO( uint16_t preserve_ax, uint16_t saved_bp )
{
    uint16_t        __based( __segname( "_STACK" ) ) *pro_bp = &saved_bp;
    near_args __far *near_arg = (near_args *)(pro_bp - 1);
    far_args __far  *far_arg  = (far_args *)(pro_bp - 2);
    int             i;
    uint8_t         __far *code_ptr;
    uint8_t         name_len = 0;
    const char      __far *func_name = "unknown";

    /* Find the caller's prologue (where __PRO will return). The low bit
     * of the saved BP will be set if far calls are used.
     */
    if( saved_bp & 1 )
        code_ptr = far_arg->caller_cs :> (uint8_t *)far_arg->caller_ip;
    else
        code_ptr = get_cs() :> (uint8_t *)near_arg->caller_ip;

    /* Look for PUSH BP / MOV BP, SP sequence. */
    for( i = 0; i < 32; ++i, --code_ptr ) {
        if( code_ptr[0] == 0x55 ) {
            if( code_ptr[1] == 0x89 && code_ptr[2] == 0xE5 ) {
                /* A far call will have an extra 'INC BP' at the start.
                 * We have to skip past the additional byte.
                 */
                if( saved_bp & 1 )
                    --code_ptr;
                name_len  = code_ptr[-1];
                func_name = "found";
                break;
            }
        }
    }

    if( name_len ) {
        /* String not null-terminated, length must be known! */
        func_name = code_ptr - name_len - 1;
    } else {
        /* String is null-terminated. */
        name_len = _fstrlen( func_name );
    }

    /* Indent. */
/*    for( i = 0; i < level; ++i )
        printf( " ");

    printf( "caller cs:ip=%Fp, bp=%04X, my bp=%04X, entering: %.*Fs\n", code_ptr, near_arg->caller_bp, get_bp(), name_len, func_name );
    ++level;*/

  debug_callstack_push(name_len, func_name, code_ptr, near_arg->caller_bp, get_bp());    
}

#pragma aux _EPI "_*" parm [ax] value [sp] modify exact [sp];

void _EPI( uint16_t preserve_ax )
{
/*    int     i;

    --level;

    /* Indent. */
  /*  for( i = 0; i < level; ++i )
        printf( " " );

    printf( "return\n" );*/

  debug_callstack_pop();    
}
