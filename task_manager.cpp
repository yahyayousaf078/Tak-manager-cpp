// ============================================================
//  Task Manager (Console App) - Version 2 (Advanced)
//  New in this version:
//    - Categories/tags for tasks (e.g. "Work", "Personal")
//    - Multiple projects/task-lists, each saved to its own file
//  Concepts used: classes/objects, vector, string, functions,
//                 file handling (fstream), sort(), STL search,
//                 enums for priority/status, std::filesystem.
// ============================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <limits>
#include <filesystem>

namespace fs = std::filesystem;

// ------------------------------------------------------------
// Enums
// ------------------------------------------------------------
enum class Priority { LOW = 0, MEDIUM = 1, HIGH = 2 };
enum class Status   { PENDING = 0, COMPLETED = 1 };

std::string priorityToString(Priority p) {
    switch (p) {
        case Priority::LOW:    return "Low";
        case Priority::MEDIUM: return "Medium";
        case Priority::HIGH:   return "High";
    }
    return "Unknown";
}

Priority stringToPriority(const std::string& s) {
    if (s == "Low")    return Priority::LOW;
    if (s == "Medium") return Priority::MEDIUM;
    if (s == "High")   return Priority::HIGH;
    return Priority::LOW; // safe default
}

std::string statusToString(Status s) {
    return (s == Status::COMPLETED) ? "Completed" : "Pending";
}

Status stringToStatus(const std::string& s) {
    return (s == "Completed") ? Status::COMPLETED : Status::PENDING;
}

// Small helper: lowercase a string (used for case-insensitive search).
std::string toLower(const std::string& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(), ::tolower);
    return out;
}

// ------------------------------------------------------------
// Task class
// ------------------------------------------------------------
class Task {
public:
    int id;
    std::string title;
    std::string deadline;   // "YYYY-MM-DD" or empty
    std::string category;   // e.g. "Work", "Personal", "Shopping" (empty = Uncategorized)
    Priority priority;
    Status status;

    Task()
        : id(0), title(""), deadline(""), category(""),
          priority(Priority::LOW), status(Status::PENDING) {}

    Task(int id_, std::string title_, std::string deadline_,
         Priority priority_, std::string category_)
        : id(id_), title(std::move(title_)), deadline(std::move(deadline_)),
          category(std::move(category_)), priority(priority_), status(Status::PENDING) {}

    // Serialize one task to a single line for file storage.
    // Format: id|title|deadline|priority|status|category
    // ('|' is used as a delimiter, so titles/categories should avoid it)
    std::string serialize() const {
        std::ostringstream oss;
        oss << id << '|' << title << '|' << deadline << '|'
            << priorityToString(priority) << '|' << statusToString(status)
            << '|' << category;
        return oss.str();
    }

    // Rebuild a Task from a stored line.
    static Task deserialize(const std::string& line) {
        std::vector<std::string> fields;
        std::stringstream ss(line);
        std::string field;
        while (std::getline(ss, field, '|')) {
            fields.push_back(field);
        }
        // Pad missing fields so files saved by v1 (no category) still load.
        while (fields.size() < 6) fields.push_back("");

        Task t;
        t.id = std::stoi(fields[0].empty() ? "0" : fields[0]);
        t.title = fields[1];
        t.deadline = fields[2];
        t.priority = stringToPriority(fields[3]);
        t.status = stringToStatus(fields[4]);
        t.category = fields[5];
        return t;
    }

    void display() const {
        std::cout << "[" << id << "] "
                  << (status == Status::COMPLETED ? "[X] " : "[ ] ")
                  << title
                  << "  | Priority: " << priorityToString(priority)
                  << "  | Deadline: " << (deadline.empty() ? "None" : deadline)
                  << "  | Category: " << (category.empty() ? "Uncategorized" : category)
                  << "  | Status: " << statusToString(status)
                  << std::endl;
    }
};

// ------------------------------------------------------------
// Project class
//   A Project is one independent task list backed by its own file.
//   e.g. "Work.txt", "Personal.txt"
// ------------------------------------------------------------
class Project {
private:
    std::vector<Task> tasks;
    std::string filepath;
    int nextId;

    int findIndexById(int id) const {
        for (size_t i = 0; i < tasks.size(); ++i) {
            if (tasks[i].id == id) return static_cast<int>(i);
        }
        return -1;
    }

public:
    std::string name;

    Project(std::string name_, std::string filepath_)
        : filepath(std::move(filepath_)), nextId(1), name(std::move(name_)) {
        loadFromFile();
    }

