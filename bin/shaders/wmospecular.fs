# wmospecular.fs is part of Noggit3, licensed via GNU General Public License (version 3).
# Bernd Lörwald <bloerwald+noggit@googlemail.com>

!!ARBfp1.0

OPTION ARB_fog_linear;

#Declarations
TEMP col;
TEMP specular;

ATTRIB tex0 = fragment.texcoord[0];
ATTRIB diff_color = fragment.color.primary;
ATTRIB spec_color = fragment.color.secondary;
PARAM c = {1.0};
#PARAM test = {1,0,0,1};

OUTPUT out = result.color;

#fetch textures
TEX col, tex0, texture[0], 2D;

#specular
MUL specular, spec_color, col.w;
MAD out, col, diff_color, specular;

#fix alpha
MOV out.a, c.x;

#######
#MOV out, test;

END
