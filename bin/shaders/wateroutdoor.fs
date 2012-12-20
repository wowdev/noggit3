!!ARBfp1.0

OPTION ARB_fog_linear;

#Declarations
TEMP col;
TEMP specular;
TEMP reg;

ATTRIB tex0 = fragment.texcoord[0];
ATTRIB diff_color = fragment.color.primary;
ATTRIB spec_color = fragment.color.secondary;
PARAM water_color_light = program.local[0];
PARAM water_color_dark  = program.local[1];
#PARAM mycol = {1,1,1,1};

OUTPUT out = result.color;

#fetch textures
TEX col, tex0, texture[0], 2D;

#specular
MUL reg.w, col.w, tex0.z;
#MOV reg.w, col.w;
MUL specular, spec_color, reg.w;

#interpolate water colors
LRP reg, tex0.z, water_color_light, water_color_dark; # ugly hack, color interpolation shuold go in a VS
#MOV reg, water_color_light;
ADD reg.xyz, col, reg;

MAD out, reg, diff_color, specular;

#fix alpha (shadow_col.a = 1)
MOV out.a, 0.9;

#######
#MOV out, water_color_light;
#MOV out, mycol;

END