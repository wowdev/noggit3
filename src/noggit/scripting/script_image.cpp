#include <noggit/scripting/script_image.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_loader.hpp>
#include <lodepng.h>
#include <dukglue.h>

using namespace noggit::scripting;

noggit::scripting::script_image::script_image(std::string path)
{
    unsigned error = lodepng::decode(image,width,height,path);
    if(error)
    {
        get_cur_tool()->addLog("[error]: failed to load image with error "+error);
    }
}

noggit::scripting::script_image::script_image(unsigned width, unsigned height)
{
    image.resize(width*height*4);
    std::fill(image.begin(),image.end(),0);
}

unsigned noggit::scripting::script_image::get_width()
{
    return width;
}

unsigned noggit::scripting::script_image::get_height()
{
    return height;
}

unsigned noggit::scripting::script_image::get_index(unsigned x, unsigned y)
{
    return ((x+y*width)*4);
}

unsigned int noggit::scripting::script_image::get_pixel(unsigned x, unsigned y)
{
    unsigned index = get_index(x,y);
    return image[index] << 24 | image[index+1] << 16 | image[index+2] << 8 | image[index+3];
}

void noggit::scripting::script_image::set_pixel(unsigned x, unsigned y, unsigned int value)
{
    unsigned index = get_index(x,y);
    image[index] = (value << 24);
    image[index+1] = (value << 16)&0xff;
    image[index+2] = (value << 8)&0xff;
    image[index+3] = (value)&0xff;
}

void noggit::scripting::script_image::save(std::string filename)
{
    unsigned error = lodepng::encode(filename.c_str(), image, width, height); 
    if(error)
    {
        get_cur_tool()->addLog("[error]: failed to save image with error "+error);
    }
}

std::shared_ptr<script_image> noggit::scripting::load_image(std::string path)
{
    return std::make_shared<script_image>(path);
}

std::shared_ptr<script_image> noggit::scripting::create_image(unsigned width, unsigned height)
{
    return std::make_shared<script_image>(width,height);
}

void noggit::scripting::register_image_functions(duk_context* ctx)
{
    GLUE_METHOD(ctx,script_image,get_index);
    GLUE_METHOD(ctx,script_image,get_pixel);
    GLUE_METHOD(ctx,script_image,set_pixel);
    GLUE_METHOD(ctx,script_image,save);
    GLUE_METHOD(ctx,script_image,get_width);
    GLUE_METHOD(ctx,script_image,get_height);

    GLUE_FUNCTION(ctx,load_image);
    GLUE_FUNCTION(ctx,create_image);
}