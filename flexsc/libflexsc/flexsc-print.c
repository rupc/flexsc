#include "flexsc.h"
#include <stdio.h>


/**
 * @brief Print sysentry information following order
 * sysnum, nargs, rstatus, sysret, args[0] ~ args[5]
 *
 * @param entry
 */
void print_sysentry(struct flexsc_sysentry *entry)
{
    printf("%5d %5d %5d %5d: %9lu %19lu %9lu %9lu %9lu %9lu\n",
            entry->sysnum, entry->nargs,
            entry->rstatus, entry->sysret,
            entry->args[0], entry->args[1],
            entry->args[2], entry->args[3],
            entry->args[4], entry->args[5]);
}
