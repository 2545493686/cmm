#include <stdio.h>
#include "lib/sc/array/sc_array.h"

int main(int argc, char const *argv[])
{
    struct sc_array_int token_types;
    struct sc_array_str token_values;

	sc_array_init(&token_types);
    sc_array_init(&token_values); 

    int token_type, read_count;
    char token_value[32];
    memset(token_value, 0, sizeof(token_value));

    while ((read_count = scanf("%d %s ", &token_type, token_value)) != -1)
    {
        printf("read %d: %d %s\n", read_count, token_type, token_value);
        if (read_count <= 1)
        {
            printf("Error: invalid value");
            assert(0);
        }
        if (token_value[32] != '\0')
        {
            printf("Error: value too long");
            assert(0);
        }

        sc_array_add(&token_types, token_type);
        sc_array_add(&token_values, token_value);

        memset(token_value, 0, sizeof(token_value));
    }

    for (size_t i = 0; i < sc_array_size(&token_types); i++) {
		printf("%d ", token_types.elems[i]);
	}

    printf("count: %ld", sc_array_size(&token_types));    
}
