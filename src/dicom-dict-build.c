#include "config.h"

#ifdef _WIN32
// the Windows CRT considers fopen and getenv unsafe
#define _CRT_SECURE_NO_WARNINGS
// and deprecates strdup
#define strdup(v) _strdup(v)
#endif

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "uthash.h"

#include <dicom/dicom.h>
#include "pdicom.h"
#include "dicom-dict-tables.h"

#define EMPTY 0xffffffff
#define MAX_PROBES 10

static FILE *c;
static FILE *h;

static void make_table(const char *name, const void *items, int count,
                       int item_size, int key_offset, int key_len,
                       bool string_key_skip_empty)
{
    int table_len = count * 4;
    unsigned *table = malloc(sizeof(unsigned) * table_len);
    for (int i = 0; i < table_len; i++) {
        table[i] = EMPTY;
    }

    #define KEY(index) ((const void *) \
        ((const char *) items + (index) * item_size + key_offset))
    int probe_counts[MAX_PROBES] = {0};
    for (int i = 0; i < count; i++) {
        if (string_key_skip_empty && ! *(const char *) KEY(i)) {
            continue;
        }
        int this_key_len = key_len ? key_len : (int) strlen(KEY(i));
        unsigned hash;
        HASH_VALUE(KEY(i), this_key_len, hash);
        int cell, probe;
        for (probe = 0, cell = hash % table_len;
             probe < MAX_PROBES && table[cell] != EMPTY;
             cell = (hash + ++probe) % table_len) {
            if (!memcmp(KEY(i), KEY(table[cell]), this_key_len)) {
                fprintf(stderr, "%s: Duplicate key at %d\n", name, i);
                exit(1);
            }
        }
        if (probe >= MAX_PROBES) {
            fprintf(stderr, "%s: Too many probes at %d\n", name, i);
            exit(1);
        }
        table[cell] = i;
        probe_counts[probe]++;
    }

    int repr_bits = count > UINT8_MAX ? 16 : 8;
    unsigned empty_repr = (1 << repr_bits) - 1;
    fprintf(c, "const unsigned %s_len = %d;\n", name, table_len);
    fprintf(h, "extern const unsigned %s_len;\n", name);
    fprintf(c, "const uint%d_t %s_empty = 0x%x;\n", repr_bits, name, empty_repr);
    fprintf(h, "extern const uint%d_t %s_empty;\n", repr_bits, name);
    fprintf(c, "const uint%d_t %s_dict[%u] = {", repr_bits, name, table_len);
    fprintf(h, "extern const uint%d_t %s_dict[];\n\n", repr_bits, name);
    for (int i = 0; i < table_len; i++) {
        if (!(i % 8)) {
            fprintf(c, "\n");
        }
        fprintf(c, "0x%x, ", table[i] == EMPTY ? empty_repr : table[i]);
    }
    fprintf(c, "\n};\n\n");
    free(table);

    if (getenv("DEBUG_DICT")) {
        double total_probes = 0;
        for (int i = 0; i < MAX_PROBES; i++) {
            total_probes += probe_counts[i] * (i + 1);
        }
        fprintf(stderr, "%-40s: %.3f probes/lookup, %7d bytes\n",
                name, total_probes / count, table_len * (repr_bits / 8));
    }
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s c-file h-file\n", argv[0]);
        return 1;
    }

    c = fopen(argv[1], "w");
    h = fopen(argv[2], "w");
    if (c == NULL || h == NULL) {
        fprintf(stderr, "Couldn't open files\n");
        return 1;
    }

    fprintf(c, "#include <stdint.h>\n\n");
    fprintf(h, "#define LOOKUP_MAX_PROBES %d\n\n", MAX_PROBES);

    #define ITEM_SIZE(table) sizeof((table)[0])
    #define FIELD_SIZE(table, field) sizeof((table)[0].field)
    #define FIELD_OFFSET(table, field) \
        ((int) ((const char *) &(table)[0].field - (const char *) &(table)[0]))

    make_table("dcm_vrtable_from_str",
               dcm_vr_table, dcm_vr_table_len,
               ITEM_SIZE(dcm_vr_table),
               FIELD_OFFSET(dcm_vr_table, str),
               0,  // strlen
               false);

    make_table("dcm_attribute_from_tag",
               dcm_attribute_table, dcm_attribute_table_len,
               ITEM_SIZE(dcm_attribute_table),
               FIELD_OFFSET(dcm_attribute_table, tag),
               FIELD_SIZE(dcm_attribute_table, tag),
               false);

    // The "" keyword appears several times and is used for retired tags ...
    // we can't map this to tags unambiguously, so we skip it in the table
    make_table("dcm_attribute_from_keyword",
               dcm_attribute_table, dcm_attribute_table_len,
               ITEM_SIZE(dcm_attribute_table),
               FIELD_OFFSET(dcm_attribute_table, keyword),
               0,  // strlen
               true);

    if (fclose(c) || fclose(h)) {
        fprintf(stderr, "Couldn't write files\n");
        return 1;
    }

    return 0;
}
