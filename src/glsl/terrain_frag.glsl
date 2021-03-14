// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 330 core

uniform sampler2D shadow_map;
uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;
uniform vec2 tex_anim_0;
uniform vec2 tex_anim_1;
uniform vec2 tex_anim_2;
uniform vec2 tex_anim_3;
uniform sampler2D alphamap;
uniform bool is_textured;
uniform bool has_mccv;
uniform bool cant_paint;
uniform bool draw_areaid_overlay;
uniform vec4 areaid_color;
uniform bool draw_impassible_flag;
uniform bool draw_terrain_height_contour;
uniform bool draw_lines;
uniform bool draw_hole_lines;

uniform bool draw_wireframe;
uniform int wireframe_type;
uniform float wireframe_radius;
uniform float wireframe_width;
uniform vec4 wireframe_color;
uniform bool rainbow_wireframe;

uniform vec3 camera;
uniform bool draw_fog;
uniform vec4 fog_color;
uniform float fog_start;
uniform float fog_end;

uniform bool draw_cursor_circle;
uniform vec3 cursor_position;
uniform float outer_cursor_radius;
uniform float inner_cursor_ratio;
uniform vec4 cursor_color;

uniform vec3 light_dir;
uniform vec3 diffuse_color;
uniform vec3 ambient_color;

in vec3 vary_position;
in vec2 vary_texcoord;
in vec3 vary_normal;
in vec3 vary_mccv;

out vec4 out_color;

const float TILESIZE  = 533.33333;
const float CHUNKSIZE = TILESIZE / 16.0;
const float HOLESIZE  = CHUNKSIZE * 0.25;
const float UNITSIZE = HOLESIZE * 0.5;

vec4 texture_blend() 
{
  if(!is_textured)
    return vec4 (1.0, 1.0, 1.0, 1.0);

  vec3 alpha = texture(alphamap, vary_texcoord / 8.0).rgb;
  float a0 = alpha.r;  
  float a1 = alpha.g;
  float a2 = alpha.b;

  vec3 t0 = texture(tex0, vary_texcoord + tex_anim_0).rgb;
  vec3 t1 = texture(tex1, vary_texcoord + tex_anim_1).rgb;
  vec3 t2 = texture(tex2, vary_texcoord + tex_anim_2).rgb;
  vec3 t3 = texture(tex3, vary_texcoord + tex_anim_3).rgb;

  return vec4 (t0 * (1.0 - (a0 + a1 + a2)) + t1 * a0 + t2 * a1 + t3 * a2, 1.0);
}

float contour_alpha(float unit_size, float pos, float line_width)
{
  float f = abs(fract((pos + unit_size*0.5) / unit_size) - 0.5);
  float df = abs(line_width / unit_size);
  return smoothstep(0.0, df, f);
}

float contour_alpha(float unit_size, vec2 pos, vec2 line_width)
{
  return 1.0 - min( contour_alpha(unit_size, pos.x, line_width.x)
                  , contour_alpha(unit_size, pos.y, line_width.y)
                  );
}

void main()
{
  float dist_from_camera = distance(camera, vary_position);

  if(draw_fog && dist_from_camera >= fog_end)
  {
    out_color = fog_color;
    return;
  } 
  vec3 fw = fwidth(vary_position.xyz);

  out_color = texture_blend();
  out_color.rgb *= vary_mccv;

  // diffuse + ambient lighting
  out_color.rgb *= vec3(clamp (diffuse_color * max(dot(vary_normal, light_dir), 0.0), 0.0, 1.0)) + ambient_color;

  if(cant_paint)
  {
    out_color *= vec4(1.0, 0.0, 0.0, 1.0);
  }
  
  if(draw_areaid_overlay)
  {
    out_color = out_color * 0.3 + areaid_color;
  }

  if(draw_impassible_flag)
  {
    out_color.rgb = mix(vec3(1.0), out_color.rgb, 0.5);
  }

  float shadow_alpha = texture(shadow_map, vary_texcoord / 8.0).r;

  out_color = vec4 (out_color.rgb * (1.0 - shadow_alpha), 1.0);

  if (draw_terrain_height_contour)
  {
    out_color = vec4(out_color.rgb * contour_alpha(4.0, vary_position.y+0.1, fw.y), 1.0);
  }

  bool lines_drawn = false;
  if(draw_lines)
  {
    vec4 color = vec4(0.0, 0.0, 0.0, 0.0);

    color.a = contour_alpha(TILESIZE, vary_position.xz, fw.xz * 1.5);
    color.g = color.a > 0.0 ? 0.8 : 0.0;

    if(color.a == 0.0)
    {
      color.a = contour_alpha(CHUNKSIZE, vary_position.xz, fw.xz);
      color.r = color.a > 0.0 ? 0.8 : 0.0;
    }
    if(draw_hole_lines && color.a == 0.0)
    {
      color.a = contour_alpha(HOLESIZE, vary_position.xz, fw.xz * 0.75);
      color.b = 0.8;
    }
    
    lines_drawn = color.a > 0.0;
    out_color.rgb = mix(out_color.rgb, color.rgb, color.a);
  }

  if(draw_fog && dist_from_camera >= fog_end * fog_start)
  {
    float start = fog_end * fog_start;
    float alpha = (dist_from_camera - start) / (fog_end - start);
    out_color.rgb = mix(out_color.rgb, fog_color.rgb, alpha);
  }

  if(draw_wireframe && !lines_drawn)
  {
    // true by default => type 0
	  bool draw_wire = true;
    float real_wireframe_radius = max(outer_cursor_radius * wireframe_radius, 2.0 * UNITSIZE); 
	
	  if(wireframe_type == 1)
	  {
		  draw_wire = (length(vary_position.xz - cursor_position.xz) < real_wireframe_radius);
	  }
	
	  if(draw_wire)
	  {
		  float alpha = contour_alpha(UNITSIZE, vary_position.xz, fw.xz * wireframe_width);
		  float xmod = mod(vary_position.x, UNITSIZE);
		  float zmod = mod(vary_position.z, UNITSIZE);
		  float d = length(fw.xz) * wireframe_width;
		  float diff = min( min(abs(xmod - zmod), abs(xmod - UNITSIZE + zmod))
                      , min(abs(zmod - xmod), abs(zmod + UNITSIZE - zmod))
                      );        

		  alpha = max(alpha, 1.0 - smoothstep(0.0, d, diff));
      out_color.rgb = mix(out_color.rgb, wireframe_color.rgb, wireframe_color.a*alpha);
	  }
  }

  if (draw_cursor_circle)
  {
    float diff = length(vary_position.xz - cursor_position.xz);
    diff = min(abs(diff - outer_cursor_radius), abs(diff - outer_cursor_radius * inner_cursor_ratio));
    float alpha = smoothstep(0.0, length(fw.xz), diff);

    out_color.rgb = mix(cursor_color.rgb, out_color.rgb, alpha);
  }
}
