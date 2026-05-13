#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"

PG_MODULE_MAGIC;

typedef struct
{
    int32 major;
    int32 minor;
    int32 patch;
} Semver;

/* ---------- helpers ---------- */

static int
semver_compare(const Semver *a, const Semver *b)
{
    if (a->major != b->major)
        return (a->major < b->major) ? -1 : 1;
    if (a->minor != b->minor)
        return (a->minor < b->minor) ? -1 : 1;
    if (a->patch != b->patch)
        return (a->patch < b->patch) ? -1 : 1;
    return 0;
}

static void
semver_input_error(const char *input)
{
    ereport(ERROR,
            (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
             errmsg("invalid input syntax for type semver: \"%s\"", input)));
}

/* ---------- I/O (Phase 1) ---------- */

PG_FUNCTION_INFO_V1(semver_in);
Datum
semver_in(PG_FUNCTION_ARGS)
{
    char   *input = PG_GETARG_CSTRING(0);
    Semver *result;
    int     matched;
    char    extra;

    /* Reject prerelease/build metadata (manual requirement) */
    if (strchr(input, '-') != NULL || strchr(input, '+') != NULL)
        semver_input_error(input);

    result = (Semver *) palloc(sizeof(Semver));

    /*
     * Parse exactly INT.INT.INT and reject trailing garbage.
     * " %c" catches extra characters including trailing spaces.
     */
    matched = sscanf(input, "%d.%d.%d %c",
                     &result->major,
                     &result->minor,
                     &result->patch,
                     &extra);

    if (matched != 3)
        semver_input_error(input);

    if (result->major < 0 || result->minor < 0 || result->patch < 0)
        semver_input_error(input);

    PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(semver_out);
Datum
semver_out(PG_FUNCTION_ARGS)
{
    Semver *input = (Semver *) PG_GETARG_POINTER(0);
    char   *result = (char *) palloc(32);

    snprintf(result, 32, "%d.%d.%d",
             input->major,
             input->minor,
             input->patch);

    PG_RETURN_CSTRING(result);
}

/* ---------- comparisons (Phase 2) ---------- */

PG_FUNCTION_INFO_V1(semver_lt);
Datum
semver_lt(PG_FUNCTION_ARGS)
{
    Semver *a = (Semver *) PG_GETARG_POINTER(0);
    Semver *b = (Semver *) PG_GETARG_POINTER(1);
    PG_RETURN_BOOL(semver_compare(a, b) < 0);
}

PG_FUNCTION_INFO_V1(semver_le);
Datum
semver_le(PG_FUNCTION_ARGS)
{
    Semver *a = (Semver *) PG_GETARG_POINTER(0);
    Semver *b = (Semver *) PG_GETARG_POINTER(1);
    PG_RETURN_BOOL(semver_compare(a, b) <= 0);
}

PG_FUNCTION_INFO_V1(semver_eq);
Datum
semver_eq(PG_FUNCTION_ARGS)
{
    Semver *a = (Semver *) PG_GETARG_POINTER(0);
    Semver *b = (Semver *) PG_GETARG_POINTER(1);
    PG_RETURN_BOOL(semver_compare(a, b) == 0);
}

PG_FUNCTION_INFO_V1(semver_ne);
Datum
semver_ne(PG_FUNCTION_ARGS)
{
    Semver *a = (Semver *) PG_GETARG_POINTER(0);
    Semver *b = (Semver *) PG_GETARG_POINTER(1);
    PG_RETURN_BOOL(semver_compare(a, b) != 0);
}

PG_FUNCTION_INFO_V1(semver_ge);
Datum
semver_ge(PG_FUNCTION_ARGS)
{
    Semver *a = (Semver *) PG_GETARG_POINTER(0);
    Semver *b = (Semver *) PG_GETARG_POINTER(1);
    PG_RETURN_BOOL(semver_compare(a, b) >= 0);
}

PG_FUNCTION_INFO_V1(semver_gt);
Datum
semver_gt(PG_FUNCTION_ARGS)
{
    Semver *a = (Semver *) PG_GETARG_POINTER(0);
    Semver *b = (Semver *) PG_GETARG_POINTER(1);
    PG_RETURN_BOOL(semver_compare(a, b) > 0);
}

PG_FUNCTION_INFO_V1(semver_cmp);
Datum
semver_cmp(PG_FUNCTION_ARGS)
{
    Semver *a = (Semver *) PG_GETARG_POINTER(0);
    Semver *b = (Semver *) PG_GETARG_POINTER(1);
    PG_RETURN_INT32(semver_compare(a, b));
}

/* ---------- utilities (Phase 2) ---------- */

PG_FUNCTION_INFO_V1(major);
Datum
major(PG_FUNCTION_ARGS)
{
    Semver *v = (Semver *) PG_GETARG_POINTER(0);
    PG_RETURN_INT32(v->major);
}

PG_FUNCTION_INFO_V1(minor);
Datum
minor(PG_FUNCTION_ARGS)
{
    Semver *v = (Semver *) PG_GETARG_POINTER(0);
    PG_RETURN_INT32(v->minor);
}

PG_FUNCTION_INFO_V1(patch);
Datum
patch(PG_FUNCTION_ARGS)
{
    Semver *v = (Semver *) PG_GETARG_POINTER(0);
    PG_RETURN_INT32(v->patch);
}

PG_FUNCTION_INFO_V1(bump_minor);
Datum
bump_minor(PG_FUNCTION_ARGS)
{
    Semver *v = (Semver *) PG_GETARG_POINTER(0);
    Semver *res = (Semver *) palloc(sizeof(Semver));

    res->major = v->major;
    res->minor = v->minor + 1;
    res->patch = 0;

    PG_RETURN_POINTER(res);
}

PG_FUNCTION_INFO_V1(is_compatible);
Datum
is_compatible(PG_FUNCTION_ARGS)
{
    Semver *a = (Semver *) PG_GETARG_POINTER(0);
    Semver *b = (Semver *) PG_GETARG_POINTER(1);

    if (a->major != b->major)
        PG_RETURN_BOOL(false);

    if (a->minor > b->minor)
        PG_RETURN_BOOL(true);
    if (a->minor < b->minor)
        PG_RETURN_BOOL(false);

    /* minors equal -> compare patch */
    PG_RETURN_BOOL(a->patch >= b->patch);
}
