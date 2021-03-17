// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/cursor_render.hpp>

#include <opengl/shader.hpp>

namespace noggit
{
  void cursor_render::draw(mode cursor_mode, math::matrix_4x4 const& mvp, math::vector_4d color, math::vector_3d const& pos, float radius, float inner_radius_ratio)
  {
    if (!_uploaded)
    {
      upload();
    }

    opengl::scoped::use_program shader {*_cursor_program.get()};

    shader.uniform("model_view_projection", mvp);
    shader.uniform("cursor_pos", pos);
    shader.uniform("color", color);
    shader.uniform("radius", radius);

    opengl::scoped::vao_binder const _ (_vaos[static_cast<int>(cursor_mode)]);

    gl.drawElements(GL_LINES, _indices_count[cursor_mode], GL_UNSIGNED_SHORT, opengl::index_buffer_is_already_bound{});

    if (inner_radius_ratio > 0.f)
    {
      shader.uniform("radius", radius*inner_radius_ratio);
      gl.drawElements(GL_LINES, _indices_count[cursor_mode], GL_UNSIGNED_SHORT, opengl::index_buffer_is_already_bound{});
    }
  }

  void cursor_render::upload()
  {
    _vaos.upload();
    _vbos.upload();

    _cursor_program.reset
      ( new opengl::program
          { { GL_VERTEX_SHADER,   opengl::shader::src_from_qrc("cursor_vs") }
          , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("cursor_fs") }
          }
      );    

    opengl::scoped::use_program shader {*_cursor_program.get()};

    create_circle_buffer(shader);
    create_sphere_buffer(shader);
    create_square_buffer(shader);
    create_cube_buffer(shader);

