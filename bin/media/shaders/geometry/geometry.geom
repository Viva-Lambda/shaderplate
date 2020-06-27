#version 430
layout(triangles) in;
/* input primitive
 *
 * Available primitives are:
 * - triangles: triple vertex input
 * - points: single vertex
 * - lines: double vertex
 * - lines_adjacency: 4 vertex input
 * - triangles_adjacency: 6 vertex input
 */
layout(line_strip, max_vertices = 6) out;
/* output primitive
 *
 * Available output primitives are:
 *
 * - line_strip: 2*n vertex output for example 6
 * means three lines would be
 * emitted
 * - points: 1*n vertex output for example 6 means
 * 6 points would be emitted
 * - triangle_strip: 3 * n vertex output for
 * example 6 means 2 triangles would
 * be emitted
 * */

out vec3 fcolor; //

in Vertex_Out {
  vec4 Normal;
  vec4 ViewDir;
  vec4 LightDir;
  vec3 lightDirColor;
  vec3 viewDirColor;
  vec3 normalColor;
}
vs_in[];
const float MAGNITUDE = 1.0;

uniform mat4 projection;

void generateNormalLine(int index) {
  // -----------------------------------------
  fcolor = vec3(0); // this value is sent to fragment shader along with vertex
  gl_Position = projection * gl_in[index].gl_Position;
  EmitVertex(); // vertex created
  // -----------------------------------------
  fcolor =
      vs_in[index].normalColor; // value sent with vertex to fragment shader
  gl_Position =
      projection * (gl_in[index].gl_Position + vs_in[index].Normal * MAGNITUDE);
  EmitVertex(); // vertex created

  EndPrimitive(); // line strip primitive is done
}
void generateViewLine(int index) {
  fcolor = vec3(0); // this value is sent to fragment shader along with vertex
  gl_Position = projection * gl_in[index].gl_Position;
  EmitVertex(); // vertex created

  fcolor = vs_in[index].viewDirColor;
  gl_Position = projection *
                (gl_in[index].gl_Position + vs_in[index].ViewDir * MAGNITUDE);
  EmitVertex();   // vertex created
  EndPrimitive(); // line strip primitive is done
}
void generateLightLine(int index) {
  //
  fcolor = vec3(0); // this value is sent to fragment shader along with vertex
  gl_Position = projection * gl_in[index].gl_Position;
  EmitVertex(); // vertex created
  // -----------------------------------------

  fcolor = vs_in[index].lightDirColor;
  gl_Position = projection *
                (gl_in[index].gl_Position + vs_in[index].LightDir * MAGNITUDE);
  EmitVertex(); // vertex created

  EndPrimitive(); // line strip primitive is done
}
void main() {
  generateNormalLine(0); // modifying first vertex normal
  generateViewLine(0);   // modifying second vertex normal
  generateLightLine(0);  // modifying second vertex normal
}
