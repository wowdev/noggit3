#include <noggit/scripting/script_procedures.hpp>
#include <noggit/scripting/script_image.hpp>
#include <noggit/scripting/script_tex.hpp>
#include <noggit/scripting/script_math.hpp>
#include <noggit/scripting/script_context.hpp>
#include <noggit/scripting/script_selection.hpp>

#include <math/vector_3d.hpp>

namespace noggit {
	namespace scripting {
		void procedures::paint_texture(
        selection & sel
			, image const& img 
			, int layer
			, float pressure
			, float angle
		)
		{
      auto center = math::vector_3d(sel.min().x + (sel.max().x - sel.min().x) / 2, 0, sel.min().z + (sel.max().z - sel.min().z) / 2);
      auto outer_radius = (sel.max().x - sel.min().x) / 2;
      int width = img.width();
      int height = img.height();
      float half_width = float(width) / 2;
      float half_height = float(height) / 2;
      for (auto & tex : sel.textures_raw())
      {
        auto global_pos = tex.get_pos_2d();
        auto dist = dist_2d(global_pos, center) / outer_radius;
        global_pos = rotate_2d(global_pos, center, angle);

        auto rel_x = (global_pos.x - center.x) / outer_radius;
        auto rel_z = (global_pos.z - center.z) / outer_radius;

        if (!(rel_x < -1 || rel_x > 1 || rel_z < -1 || rel_z > 1))
        {
          auto img_x = round(half_width + half_width * rel_x);
          auto img_z = round(half_height + half_height * rel_z);
          if (img_x >= 0 && img_x < width && img_z >= 0 && img_z < height) {
            auto old = tex.get_alpha(layer);
            tex.set_alpha(layer, old + (img.get_red(img_x, img_z) * pressure * (1 - dist)));
          }
        }
      }
		}

    void register_procedures(script_context* state)
    {
      state->new_usertype<procedures>("procedures_class"
        , "paint_texture", &procedures::paint_texture
        );
      state->set("procedures", procedures());
    }
	}
}