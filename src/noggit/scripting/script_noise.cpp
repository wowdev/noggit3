#include <noggit/scripting/script_noise.hpp>
#include <noggit/scripting/script_loader.hpp>
#include <dukglue.h>

using namespace noggit::scripting;

float noggit::scripting::script_noise_2d::get(int x, int y)
{
    unsigned index = x + y * _width;
    return _noise[index]; 
}

float noggit::scripting::script_noise_2d::get_f(float x, float y)
{
    return get(std::floor(x),std::floor(y));
}

void noggit::scripting::script_noise_2d::set(int x, int y, float value)
{
    unsigned index = x + y * _width;
    _noise[index] = value;
}

noggit::scripting::script_noise_2d::script_noise_2d(unsigned start_x, unsigned start_y, unsigned width, unsigned height) 
    : _width(width), _height(height), _start_x(start_x), _start_y(start_y)
{

    _noise.resize(width * height);
}

unsigned noggit::scripting::script_noise_2d::get_start_x() { return _start_x; }
unsigned noggit::scripting::script_noise_2d::get_start_y() { return _start_y; }
unsigned noggit::scripting::script_noise_2d::get_width() { return _width;  }
unsigned noggit::scripting::script_noise_2d::get_height() { return _height;  }

noggit::scripting::script_noise_generator::script_noise_generator(FastNoise::SmartNode<> generator)
    : _generator(generator){}


std::shared_ptr<script_noise_2d> noggit::scripting::script_noise_generator::uniform_2d(std::string seed, int xStart, int yStart, unsigned xSize, unsigned ySize, float frequency)
{
    auto map = std::make_shared<script_noise_2d>(xStart,yStart,xSize,ySize);
    _generator->GenUniformGrid2D(map->_noise.data(), xStart, yStart, xSize, ySize, frequency, std::hash<std::string>()(seed));
    return map;
}

std::shared_ptr<script_noise_generator> noggit::scripting::noise_simplex()
{
    return std::make_shared<script_noise_generator>(FastNoise::New<FastNoise::Simplex>());
}
std::shared_ptr<script_noise_generator> noggit::scripting::noise_perlin()
{
    return std::make_shared<script_noise_generator>(FastNoise::New<FastNoise::Perlin>());
}
std::shared_ptr<script_noise_generator> noggit::scripting::noise_value()
{
    return std::make_shared<script_noise_generator>(FastNoise::New<FastNoise::Value>());
}
std::shared_ptr<script_noise_generator> noggit::scripting::noise_fractal()
{
    return std::make_shared<script_noise_generator>(FastNoise::New<FastNoise::FractalFBm>());
}
std::shared_ptr<script_noise_generator> noggit::scripting::noise_cellular()
{
    return std::make_shared<script_noise_generator>(FastNoise::New<FastNoise::CellularValue>());
}
std::shared_ptr<script_noise_generator> noggit::scripting::noise_white()
{
    return std::make_shared<script_noise_generator>(FastNoise::New<FastNoise::White>());
}
std::shared_ptr<script_noise_generator> noggit::scripting::noise_custom(std::string encodedNodeTree)
{
    return std::make_shared<script_noise_generator>(FastNoise::NewFromEncodedNodeTree(encodedNodeTree.c_str()));
}

void noggit::scripting::register_noise_functions(duk_context* ctx)
{
    GLUE_METHOD(ctx,script_noise_2d,get);
    GLUE_METHOD(ctx,script_noise_2d,get_f);
    GLUE_METHOD(ctx,script_noise_2d,set);
    GLUE_METHOD(ctx,script_noise_2d,get_start_x);
    GLUE_METHOD(ctx,script_noise_2d,get_start_y);
    GLUE_METHOD(ctx,script_noise_2d,get_width);
    GLUE_METHOD(ctx,script_noise_2d,get_height);
    GLUE_METHOD(ctx,script_noise_generator,uniform_2d);

    GLUE_FUNCTION(ctx,noise_simplex);
    GLUE_FUNCTION(ctx,noise_perlin);
    GLUE_FUNCTION(ctx,noise_value);
    GLUE_FUNCTION(ctx,noise_fractal);
    GLUE_FUNCTION(ctx,noise_cellular);
    GLUE_FUNCTION(ctx,noise_white);
    GLUE_FUNCTION(ctx,noise_custom);
}