    // ---------------- Add ----------------
    void addTask(const std::string& title, const std::string& deadline,
                 Priority priority, const std::string& category) {
        Task t(nextId++, title, deadline, priority, category);
        tasks.push_back(t);
        std::cout << "Task added with ID " << t.id << ".\n";
    }

    // ---------------- View ----------------
    void viewTasks() const {
        if (tasks.empty()) {
            std::cout << "No tasks found in this project.\n";
            return;
        }
        std::cout << "\n----- " << name << " (" << tasks.size() << " tasks) -----\n";
        for (const auto& t : tasks) t.display();
        std::cout << "-------------------------------\n";
    }

    // View tasks belonging to one category only.
    void viewByCategory(const std::string& cat) const {
        std::string target = toLower(cat);
        bool found = false;
        std::cout << "\n----- Category: " << cat << " -----\n";
        for (const auto& t : tasks) {
            if (toLower(t.category) == target) {
                t.display();
                found = true;
            }
        }
        if (!found) std::cout << "No tasks in this category.\n";
        std::cout << "-------------------------------\n";
    }

    // List distinct categories currently in use.
    std::vector<std::string> listCategories() const {
        std::vector<std::string> cats;
        for (const auto& t : tasks) {
            std::string c = t.category.empty() ? "Uncategorized" : t.category;
            if (std::find(cats.begin(), cats.end(), c) == cats.end()) {
                cats.push_back(c);
            }
        }
        std::sort(cats.begin(), cats.end());
        return cats;
    }

    // ---------------- Mark completed ----------------
    bool markCompleted(int id) {
        int idx = findIndexById(id);
        if (idx == -1) return false;
        tasks[idx].status = Status::COMPLETED;
        return true;
    }

    // ---------------- Delete ----------------
    bool deleteTask(int id) {
        int idx = findIndexById(id);
        if (idx == -1) return false;
        tasks.erase(tasks.begin() + idx);
        return true;
    }

    // ---------------- Set priority ----------------
    bool setPriority(int id, Priority priority) {
        int idx = findIndexById(id);
        if (idx == -1) return false;
        tasks[idx].priority = priority;
        return true;
    }

    // ---------------- Set / change deadline ----------------
    bool setDeadline(int id, const std::string& deadline) {
        int idx = findIndexById(id);
        if (idx == -1) return false;
        tasks[idx].deadline = deadline;
        return true;
    }

    // ---------------- Set / change category ----------------
    bool setCategory(int id, const std::string& category) {
        int idx = findIndexById(id);
        if (idx == -1) return false;
        tasks[idx].category = category;
        return true;
    }

    // ---------------- Search by title (substring, case-insensitive) ----------------
    std::vector<Task> searchByTitle(const std::string& keyword) const {
        std::vector<Task> results;
        std::string lowerKeyword = toLower(keyword);
        for (const auto& t : tasks) {
            if (toLower(t.title).find(lowerKeyword) != std::string::npos) {
                results.push_back(t);
            }
        }
        return results;
    }

    // ---------------- Sort by priority (High -> Low) ----------------
    void sortByPriority() {
        std::sort(tasks.begin(), tasks.end(), [](const Task& a, const Task& b) {
            return static_cast<int>(a.priority) > static_cast<int>(b.priority);
        });
        std::cout << "Tasks sorted by priority (High -> Low).\n";
    }

    // ---------------- Sort by deadline (earliest first) ----------------
    void sortByDeadline() {
        std::sort(tasks.begin(), tasks.end(), [](const Task& a, const Task& b) {
            if (a.deadline.empty()) return false;
            if (b.deadline.empty()) return true;
            return a.deadline < b.deadline;
        });
        std::cout << "Tasks sorted by deadline (earliest first).\n";
    }

    // ---------------- File I/O ----------------
    void saveToFile() const {
        std::ofstream outFile(filepath, std::ios::trunc);
        if (!outFile) {
            std::cerr << "Error: could not open file for saving.\n";
            return;
        }
        for (const auto& t : tasks) outFile << t.serialize() << '\n';
    }

    void loadFromFile() {
        std::ifstream inFile(filepath);
        if (!inFile) return; // new project, no file yet
        tasks.clear();
        std::string line;
        int maxId = 0;
        while (std::getline(inFile, line)) {
            if (line.empty()) continue;
            Task t = Task::deserialize(line);
            tasks.push_back(t);
            maxId = std::max(maxId, t.id);
        }
        nextId = maxId + 1;
    }
};

