#include "postgres.h"
#include "fmgr.h"

PG_MODULE_MAGIC;

typedef struct {
    int32 major;
    int32 minor;
    int32 patch;
} Semver;

PG_FUNCTION_INFO_V1(semver_in);
Datum semver_in(PG_FUNCTION_ARGS)
{
    char *input = PG_GETARG_CSTRING(0);
    Semver *result = (Semver *) palloc(sizeof(Semver));
    int matched = sscanf(input, "%d.%d.%d",
                         &result->major,
                         &result->minor,
                         &result->patch);

    if (matched != 3)
    {
        ereport(ERROR,
                errmsg("invalid input syntax for type semver: \"%s\"", input));
    }

    PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(semver_out);
Datum semver_out(PG_FUNCTION_ARGS)
{
    Semver *input = (Semver *) PG_GETARG_POINTER(0);
    char *result = (char *) palloc(32);

    snprintf(result, 32, "%d.%d.%d",
             input->major,
             input->minor,
             input->patch);

    PG_RETURN_CSTRING(result);
}
