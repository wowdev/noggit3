#include <noggit/scripting/script_filesystem.hpp>
#include <noggit/scripting/script_exception.hpp>
#include <noggit/scripting/script_loader.hpp>
#include <dukglue.h>

using namespace noggit::scripting;
namespace fs = boost::filesystem;

std::string noggit::scripting::read_file(std::string path)
{
    if(!fs::exists(path))
    {
        throw script_exception("No such file:"+path);
    }
    std::ifstream t(path);
    std::string str((std::istreambuf_iterator<char>(t)),
        std::istreambuf_iterator<char>());
    return str;
}

static void mkdirs(std::string pathstr)
{
    auto path = fs::path(pathstr);
    auto parent_path = path.parent_path();
    if(parent_path.string().size()>0)
    {
        fs::create_directories(path.parent_path());
    }
}

void noggit::scripting::write_file(std::string path, std::string input)
{
    mkdirs(path);
    std::ofstream(path) << input;
}

void noggit::scripting::append_file(std::string path, std::string input)
{
    mkdirs(path);
    std::ofstream outfile;
    outfile.open(path, std::ios_base::app); // append instead of overwrite
    outfile << input; 
}

bool noggit::scripting::path_exists(std::string path)
{
    return fs::exists(path);
}

std::shared_ptr<script_file_iterator> noggit::scripting::read_directory(std::string path)
{
    fs::recursive_directory_iterator dir(path), end;
    return std::make_shared<script_file_iterator>(dir,end);
}


void noggit::scripting::script_file_iterator::skip_dirs()
{
    while(_dir != _end && boost::filesystem::is_directory(_dir->path()))
    {
        ++_dir;
    }
}

noggit::scripting::script_file_iterator::script_file_iterator
(fs::recursive_directory_iterator dir, fs::recursive_directory_iterator end): _dir(dir), _end(end)
{
    skip_dirs();
}

bool noggit::scripting::script_file_iterator::is_on_file()
{
    return _dir != _end;
}

std::string noggit::scripting::script_file_iterator::get_file()
{
    return _dir->path().string();
}

void noggit::scripting::script_file_iterator::next_file()
{
    if(_dir != _end)
    {
        ++_dir;
        skip_dirs();
    }
}

void noggit::scripting::register_filesystem_functions(duk_context* ctx)
{
    GLUE_FUNCTION(ctx,read_file);
    GLUE_FUNCTION(ctx,write_file);
    GLUE_FUNCTION(ctx,read_directory);
    GLUE_FUNCTION(ctx,append_file);

    GLUE_METHOD(ctx,script_file_iterator,is_on_file);
    GLUE_METHOD(ctx,script_file_iterator,get_file);
    GLUE_METHOD(ctx,script_file_iterator,next_file);
}