    _uploaded = true;
  }

  void cursor_render::create_circle_buffer(opengl::scoped::use_program& shader)
  {
    std::vector<math::vector_3d> vertices;
    std::vector<std::uint16_t> indices;

    int segment = 60;

    for (int i = 0; i < segment; ++i)
    {
      float x = math::cos(math::degrees(i * 360 / segment));
      float z = math::sin(math::degrees(i * 360 / segment));
      vertices.emplace_back(x, 0.f, z);
      indices.emplace_back(i);
      indices.emplace_back((i + 1) % segment);
    }

    _indices_count[mode::circle] = indices.size();

    int id = static_cast<int>(mode::circle);

    gl.bufferData<GL_ARRAY_BUFFER>(_vbos[id * 2], vertices.size() * sizeof(*vertices.data()), vertices.data(), GL_STATIC_DRAW);
    gl.bufferData<GL_ELEMENT_ARRAY_BUFFER>(_vbos[id * 2 + 1], indices.size() * sizeof(*indices.data()), indices.data(), GL_STATIC_DRAW);

    opengl::scoped::index_buffer_manual_binder indices_binder(_vbos[id * 2 + 1]);

    {
      opengl::scoped::vao_binder const _(_vaos[id]);

      shader.attrib(_, "position", _vbos[id * 2], 3, GL_FLOAT, GL_FALSE, 0, 0);

      indices_binder.bind();
    }
  }

  void cursor_render::create_sphere_buffer(opengl::scoped::use_program& shader)
  {
    std::vector<math::vector_3d> vertices;
    std::vector<std::uint16_t> indices;

    int segment = 60;
    int rotation_plane = 6;

    int id_ofs = 0;

    for (int r = 0; r <= rotation_plane; ++r)
    {
      math::degrees rotation(360.f*r / static_cast<float>(rotation_plane));

      math::matrix_4x4 m(math::matrix_4x4::rotation_xyz, math::degrees::vec3(math::degrees(0.f), math::degrees(0.f), rotation));

      for (int i = 0; i < segment; ++i)
      {
        float x = math::cos(math::degrees(i * 360 / segment));
        float z = math::sin(math::degrees(i * 360 / segment));

        math::vector_3d v(x, 0.f, z);

        vertices.emplace_back(m*v);
        if (r < rotation_plane)
        {
          indices.emplace_back(i + id_ofs);
          indices.emplace_back(((i + 1) % segment) + id_ofs);
        }
      }

      id_ofs += segment;
    }
    
    for (int i = 0; i < segment; ++i)
    {
      for (int r = 0; r < rotation_plane; ++r)
      {
        indices.emplace_back(i + r*segment);
        indices.emplace_back(i + (r+1)*segment);
      }
    }    

    _indices_count[mode::sphere] = indices.size();

    int id = static_cast<int>(mode::sphere);

    gl.bufferData<GL_ARRAY_BUFFER>(_vbos[id * 2], vertices.size() * sizeof(*vertices.data()), vertices.data(), GL_STATIC_DRAW);
    gl.bufferData<GL_ELEMENT_ARRAY_BUFFER>(_vbos[id * 2 + 1], indices.size() * sizeof(*indices.data()), indices.data(), GL_STATIC_DRAW);

    opengl::scoped::index_buffer_manual_binder indices_binder(_vbos[id * 2 + 1]);

    {
      opengl::scoped::vao_binder const _(_vaos[id]);

      shader.attrib(_, "position", _vbos[id * 2], 3, GL_FLOAT, GL_FALSE, 0, 0);

      indices_binder.bind();
    }
  }

  void cursor_render::create_square_buffer(opengl::scoped::use_program& shader)
  {
    std::vector<math::vector_3d> vertices = 
    {
       {-0.5f,0.f,-0.5f}
      ,{ 0.5f,0.f,-0.5f}
      ,{ 0.5f,0.f, 0.5f}
      ,{-0.5f,0.f, 0.5f}
    };

    std::vector<std::uint16_t> indices = {0,1, 1,2 ,2,3 ,3,0};    

    _indices_count[mode::square] = indices.size();

    int id = static_cast<int>(mode::square);

    gl.bufferData<GL_ARRAY_BUFFER>(_vbos[id * 2], vertices.size() * sizeof(*vertices.data()), vertices.data(), GL_STATIC_DRAW);
    gl.bufferData<GL_ELEMENT_ARRAY_BUFFER>(_vbos[id * 2 + 1], indices.size() * sizeof(*indices.data()), indices.data(), GL_STATIC_DRAW);

    opengl::scoped::index_buffer_manual_binder indices_binder(_vbos[id * 2 + 1]);

    {
      opengl::scoped::vao_binder const _(_vaos[id]);

      shader.attrib(_, "position", _vbos[id * 2], 3, GL_FLOAT, GL_FALSE, 0, 0);

      indices_binder.bind();
    }

    gl.bindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  void cursor_render::create_cube_buffer(opengl::scoped::use_program& shader)
  {
    std::vector<math::vector_3d> vertices =
    {
        {-0.5f,-0.5f,-0.5f}
      , { 0.5f,-0.5f,-0.5f}
      , { 0.5f,-0.5f, 0.5f}
      , {-0.5f,-0.5f, 0.5f}
      , {-0.5f, 0.5f,-0.5f}
      , { 0.5f, 0.5f,-0.5f}
      , { 0.5f, 0.5f, 0.5f}
      , {-0.5f, 0.5f, 0.5f}
    };

    std::vector<std::uint16_t> indices = 
    {  
        0,1, 1,2 ,2,3 ,3,0
      , 0,4, 1,5 ,2,6 ,3,7
      , 4,5, 5,6 ,6,7 ,7,4
    };

    _indices_count[mode::cube] = indices.size();

    int id = static_cast<int>(mode::cube);

    gl.bufferData<GL_ARRAY_BUFFER>(_vbos[id * 2], vertices.size() * sizeof(*vertices.data()), vertices.data(), GL_STATIC_DRAW);
    gl.bufferData<GL_ELEMENT_ARRAY_BUFFER>(_vbos[id * 2 + 1], indices.size() * sizeof(*indices.data()), indices.data(), GL_STATIC_DRAW);

    opengl::scoped::index_buffer_manual_binder indices_binder(_vbos[id * 2 + 1]);

    {
      opengl::scoped::vao_binder const _(_vaos[id]);

      shader.attrib(_, "position", _vbos[id * 2], 3, GL_FLOAT, GL_FALSE, 0, 0);

      indices_binder.bind();
    }

    gl.bindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }
}
