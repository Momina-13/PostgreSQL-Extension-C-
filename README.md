# PostgreSQL-Extension-C-
# semver — PostgreSQL Semantic Version Extension

A custom PostgreSQL extension written in C that adds a `semver` data type for storing and working with semantic version numbers like `1.2.3`, `16.4.0`, and `20.10.0`.

---

## What Is This?

PostgreSQL does not understand version numbers by default. If you store `1.10.0` and `1.9.0` as plain text and try to compare them, the database gets it wrong because text comparison is alphabetical, not numeric.

This extension teaches PostgreSQL what a version number is — so it stores, displays, and compares them correctly.

---

## System Requirements

- Ubuntu / Debian Linux (or WSL on Windows)
- PostgreSQL 16
- The following packages:

```bash
sudo apt update
sudo apt install postgresql postgresql-server-dev-all build-essential
```

---

## Project Structure

```
semver/
├── semver.c          # C source code — the brain of the extension
├── semver.control    # Metadata file so PostgreSQL recognizes the extension
├── semver--1.0.sql   # SQL registration — tells PostgreSQL about your C functions
└── Makefile          # Build instructions
```

---

## How to Build and Install

### Step 1: Start PostgreSQL
```bash
sudo service postgresql start
```

### Step 2: Compile the extension
```bash
make
```

### Step 3: Install the extension
```bash
sudo make install
```

### Step 4: Open your database
```bash
psql mydb
```

### Step 5: Load the extension
```sql
CREATE EXTENSION semver;
```

---

## Supported Features (Phase 1)

### Store and display version numbers
```sql
SELECT '1.2.3'::semver;
-- Result: 1.2.3
```

### Store versions in a table column
```sql
CREATE TABLE packages (
    name    text,
    version semver
);

INSERT INTO packages VALUES ('postgres', '16.4.0');
INSERT INTO packages VALUES ('nginx',    '1.27.1');

SELECT * FROM packages;
```

### Reject invalid input
```sql
SELECT '1.2.x'::semver;
-- ERROR: invalid input syntax for type semver: "1.2.x"
```

---

## How It Works Internally

A `semver` value is stored as 3 separate integers in memory:

```
'1.2.3'  →  { major=1, minor=2, patch=3 }
```

This is 12 bytes total (3 × 4 bytes). When displayed, the 3 numbers are formatted back into `1.2.3`.

Two C functions handle this:

| Function | Job |
|---|---|
| `semver_in` | Takes text like `"1.2.3"` → stores as 3 integers |
| `semver_out` | Takes 3 integers → formats back as `"1.2.3"` |

---

## Known Limitations (Phase 1)

- No comparison operators yet (`<`, `>`, `=`, etc.)
- No sorting support yet
- No `MAX` / `MIN` aggregate support yet
- Pre-release labels (`1.0.0-rc.1`) are not supported
- Build metadata (`1.0.0+abc`) is not supported

These will be added in Phase 2 and Phase 3.

---

## Authors

- Course: Advanced Database Management — CS 4th Semester
- Project: 02 — PostgreSQL Extension in C
