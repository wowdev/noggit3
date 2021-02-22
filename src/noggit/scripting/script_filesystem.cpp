// This file is part of the Script Brushes extension of Noggit3 by TSWoW (https://github.com/tswow/)
// licensed under GNU General Public License (version 3).
#include <noggit/scripting/script_filesystem.hpp>
#include <noggit/scripting/script_exception.hpp>

using namespace noggit::scripting;
namespace fs = boost::filesystem;

static void skip_dirs(script_file_iterator &itr)
{
    while (itr._dir != itr._end && boost::filesystem::is_directory(itr._dir->path()))
    {
        ++itr._dir;
    }
}

const char *noggit::scripting::read_file(const char *path)
{
    if (!fs::exists(path))
    {
        throw script_exception("No such file:" + *path);
    }
    std::ifstream t(path);
    std::string str((std::istreambuf_iterator<char>(t)),
                    std::istreambuf_iterator<char>());
    return str.c_str();
}

static void mkdirs(const char *pathstr)
{
    auto path = fs::path(pathstr);
    auto parent_path = path.parent_path();
    if (parent_path.string().size() > 0)
    {
        fs::create_directories(path.parent_path());
    }
}

void noggit::scripting::write_file(const char *path, const char *input)
{
    mkdirs(path);
    std::ofstream(path) << input;
}

void noggit::scripting::append_file(const char *path, const char *input)
{
    mkdirs(path);
    std::ofstream outfile;
    outfile.open(path, std::ios_base::app); // append instead of overwrite
    outfile << input;
}

bool noggit::scripting::path_exists(const char *path)
{
    return fs::exists(path);
}

script_file_iterator noggit::scripting::read_directory(const char *path)
{
    fs::recursive_directory_iterator dir(path), end;
    return script_file_iterator(dir, end);
}

noggit::scripting::script_file_iterator::script_file_iterator(fs::recursive_directory_iterator dir, fs::recursive_directory_iterator end) : _dir(dir), _end(end)
{
    skip_dirs(*this);
}

const char *noggit::scripting::file_itr_get(script_file_iterator &itr)
{
    return itr._dir->path().string().c_str();
}

bool noggit::scripting::file_itr_next(script_file_iterator &itr)
{
    if (!itr._started)
    {
        itr._started = true;
        return itr._dir != itr._end;
    }

    if (itr._dir != itr._end)
    {
        ++itr._dir;
        skip_dirs(itr);
    }

    return itr._dir != itr._end;
}