#include <lua.hpp>

#include <iostream>

#include <QApplication>
#include <QImage>
#include <QLabel>

namespace lua
{
//   struct libraries_type
//   {
//     size_t _base : 1;
//     size_t _io : 1;
//     size_t _table : 1;
//     size_t _string : 1;
//     size_t _math : 1;
//     libraries_type()
//       : _base (false)
//       , _io (false)
//       , _table (false)
//       , _string (false)
//       , _math (false)
//     {}
// #define CHAIN_SETTER(NAME)                                            \
//     libraries_type& NAME (bool on = true)                             \
//     {                                                                 \
//       _ ## NAME = on;                                                 \
//       return *this;                                                   \
//     }

//     CHAIN_SETTER(base)
//     CHAIN_SETTER(io)
//     CHAIN_SETTER(table)
//     CHAIN_SETTER(string)
//     CHAIN_SETTER(math)

// #undef CHAIN_SETTER
//   };

  class context
  {
  public:
    context()
      : _state (lua_open())
    { }
    ~context()
    {
      lua_close (_state);
    }

    void open_libraries()
    {
      luaL_openlibs (_state);
    }

    void parse (const std::string& data)
    {
      luaL_dostring (_state, data.c_str());
    }

    void call_function (const std::string& name)
    {
      lua_getglobal (_state, name.c_str());
      lua_call (_state, 0, 0);
    }

    template<typename Arg1>
    void call_function (const std::string& name, const Arg1& arg1)
    {
      lua_getglobal (_state, name.c_str());
      push_value (arg1);
      lua_call (_state, 1, 1);
    }

    template<typename Arg1, typename Arg2>
    void call_function ( const std::string& name
                       , const Arg1& arg1
                       , const Arg2& arg2
                       )
    {
      lua_getglobal (_state, name.c_str());
      push_value (arg1);
      push_value (arg2);
      lua_call (_state, 2, 1);
    }

    template<typename Arg1, typename Arg2, typename Arg3, typename Arg4>
    void call_function ( const std::string& name
                       , const Arg1& arg1
                       , const Arg2& arg2
                       , const Arg3& arg3
                       , const Arg4& arg4
                       )
    {
      lua_getglobal (_state, name.c_str());
      push_value (arg1);
      push_value (arg2);
      push_value (arg3);
      push_value (arg4);
      lua_call (_state, 4, 1);
    }


    template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
    void call_function ( const std::string& name
                       , const Arg1& arg1
                       , const Arg2& arg2
                       , const Arg3& arg3
                       , const Arg4& arg4
                       , const Arg5& arg5
                       , const Arg6& arg6
                       )
    {
      lua_getglobal (_state, name.c_str());
      push_value (arg1);
      push_value (arg2);
      push_value (arg3);
      push_value (arg4);
      push_value (arg5);
      push_value (arg6);
      lua_call (_state, 6, 1);
    }

    template<typename type>
    void push_value (const type& value)
    {
      lua_pushnumber (_state, value);
    }

    template<typename type>
    type get_value()
    {
      type ret (lua_tonumber (_state, -1));
      lua_pop (_state, 1);
      return ret;
    }

  private:
    context (const context&);
    context& operator= (const context&);

    lua_State* _state;
  };
}

int main(int argc, char** argv)
{
  QApplication a (argc, argv);

  lua::context context;
  context.open_libraries();

  context.parse ( ""
                "function distance(center_x, center_y, size_x, size_y, vec_x, vec_y)"
                "  local offset_x = (vec_x - center_x) / (0.5 * size_x);"
                "  local offset_y = (vec_y - center_y) / (0.5 * size_y);"
                "  return math.sqrt (offset_x*offset_x + offset_y*offset_y);"
                "end;"
                );

  context.parse ( ""
                "function brush_random (_, _, _, _, _, _)"
                "  return math.random();"
                "end;"
                );
  context.parse ( ""
                "function brush_square (_, _, _, _, pos_x, pos_y)"
                "  local radius = size_x;"
                "  local inner_radius = 0.8;"
                "  local outer_radius = 0.2;"
                "  if pos_x > 0.2 and pos_y > 0.2 and "
                "     pos_x < 0.8 and pos_y < 0.8 then"
                "    return 1.0;"
                "  end;"
                "  return 0.0;"
                "end;"
                );
  context.parse ( ""
                "function brush_circle ( center_x, center_y"
                "                      , size_x, _"
                "                      , pos_x, pos_y"
                "                      )"
                "  local radius = size_x;"
                "  local inner_radius = 0.8;"
                "  local outer_radius = 0.2;"
                "  local dist = distance ( center_x, center_y"
                "                        , size_x, size_x"
                "                        , pos_x, pos_y"
                "                        );"
                "  if dist > radius then"
                "    return 0.0;"
                "  end;"
                "  if dist < inner_radius then"
                "    return 1.0;"
                "  end;"
                "  return (1.0 - (dist - inner_radius) / outer_radius);"
                "end;"
                );

  size_t pixels (11*20);
  QImage image (pixels, pixels, QImage::Format_RGB32);

  for (size_t x (0); x < pixels; ++x)
  {
    for (size_t y (0); y < pixels; ++y)
    {
      context.call_function ( "brush_circle"
                            , 0.5f, 0.5f
                            , 1.0f, 1.0f
                            , x * (1.0f / (pixels - 1))
                            , y * (1.0f / (pixels - 1))
                            );
      const float value (context.get_value<float>());
      image.setPixel ( x, y, int(value * 255)
                           | int(value * 255) << 8
                           | int(value * 255) << 16
                     );
    }
  }

  QLabel myLabel;
  myLabel.setPixmap (QPixmap::fromImage (image));
  myLabel.show();

  return a.exec();
}