// ------------------------------------------------------------
// ProjectManager class
//   Discovers, creates, and deletes projects. Each project is
//   one file inside the "projects" directory.
// ------------------------------------------------------------
class ProjectManager {
private:
    fs::path dataDir;

public:
    explicit ProjectManager(const std::string& dirName) : dataDir(dirName) {
        if (!fs::exists(dataDir)) {
            fs::create_directories(dataDir);
        }
    }

    // Returns sorted list of existing project names (derived from filenames).
    std::vector<std::string> listProjects() const {
        std::vector<std::string> names;
        for (const auto& entry : fs::directory_iterator(dataDir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                names.push_back(entry.path().stem().string());
            }
        }
        std::sort(names.begin(), names.end());
        return names;
    }

    bool projectExists(const std::string& name) const {
        return fs::exists(dataDir / (name + ".txt"));
    }

    // Opens (or creates) a project by name and returns it.
    Project openProject(const std::string& name) const {
        return Project(name, (dataDir / (name + ".txt")).string());
    }

    bool deleteProject(const std::string& name) const {
        fs::path p = dataDir / (name + ".txt");
        if (!fs::exists(p)) return false;
        return fs::remove(p);
    }
};

// ------------------------------------------------------------
// Console UI helpers
// ------------------------------------------------------------
int readIntChoice() {
    int choice;
    while (!(std::cin >> choice)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid input. Enter a number: ";
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return choice;
}

std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    return line;
}

Priority readPriorityChoice() {
    std::cout << "Priority (1=Low, 2=Medium, 3=High): ";
    int choice = readIntChoice();
    switch (choice) {
        case 1: return Priority::LOW;
        case 3: return Priority::HIGH;
        default: return Priority::MEDIUM;
    }
}

int readIdPrompt(const std::string& prompt) {
    std::cout << prompt;
    return readIntChoice();
}

void printProjectMenu(const std::string& projectName) {
    std::cout << "\n========== PROJECT: " << projectName << " ==========\n"
              << " 1. Add a task\n"
              << " 2. View all tasks\n"
              << " 3. Mark a task as completed\n"
              << " 4. Delete a task\n"
              << " 5. Set task priority\n"
              << " 6. Set/change task deadline\n"
              << " 7. Search tasks by title\n"
              << " 8. Sort tasks by priority\n"
              << " 9. Sort tasks by deadline\n"
              << "10. Set/change task category\n"
              << "11. View tasks by category\n"
              << "12. List all categories\n"
              << " 0. Save and return to project menu\n"
              << "=========================================\n"
              << "Choose an option: ";
}

// Runs the full task menu loop for one open project.
void runProjectMenu(Project& project) {
    bool inProject = true;
    while (inProject) {
        printProjectMenu(project.name);
        int choice = readIntChoice();

        switch (choice) {
            case 1: {
                std::string title = readLine("Enter task title: ");
                std::string deadline = readLine("Enter deadline (YYYY-MM-DD, or leave blank): ");
                Priority p = readPriorityChoice();
                std::string category = readLine("Enter category/tag (or leave blank): ");
                project.addTask(title, deadline, p, category);
                project.saveToFile();
                break;
            }
            case 2:
                project.viewTasks();
                break;
            case 3: {
                int id = readIdPrompt("Enter task ID to mark completed: ");
                if (project.markCompleted(id)) {
                    std::cout << "Task marked as completed.\n";
                    project.saveToFile();
                } else {
                    std::cout << "Task ID not found.\n";
                }
                break;
            }
            case 4: {
                int id = readIdPrompt("Enter task ID to delete: ");
                if (project.deleteTask(id)) {
                    std::cout << "Task deleted.\n";
                    project.saveToFile();
                } else {
                    std::cout << "Task ID not found.\n";
                }
                break;
            }
            case 5: {
                int id = readIdPrompt("Enter task ID to update priority: ");
                Priority p = readPriorityChoice();
                if (project.setPriority(id, p)) {
                    std::cout << "Priority updated.\n";
                    project.saveToFile();
                } else {
                    std::cout << "Task ID not found.\n";
                }
                break;
            }
            case 6: {
                int id = readIdPrompt("Enter task ID to update deadline: ");
                std::string deadline = readLine("Enter new deadline (YYYY-MM-DD): ");
                if (project.setDeadline(id, deadline)) {
                    std::cout << "Deadline updated.\n";
                    project.saveToFile();
                } else {
                    std::cout << "Task ID not found.\n";
                }
                break;
            }
            case 7: {
                std::string keyword = readLine("Enter keyword to search: ");
                auto results = project.searchByTitle(keyword);
                if (results.empty()) {
                    std::cout << "No matching tasks found.\n";
                } else {
                    std::cout << "\n----- Search Results (" << results.size() << ") -----\n";
                    for (const auto& t : results) t.display();
                }
                break;
            }
            case 8:
                project.sortByPriority();
                project.saveToFile();
                project.viewTasks();
                break;
            case 9:
                project.sortByDeadline();
                project.saveToFile();
                project.viewTasks();
                break;
            case 10: {
                int id = readIdPrompt("Enter task ID to update category: ");
                std::string category = readLine("Enter new category/tag: ");
                if (project.setCategory(id, category)) {
                    std::cout << "Category updated.\n";
                    project.saveToFile();
                } else {
                    std::cout << "Task ID not found.\n";
                }
                break;
            }
            case 11: {
                std::string category = readLine("Enter category to view: ");
                project.viewByCategory(category);
                break;
            }
            case 12: {
                auto cats = project.listCategories();
                if (cats.empty()) {
                    std::cout << "No categories yet.\n";
                } else {
                    std::cout << "\nCategories in use:\n";
                    for (const auto& c : cats) std::cout << "  - " << c << "\n";
                }
                break;
            }
            case 0:
                project.saveToFile();
                std::cout << "Project saved.\n";
                inProject = false;
                break;
            default:
                std::cout << "Invalid option. Please try again.\n";
        }
    }
}

