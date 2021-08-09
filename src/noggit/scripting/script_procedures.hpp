// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <vector>

namespace noggit {
	namespace scripting {
		class selection;
		class image;
		class script_context;
		class procedures {
		public:
			void paint_texture(
				  selection & sel
				, image const& image
				, int layer
				, float pressure
				, float angle
			);
		};

		void register_procedures(script_context * state);
	}
}