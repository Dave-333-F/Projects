# Template-based library manager

Run `bin/library-cursor.exe` from the coursework root, or select Library Manager in
`bin/coursework-cursor.exe`. Build using `build-all.ps1` at the root.

## Controls

Use Up/Down to select an action on the right, then Enter to run it, or click the
action. Left/Right switches the Catalogue, Members and Help tabs. Esc goes back.
Normal text entry is restored for names, IDs and confirmation prompts. The letter
shortcuts below remain optional; menu navigation no longer requires numbers.

| Command | Action |
|---|---|
| Left/Right | Switch Catalogue, Members and Help tabs |
| `A` | Add a book with title and author |
| `M` | Register a member |
| `B` | Borrow using book ID and member ID |
| `R` | Return using book ID |
| `D` | Remove an available book, with confirmation |
| `X` | Remove a member with no active loans, with confirmation |
| `S`, `N` | Search by title/author/ID; clear search |
| `[`, `]` | Previous/next page |
| `V` | Save without another change |
| Esc | Return to the launcher or exit standalone |

Commands ignore letter case. Deletion requires typing the exact word `REMOVE`.
Titles, authors and member names support 1-120 printable ASCII characters.

## The template to explain to your lecturer

In `model.hpp`:

```cpp
template <typename T>
class Repository { /* insert, find, erase, nextId, all */ };

Repository<Book> books;
Repository<Member> members;
```

`T` is a placeholder for a record type with a positive integer `id` field. The
compiler makes a typed repository for books and another for members from one
implementation. Both use `std::map<int, T>` for unique IDs and ordered display.
This is actual template reuse, not merely the use of `std::vector` in a normal class.

`Library` applies rules on top of those repositories. A book cannot be borrowed
twice or assigned to an unknown member. Borrowed books and members with active
loans cannot be deleted. An available book cannot be returned a second time.

## Saving and loading

The default path is `data/library.txt` beside the executable (`bin/data/library.txt`
for the supplied build). It is independent of the current working directory.
The launcher and standalone app share it. For a separate test catalogue:

```powershell
.\bin\library-cursor.exe --data .\practice-library.txt
```

If there is no file, the app opens an illustrative catalogue in memory. It saves
only on the first successful change or `V`. Each successful edit saves automatically.

`storage.hpp` uses a versioned text format with quoted fields:

```text
STUDY_LIBRARY 1
M 1 "Samuel"
B 1 0 "Example Title" "Example Author"
```

`M` means member. A book record contains ID, borrower ID (`0` = available), title
and author. Quotes and backslashes are escaped with `std::quoted`. Duplicate IDs,
invalid values, unknown record types, unsupported headers, missing borrowers and
malformed records are rejected. A damaged file is reported and left untouched.
There is no automatic reset that could erase a damaged catalogue.

For an edit, `app.hpp` copies the model, validates the proposed change, saves the
copy and only then replaces the in-memory model. Saving writes/flushed bytes to a
unique temporary sibling and uses Windows file replacement. Failed saving cancels
the in-memory edit. This reduces partial-save risk; it is not a database, a backup
system or protection against disk failure. Use one process per catalogue file.

`Repository<T>` is general record storage. Its template operations are exposed for
teaching/testing; normal application changes go through `Library` to enforce loans.

This version tracks individual book copies. It does not yet implement passwords,
due dates, fines, ISBN lookups or concurrent users. Persistent storage targets Windows.
