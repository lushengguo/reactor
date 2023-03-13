#include "base/mixed.hpp"
#include <algorithm>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
namespace reactor
{
std::string replace_all(std::string_view str, std::string_view from, std::string_view to)
{
    std::string s(str);
    if (from.empty() || str.empty())
        return s;

    size_t index = 0;
    while ((index = s.find(from, index)) != std::string::npos)
    {
        s.replace(index, from.size(), to);
        index += to.size();
    }
    return s;
}

bool is_numeric_string(std::string_view s)
{
    if (s.empty())
        return false;

    return static_cast<size_t>(std::count_if(s.begin(), s.end(), [](unsigned char ch) { return std::isdigit(ch); })) == s.size();
}

size_t calc_file_size(const char *path)
{
    if (path == NULL)
        return 0;

    std::ifstream ifs(path);
    if (ifs.is_open())
    {
        ifs.seekg(0, std::ios::end);
        size_t size = static_cast<size_t>(ifs.tellg());
        ifs.close();
        return size;
    }
    else
    {
        return 0;
    }
}

int read_file(const char *path, std::string &str)
{
    if (access(path, F_OK) != 0)
    {
        fprintf(stderr, "can not open file while reading into string:%s\n", path);
        return -1;
    }

    str = "";
    std::ifstream ifile(path);
    if (ifile.fail())
        fprintf(stderr, "open %s file failed while reading into string\n", path);

    char ch;
    while (ifile.get(ch))
    {
        str.append(1, ch);
    }

    ifile.close();
    return 0;
}

void recursion_create_dir(const char *path)
{
    if (path == NULL || strlen(path) == 0)
        return;

    if (access(path, F_OK) == 0)
        return;

    std::string str("mkdir -p ");
    str += path;
    if (system(str.c_str()) != 0)
    {
        fprintf(stderr,
                "recursion creat directory=%s error"
                "check permissions\n",
                path);
    }
}

std::vector<std::string> get_file_names(const char *dir_path)
{
    std::vector<std::string> files;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(dir_path)) != NULL)
    {
        /* print all the files and directories within directory */
        while ((ent = readdir(dir)) != NULL)
        {
            // printf("%s\n", ent->d_name);
            files.push_back(ent->d_name);
        }
        closedir(dir);
    }

    return files;
}

bool retry_n_times(size_t n, std::function<bool()> func, const char *error_message)
{
    while (n--)
    {
        if (func())
        {
            return true;
        }
        if (error_message)
        {
            fprintf(stderr, "%s", error_message);
        }
    }
    return false;
}

std::string Basename(std::string fullpath)
{
    if (fullpath.empty())
        return "";

    char buffer[fullpath.size() + 1];
    strcpy(buffer, fullpath.c_str());
    return ::basename(buffer);
}
} // namespace reactor