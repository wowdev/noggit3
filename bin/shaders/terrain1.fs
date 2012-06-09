# terrain1.fs is part of Noggit3, licensed via GNU General Public License (version 3).
# Bernd Lörwald <bloerwald+noggit@googlemail.com>

!!ARBfp1.0

OPTION ARB_fog_linear;

#Declarations
TEMP col;
TEMP specular;
TEMP blend;

ATTRIB tex0 = fragment.texcoord[0];
ATTRIB tex1 = fragment.texcoord[1];
ATTRIB diff_color = fragment.color.primary;
ATTRIB spec_color = fragment.color.secondary;
PARAM shadow_col = program.local[0];

OUTPUT out = result.color;

#fetch textures
TEX col, tex0, texture[0], 2D;
TEX blend, tex1, texture[1], 2D;
#specular
MUL specular, spec_color, col.w;
MAD col, col, diff_color, specular;

#shadow
LRP out, blend.a, shadow_col, col;

#fix alpha (shadow_col.a = 1)
MOV out.a, shadow_col.a;

END
