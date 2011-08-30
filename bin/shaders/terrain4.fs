!!ARBfp1.0

OPTION ARB_fog_linear;

#Declarations
TEMP col;
TEMP layer;
TEMP blend;
ALIAS specular=layer;

ATTRIB tex0 = fragment.texcoord[0];
ATTRIB tex1 = fragment.texcoord[1];
ATTRIB diff_color = fragment.color.primary;
ATTRIB spec_color = fragment.color.secondary;
PARAM shadow_col = program.local[0];

OUTPUT out = result.color;

#fetch textures
TEX col, tex0, texture[0], 2D;
TEX blend, tex1, texture[1], 2D;

#layer 1
TEX layer, tex0, texture[2], 2D;
LRP col, blend.r, layer, col;

#layer 2
TEX layer, tex0, texture[3], 2D;
LRP col, blend.g, layer, col;

#layer 3
TEX layer, tex0, texture[4], 2D;
LRP col, blend.b, layer, col;

#specular
MUL specular, spec_color, col.w;
MAD col, col, diff_color, specular;

#shadow
LRP out, blend.a, shadow_col, col;

#fix alpha (shadow_col.a = 1)
MOV out.a, shadow_col.a;

END