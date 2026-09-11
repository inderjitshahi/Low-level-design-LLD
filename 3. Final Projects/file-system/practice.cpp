#include <iostream>
#include <memory>
#include <utility>
#include <unordered_map>
#include <vector>
using namespace std;

enum class FileSystemEntityType
{
    FILE = 0,
    DIRECTORY
};

class FileSystemEntity
{
protected:
    string name;

public:
    FileSystemEntity(const string &name) : name(name) {}
    string getName() const
    {
        return name;
    }
    virtual FileSystemEntityType getFileType() const = 0;
    virtual int getSize() const = 0;
    virtual void print(int indent = 0) const = 0;
    virtual ~FileSystemEntity() = default;
};

class File : public FileSystemEntity
{
    string content;

public:
    File(const string &name) : FileSystemEntity(name) {}

    FileSystemEntityType getFileType() const override
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

class Directory : public FileSystemEntity
{
private:
    unordered_map<string, unique_ptr<FileSystemEntity>> children;

public:
    Directory(const string &name) : FileSystemEntity(name) {}

    FileSystemEntityType getFileType() const override
    {
        return FileSystemEntityType::DIRECTORY;
    }

    int getSize() const override
    {
        int totalSize = 0;
        for (const auto &child : children)
        {
            totalSize += child.second->getSize();
        }
        return totalSize;
    }

    void add(unique_ptr<FileSystemEntity> entity)
    {
        auto it = children.find(entity->getName());
        if (it == children.end())
        {
            children.emplace(entity->getName(), move(entity));
        }
    }

    void remove(const string &name)
    {
        auto it = children.find(name);
        if (it != children.end())
        {
            children.erase(name);
        }
    }

    FileSystemEntity *get(const string &name) const
    {
        auto it = children.find(name);
        if (it != children.end())
        {
            return it->second.get(); // can't return the unique pointer as will trigger copy
        }
        return nullptr;
    }

    void print(int indent = 0) const override
    {
        cout << string(indent, ' ')
             << "+ " << name << "/"
             << '\n';
        for (const auto &child : children)
        {
            child.second->print(indent + 2);
        }
    }
};

class FileSystem
{
private:
    unique_ptr<Directory> root = make_unique<Directory>("root");

    vector<string> splitPath(const string &path)
    {
        vector<string> paths;
        string cur = "";
        for (char ch : path)
        {
            if (ch == '/' and cur.size() > 0)
            {
                paths.push_back(cur);
                cur.clear();
            }
            else
            {
                cur += ch;
            }
        }

        if (!cur.empty())
        {
            paths.push_back(cur);
        }
        return paths;
    }

    Directory *getDirectory(const vector<string> &paths, int cnt)
    {
        Directory *current = root.get();

        for (int i = 0; i < cnt; i++)
        {
            FileSystemEntity *entity = current->get(paths[i]);
            if (entity == nullptr)
                return nullptr;

            if (entity->getFileType() != FileSystemEntityType::DIRECTORY)
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
        auto parts = splitPath(path);
        if (parts.empty())
            return;
        Directory *parent = getDirectory(parts, parts.size() - 1);
        if (!parent)
            return;
        parent->add(make_unique<Directory>(parts.back()));
    }

    void createFile(const string &path)
    {
        auto parts = splitPath(path);
        if (parts.empty())
            return;
        Directory *parent = getDirectory(parts, parts.size() - 1);
        if (!parent)
            return;
        parent->add(make_unique<File>(parts.back()));
    }

    FileSystemEntity *getEntity(const string &path)
    {
        vector<string> parts = splitPath(path);
        if (parts.empty())
        {
            return root.get();
        }

        Directory *cur = root.get();
        for (int i = 0; i < parts.size(); i++)
        {
            FileSystemEntity *entity = cur->get(parts[i]);
            if (!entity)
            {
                return nullptr;
            }

            if (i == parts.size() - 1)
            {
                return entity;
            }

            if (entity->getFileType() != FileSystemEntityType::DIRECTORY)
            {
                return nullptr;
            }

            cur = static_cast<Directory *>(entity);
        }
        return nullptr;
    }

    void writeFile(const string &path, const string &data)
    {
        FileSystemEntity *entity = getEntity(path);
        if (!entity or entity->getFileType() != FileSystemEntityType::FILE)
        {
            throw runtime_error("Not a valid file path to write.\n");
        }
        File *file = static_cast<File *>(entity);
        file->write(data);
    }

    const string &readFile(const string &path)
    {
        FileSystemEntity *entity = getEntity(path);
        if (!entity or entity->getFileType() != FileSystemEntityType::FILE)
        {
            throw runtime_error("Not a valid file path to read.\n");
        }
        File *file = static_cast<File *>(entity);
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
};

int main()
{
    try
    {
        FileSystem fs;
        fs.mkdir("/home");
        fs.mkdir("/home/user");
        fs.mkdir("/home/user/docs");
        fs.mkdir_p("/home/user/docs2/pdfs");

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
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << "\n";
    }
    return 0;
}