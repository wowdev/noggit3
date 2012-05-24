#version 120

uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;
uniform sampler2D alpha1;
uniform sampler2D alpha2;
uniform sampler2D alpha3;

varying vec3 vertex_light_position;
varying vec3 vertex_normal;

void main()
{
    vec4 t0 = texture2D(tex0, gl_TexCoord[0].st);
    vec4 t1 = texture2D(tex1, gl_TexCoord[0].st);
    vec4 t2 = texture2D(tex2, gl_TexCoord[0].st);
    vec4 t3 = texture2D(tex3, gl_TexCoord[0].st);
    vec4 a1 = texture2D(alpha1, gl_TexCoord[1].st);
    vec4 a2 = texture2D(alpha2, gl_TexCoord[1].st);
    vec4 a3 = texture2D(alpha3, gl_TexCoord[1].st);

    float diffuse_value = max(dot(vertex_normal, vertex_light_position), 0.0); //! todo light correctly

    gl_FragColor = t0*(1.0-min(a1.a + a2.a + a3.a, 1.0)) + t1*a1.a + t2*a2.a + t3*a3.a;
}
/*(1.0 - (a1.a+a2.a+a2.a)) min(a1.a + a2.a + a3.a,1.0)  // t0 + t1*a1.a + t2*a2.a + t3*a3.a // + t1*a1.a + t2*a2.a + t3*a3.a  *(1.0-min(a1.a+a2.a+a2.a,1.0)) */
