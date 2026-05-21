CREATE TYPE semver;

CREATE FUNCTION semver_in(cstring)
RETURNS semver
AS 'MODULE_PATHNAME', 'semver_in'
LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION semver_out(semver)
RETURNS cstring
AS 'MODULE_PATHNAME', 'semver_out'
LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE semver (
  INPUT = semver_in,
  OUTPUT = semver_out,
  INTERNALLENGTH = 12,
  ALIGNMENT = int4
);

-- comparison functions
CREATE FUNCTION semver_lt(semver, semver) RETURNS bool
AS 'MODULE_PATHNAME','semver_lt' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION semver_le(semver, semver) RETURNS bool
AS 'MODULE_PATHNAME','semver_le' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION semver_eq(semver, semver) RETURNS bool
AS 'MODULE_PATHNAME','semver_eq' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION semver_ne(semver, semver) RETURNS bool
AS 'MODULE_PATHNAME','semver_ne' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION semver_ge(semver, semver) RETURNS bool
AS 'MODULE_PATHNAME','semver_ge' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION semver_gt(semver, semver) RETURNS bool
AS 'MODULE_PATHNAME','semver_gt' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION semver_cmp(semver, semver) RETURNS int4
AS 'MODULE_PATHNAME','semver_cmp' LANGUAGE C IMMUTABLE STRICT;

-- operators (FIXED)
CREATE OPERATOR <  (LEFTARG=semver, RIGHTARG=semver, PROCEDURE=semver_lt, COMMUTATOR='>',  NEGATOR='>=');
CREATE OPERATOR <= (LEFTARG=semver, RIGHTARG=semver, PROCEDURE=semver_le, COMMUTATOR='>=', NEGATOR='>');
CREATE OPERATOR =  (LEFTARG=semver, RIGHTARG=semver, PROCEDURE=semver_eq, COMMUTATOR='=',  NEGATOR='<>');
CREATE OPERATOR <> (LEFTARG=semver, RIGHTARG=semver, PROCEDURE=semver_ne, COMMUTATOR='<>', NEGATOR='=');
CREATE OPERATOR >= (LEFTARG=semver, RIGHTARG=semver, PROCEDURE=semver_ge, COMMUTATOR='<=', NEGATOR='<');
CREATE OPERATOR >  (LEFTARG=semver, RIGHTARG=semver, PROCEDURE=semver_gt, COMMUTATOR='<',  NEGATOR='<=');

-- utility functions
CREATE FUNCTION major(semver) RETURNS int4
AS 'MODULE_PATHNAME','major' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION minor(semver) RETURNS int4
AS 'MODULE_PATHNAME','minor' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION patch(semver) RETURNS int4
AS 'MODULE_PATHNAME','patch' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION bump_minor(semver) RETURNS semver
AS 'MODULE_PATHNAME','bump_minor' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION is_compatible(semver, semver) RETURNS bool
AS 'MODULE_PATHNAME','is_compatible' LANGUAGE C IMMUTABLE STRICT;
-- Aggregate helper functions
CREATE FUNCTION semver_larger(semver, semver)
RETURNS semver
AS 'MODULE_PATHNAME', 'semver_larger'
LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION semver_smaller(semver, semver)
RETURNS semver
AS 'MODULE_PATHNAME', 'semver_smaller'
LANGUAGE C IMMUTABLE STRICT;

-- Aggregates for max and min
CREATE AGGREGATE max(semver) (
    SFUNC = semver_larger,
    STYPE = semver
);

CREATE AGGREGATE min(semver) (
    SFUNC = semver_smaller,
    STYPE = semver
);

-- B-tree Operator Class for semver (Bonus)
CREATE OPERATOR CLASS semver_ops
  DEFAULT FOR TYPE semver USING btree AS
    OPERATOR 1  <  (semver, semver),
    OPERATOR 2  <= (semver, semver),
    OPERATOR 3  =  (semver, semver),
    OPERATOR 4  >= (semver, semver),
    OPERATOR 5  >  (semver, semver),
    FUNCTION 1  semver_cmp(semver, semver);