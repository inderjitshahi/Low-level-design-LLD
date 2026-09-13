#include <iostream>
#include <unordered_map>
#include <memory>
#include <utility>
#include <vector>
#include <string>
using namespace std;

enum class FileSystemEntityType
{
    FILE = 0,
    DIRECTORY
};

// base class
class FileSystemEntity
{
protected:
    // accessible in derived class
    string name;

public:
    FileSystemEntity(const string &name) : name(name) {}

    const string &getName() const
    {
        return name;
    }

    virtual FileSystemEntityType getType() const = 0;
    virtual int getSize() const = 0;
    virtual void print(int indent = 0) const = 0;

    virtual ~FileSystemEntity() = default;
};

class File : public FileSystemEntity
{
private:
    string content;

public:
    File(string name) : FileSystemEntity(move(name)) {}

    FileSystemEntityType getType() const override
    {
        return FileSystemEntityType::FILE;
    }

    int getSize() const override
    {
        return content.size();
    }

    void write(const string &data)
    {
        content = data;
    }

    void append(const string &data)
    {
        content += data;
    }

    const string &read() const
    {
        return content;
    }

    void print(int indent = 0) const override
    {
        cout << string(indent, ' ') << "- " << name << " (" << getSize() << " bytes)" << "\n";
    }
};

// ######################## Directory #################################
class Directory : public FileSystemEntity
{
private:
    unordered_map<string, unique_ptr<FileSystemEntity>> children;

public:
    Directory(string name) : FileSystemEntity(name) {}

    FileSystemEntityType getType() const override
    {
        return FileSystemEntityType::DIRECTORY;
    }

    int getSize() const override
    {
        int totalSize = 0;
        for (const auto &entry : children)
        {
            totalSize += entry.second->getSize();
        }
        return totalSize;
    }

    void add(unique_ptr<FileSystemEntity> entity)
    {
        string entityName = entity->getName();
        auto it = children.find(entityName);
        if (it != children.end())
        {
            throw runtime_error("File or directory already exists");
        }

        children.emplace(entityName, move(entity));
    }

    void remove(const string &name)
    {
        auto it = children.find(name);
        if (it == children.end())
        {
            throw runtime_error("File or directory not found");
        }
        children.erase(it);
    }

    FileSystemEntity *get(const string &name)
    {
        auto it = children.find(name);
        if (it == children.end())
        {
            return nullptr;
        }

        // unique_ptr<T> separates lifetime management (who deletes it) from access rights (who can read/modify it).
        return it->second.get(); // returns the raw pointer (T*) to the managed resource without transferring ownership. children still belongs to directory
    }

    void print(int indent = 0) const override
    {
        cout << string(indent, ' ')
             << "+ " << name << "/"
             << '\n';

        for (const auto &entry : children)
        {
            entry.second->print(indent + 2);
        }
    }
};

// ---------------------- File System ------------------------------------//
class FileSystem
{
private:
    unique_ptr<Directory> root = make_unique<Directory>("root"); // root directory

    vector<string> splitPath(const string &path) const
    {
        vector<string> parts;
        string current = "";
        for (char ch : path)
        {
            if (ch == '/')
            {
                if (!current.empty())
                {
                    parts.push_back(current);
                    current.clear();
                }
            }
            else
            {
                current += ch;
            }
        }

        if (!current.empty())
        {
            parts.push_back(current);
        }

        return parts;
    }

    Directory *getDirectory(const vector<string> &parts, int count)
    {
        Directory *current = root.get(); // row pointer of root
        for (int i = 0; i < count; i++)
        {
            FileSystemEntity *entity = current->get(parts[i]);
            if (!entity)
                return nullptr;

            if (entity->getType() != FileSystemEntityType::DIRECTORY)
            {
                return nullptr;
            }

            current = static_cast<Directory *>(entity);
        }

        return current;
    }

public:
    void mkdir(const string &path)
    {
        vector<string> parts = splitPath(path);
        if (parts.empty())
        {
            return;
        }

        Directory *parent = getDirectory(parts, parts.size() - 1);

        if (!parent)
        {
            throw runtime_error("Parent directory not found");
        }

        parent->add(make_unique<Directory>(parts.back()));
    }

