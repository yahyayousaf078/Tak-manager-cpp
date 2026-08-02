# Task Manager (C++)

A console-based task manager written in modern C++ (C++17). Supports multiple
projects/task-lists, categories/tags, priorities, deadlines, and persistent
file storage — no external dependencies required.

## Features

- ✅ Add, view, complete, and delete tasks
- 🚦 Set task priority: **Low / Medium / High**
- 📅 Add or update a deadline (`YYYY-MM-DD`)
- 🏷️ Categories/tags for tasks (e.g. `Work`, `Personal`, `Shopping`)
- 📁 Multiple projects — each is its own independent task list, saved to its own file
- 🔎 Search tasks by title (case-insensitive)
- ↕️ Sort tasks by priority or by deadline
- 💾 Data persists automatically between sessions (plain text files, human-readable)

## Project Structure

```
task-manager-cpp/
├── src/
│   └── task_manager.cpp   # all source code
├── Makefile                # build / run / clean shortcuts
├── LICENSE
├── .gitignore
└── README.md
```

Each project you create is saved as `projects/<ProjectName>.txt` (this folder
is created automatically on first run and is git-ignored, since it's user
data, not source code).

## Concepts demonstrated

- Classes & objects (`Task`, `Project`, `ProjectManager`)
- `enum class` for `Priority` and `Status`
- `std::vector` and `std::string`
- File handling with `<fstream>`
- Sorting with `std::sort` and custom comparators
- Searching with STL/loops
- `std::filesystem` for discovering and managing project files

## Build & Run

### Option 1: using `make`
```bash
make        # builds the binary
make run    # builds (if needed) and runs it
make clean  # removes the binary and saved project data
```

### Option 2: compile directly
```bash
g++ -std=c++17 -Wall -o task_manager src/task_manager.cpp
./task_manager
```

> Requires a C++17-compatible compiler (GCC 8+, Clang 7+, or MSVC 2019+).
> On Windows, run `task_manager.exe` instead of `./task_manager`.

## Usage

On launch you'll land on the **project menu**:

```
========== TASK MANAGER ==========
 1. Open a project
 2. Create a new project
 3. Delete a project
 4. List all projects
 0. Exit
===================================
```

Create a project (e.g. `Work`), open it, and you'll get the task menu:

```
========== PROJECT: Work ==========
 1. Add a task
 2. View all tasks
 3. Mark a task as completed
 4. Delete a task
 5. Set task priority
 6. Set/change task deadline
 7. Search tasks by title
 8. Sort tasks by priority
 9. Sort tasks by deadline
10. Set/change task category
11. View tasks by category
12. List all categories
 0. Save and return to project menu
=========================================
```

Every change is saved to disk immediately, so you can close the program at
any time without losing data.

## Data format

Each task is stored as one line, pipe-delimited:

```
id|title|deadline|priority|status|category
```

Example (`projects/Work.txt`):
```
1|Finish slides|2026-08-10|High|Pending|Presentation
2|Email client|2026-08-04|Medium|Pending|Communication
```

## Roadmap / possible next steps

- [ ] Real date validation + overdue warnings (`<ctime>`/`<chrono>`)
- [ ] Undo last action
- [ ] Colored console output
- [ ] Recurring tasks

## License

Released under the [MIT License](LICENSE).
