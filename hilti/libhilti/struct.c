/* $Id$
 *
 * Support functions HILTI's struct data type.
 *
 */

#include "hilti.h"
#include "str.h"

#include <stdio.h>

static hlt_string_constant prefix = { 1, "<" };
static hlt_string_constant postfix = { 1, ">" };
static hlt_string_constant separator = { 2, ", " };
static hlt_string_constant equal = { 1, "=" };
static hlt_string_constant null = { 6, "<null>" };

hlt_string hlt_struct_to_string(const hlt_type_info* type, const void* obj, int32_t options, hlt_exception** excpt, hlt_execution_context* ctx)
{
    // One entry in the type parameter array.
    struct field {
        const char* field;
        int16_t offset;
    };

    assert(type->type == HLT_TYPE_STRUCT);

    struct field* array = (struct field *)type->aux;

    obj = *((const char**)obj);

    if ( ! obj )
        return &null;

    uint32_t mask = *((uint32_t*)obj);

    int i;
    hlt_string s = hlt_string_from_asciiz("", excpt, ctx);

    s = hlt_string_concat(s, &prefix, excpt, ctx);
    if ( *excpt )
        return 0;

    hlt_type_info** types = (hlt_type_info**) &type->type_params;
    for ( i = 0; i < type->num_params; i++ ) {

        if ( array[i].field[0] && array[i].field[1] &&
             array[i].field[0] == '_' && array[i].field[1] == '_' )
            // Don't print internal names.
            continue;

        if ( i >  0 ) {
            s = hlt_string_concat(s, &separator, excpt, ctx);
            if ( *excpt )
                return 0;
        }

        hlt_string t;

        uint32_t is_set = (mask & (1 << i));

        if ( ! is_set )
            t = hlt_string_from_asciiz("(not set)", excpt, ctx);

        else if ( types[i]->to_string ) {
            t = (types[i]->to_string)(types[i], obj + array[i].offset, 0, excpt, ctx);
            if ( *excpt )
                return 0;
        }
        else
            // No format function.
            t = hlt_string_from_asciiz(types[i]->tag, excpt, ctx);

        hlt_string field_s = hlt_string_from_asciiz(array[i].field, excpt, ctx);

        s = hlt_string_concat(s, field_s, excpt, ctx);
        s = hlt_string_concat(s, &equal, excpt, ctx);
        s = hlt_string_concat(s, t, excpt, ctx);
        if ( *excpt )
            return 0;

    }

    return hlt_string_concat(s, &postfix, excpt, ctx);
}