void printProjectSelectMenu() {
    std::cout << "\n========== TASK MANAGER ==========\n"
              << " 1. Open a project\n"
              << " 2. Create a new project\n"
              << " 3. Delete a project\n"
              << " 4. List all projects\n"
              << " 0. Exit\n"
              << "===================================\n"
              << "Choose an option: ";
}

// ------------------------------------------------------------
// main
// ------------------------------------------------------------
int main() {
    ProjectManager manager("projects");
    bool running = true;

    std::cout << "Welcome to Task Manager (Advanced)!\n";
    std::cout << "Each project is its own task list, saved separately.\n";

    while (running) {
        auto projects = manager.listProjects();

        printProjectSelectMenu();
        int choice = readIntChoice();

        switch (choice) {
            case 1: {
                if (projects.empty()) {
                    std::cout << "No projects yet. Create one first.\n";
                    break;
                }
                std::cout << "\nAvailable projects:\n";
                for (size_t i = 0; i < projects.size(); ++i) {
                    std::cout << "  " << (i + 1) << ". " << projects[i] << "\n";
                }
                std::cout << "Enter project number: ";
                int idx = readIntChoice();
                if (idx < 1 || idx > static_cast<int>(projects.size())) {
                    std::cout << "Invalid project number.\n";
                    break;
                }
                Project project = manager.openProject(projects[idx - 1]);
                runProjectMenu(project);
                break;
            }
            case 2: {
                std::string name = readLine("Enter new project name: ");
                if (name.empty()) {
                    std::cout << "Project name cannot be empty.\n";
                    break;
                }
                if (manager.projectExists(name)) {
                    std::cout << "A project with that name already exists.\n";
                    break;
                }
                Project project = manager.openProject(name); // creates it on first save
                project.saveToFile();
                std::cout << "Project \"" << name << "\" created.\n";
                std::cout << "Enter it now? (1 = Yes, 0 = No): ";
                if (readIntChoice() == 1) {
                    runProjectMenu(project);
                }
                break;
            }
            case 3: {
                if (projects.empty()) {
                    std::cout << "No projects to delete.\n";
                    break;
                }
                std::cout << "\nAvailable projects:\n";
                for (size_t i = 0; i < projects.size(); ++i) {
                    std::cout << "  " << (i + 1) << ". " << projects[i] << "\n";
                }
                std::cout << "Enter project number to delete: ";
                int idx = readIntChoice();
                if (idx < 1 || idx > static_cast<int>(projects.size())) {
                    std::cout << "Invalid project number.\n";
                    break;
                }
                std::string name = projects[idx - 1];
                std::cout << "Are you sure you want to delete \"" << name
                          << "\"? This cannot be undone. (1 = Yes, 0 = No): ";
                if (readIntChoice() == 1) {
                    if (manager.deleteProject(name)) {
                        std::cout << "Project deleted.\n";
                    } else {
                        std::cout << "Could not delete project.\n";
                    }
                }
                break;
            }
            case 4: {
                if (projects.empty()) {
                    std::cout << "No projects yet.\n";
                } else {
                    std::cout << "\nProjects:\n";
                    for (const auto& p : projects) std::cout << "  - " << p << "\n";
                }
                break;
            }
            case 0:
                std::cout << "Goodbye!\n";
                running = false;
                break;
            default:
                std::cout << "Invalid option. Please try again.\n";
        }
    }

    return 0;
}
