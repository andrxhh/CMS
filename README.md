**Overview**

- **Project**: A simple command-line Course Management System (CMS) for storing student records.

**Build**

- **Compiler**: `gcc` (MinGW/MSYS or other gcc toolchain on Windows).
- **Command**: Run this from the project root:

```
gcc src/*.c -Iinclude/ -o cms.exe
```

**Run**

- **Interactive**: launch the program and type commands at the prompt:

```
.\cms.exe
```

- **Batch / scripted**: feed a file of commands into the program:

```
cms.exe < test_inputs_full.txt
```

**Main Features**

- **Insert**: add a student record with `ID`, `Name`, `Programme`, and `Mark`.
- **Update**: update an existing record by `ID` with key=value patches.
- **Delete**: remove a record (with confirmation prompt).
- **Show**: display all student records.
- **Find / Query**: search records by field(s) and support basic filtering.
- **Sort**: sort the stored records by fields.
- **Save / Load (Open)**: persist and restore the database to/from a text file.
- **Help**: built-in usage hints for commands.
- **Exit**: prompts to save unsaved changes before quitting.

**Notes & Defensive Behavior**

- The parser accepts `key=value` pairs; values may be quoted to include spaces or `=` characters.
- Numeric fields (like `ID` and `Mark`) are validated; invalid inputs report helpful errors.
- Long strings are rejected with a clear message when they exceed field limits.