    void createFile(const string &path)
    {
        vector<string> parts = splitPath(path);

        if (parts.empty())
        {
            throw runtime_error("Invalid path.\n");
        }

        Directory *parent = getDirectory(parts, parts.size() - 1);
        if (!parent)
        {
            throw runtime_error("Parent directory not found.\n");
        }
        parent->add(make_unique<File>(parts.back()));
    }

    FileSystemEntity *getEntity(const string &path)
    {
        vector<string> parts = splitPath(path);
        if (parts.empty())
        {
            return root.get();
        }

        Directory *current = root.get();

        for (int i = 0; i < parts.size(); i++)
        {
            FileSystemEntity *entity = current->get(parts[i]);
            if (!entity)
            {
                return nullptr;
            }

            if (i == parts.size() - 1)
            {
                return entity;
            }

            if (entity->getType() != FileSystemEntityType::DIRECTORY)
            {
                return nullptr;
            }

            current = static_cast<Directory *>(entity);
        }
        return nullptr;
    }

    void writeFile(const string &path, const string &content)
    {
        FileSystemEntity *entity = getEntity(path);
        if (!entity or entity->getType() != FileSystemEntityType::FILE)
        {
            throw runtime_error("File not found.\n");
        }

        File *file = static_cast<File *>(entity);
        file->write(content);
    }

    string readFile(const string &path)
    {
        FileSystemEntity *entity =
            getEntity(path);

        if (!entity ||
            entity->getType() !=
                FileSystemEntityType::FILE)
        {
            throw runtime_error(
                "File not found");
        }

        File *file =
            static_cast<File *>(entity);

        return file->read();
    }
    void remove(const string &path)
    {
        vector<string> parts = splitPath(path);
        if (parts.empty())
        {
            throw runtime_error("Can not delete root.\n");
        }
        Directory *parent = getDirectory(parts, parts.size() - 1);
        if (!parent)
        {
            throw runtime_error("Parent directory not found.\n");
        }

        parent->remove(parts.back());
    }

    void mkdir_p(const string &path)
    {
        vector<string> parts = splitPath(path);
        if (parts.empty())
        {
            throw runtime_error("Empty directory cant be created!\n");
        }

        Directory *current = root.get();
        for (int i = 0; i < parts.size(); i++)
        {
            FileSystemEntity *entity = current->get(parts[i]);
            if (!entity)
            {
                auto newDir = make_unique<Directory>(parts[i]);
                Directory *dirPtr = newDir.get();
                current->add(move(newDir));
                current = dirPtr;
            }
            else
            {
                // Verify entity is actually a directory, not a file
                Directory *dirPtr = dynamic_cast<Directory *>(entity);
                if (!dirPtr)
                {
                    throw runtime_error("Path segment '" + parts[i] + "' is not a directory.");
                }
                current = dirPtr;
            }
        }
    }

    void print()
    {
        root->print();
    }

    // mkdirRecursive() can be added
};

int main()
{
    FileSystem fs;
    fs.mkdir("/home");
    fs.mkdir("/home/user");
    fs.mkdir("/home/user/docs");

    fs.createFile(
        "/home/user/docs/test.txt");

    fs.writeFile(
        "/home/user/docs/test.txt",
        "Hello World");

    fs.createFile(
        "/home/user/docs/notes.txt");

    fs.writeFile(
        "/home/user/docs/notes.txt",
        "LLD practice");

    cout << fs.readFile(
                "/home/user/docs/test.txt")
         << "\n\n";

    fs.print();

    cout << "\nTotal root size: "
         << fs.getEntity("/")->getSize()
         << " bytes\n";

    fs.remove(
        "/home/user/docs/notes.txt");

    cout << "\nAfter deletion:\n";

    fs.print();

    return 0;
}