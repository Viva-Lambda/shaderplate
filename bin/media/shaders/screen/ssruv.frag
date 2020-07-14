#version 430
layout(binding = 0) uniform sampler2D gPosition; // position in viewspace
layout(binding = 1) uniform sampler2D gNormal;   // normals in viewspace
layout(binding = 2) uniform sampler2D lightBuffer;
// normal buffer - from g-buffer in in view space
layout(location = 0) out vec4 FragColor;

uniform mat4 projection;
uniform float pixThickness = 0.5;
uniform float pixMaxDistance = 6;
uniform float pixResolution = 0.3;
uniform int pixSteps = 2;

in vec2 TexCoords;

bool inScreen(vec4 uv) {
  bool x_check = uv.x < 1.0 && uv.x > 0.0;
  bool y_check = uv.y < 1.0 && uv.y > 0.0;
  return x_check && y_check;
}
vec4 getRayHit1() {
  vec4 uv = vec4(0);
  vec4 positionFrom = texture(gPosition, TexCoords);
  vec2 texSize = textureSize(gPosition, 0);
  if (positionFrom.w <= 0.0) {
    return uv;
  }
  vec3 viewDir = normalize(positionFrom.xyz);
  vec3 normal = normalize(texture(gNormal, TexCoords).xyz);
  vec3 pivot = normalize(reflect(viewDir, normal));
  vec4 positionTo = positionFrom;

  vec4 startView = vec4(positionFrom.xyz + (pivot * 0.0), 1.0);
  vec4 endView = vec4(positionFrom.xyz + (pivot * pixMaxDistance), 1.0);

  vec4 startFrag = projection * startView;
  startFrag.xyz /= startFrag.w;
  startFrag.xy = startFrag.xy * 0.5 + 0.5;
  startFrag.xy *= texSize;

  vec4 endFrag = projection * endView;
  endFrag.xyz /= endFrag.w;
  endFrag.xy = endFrag.xy * 0.5 + 0.5;
  endFrag.xy *= texSize;

  vec2 frag = startFrag.xy;
  uv.xy = frag / texSize;

  float deltaX = endFrag.x - startFrag.x;
  float deltaY = endFrag.y - startFrag.y;

  // decide axis
  float useX = abs(deltaX) >= abs(deltaY) ? 1.0 : 0.0;

  float delta =
      mix(abs(deltaY), abs(deltaX), useX) * clamp(pixResolution, 0.0, 1.0);

  vec2 increment = vec2(deltaX, deltaY) / max(delta, 0.001);

  float search0 = 0.0;
  float search1 = 0.0;
  int hit0 = 0;
  int hit1 = 0;

  float steps = pixSteps;

  float viewDistance = startView.y;
  float depth = pixThickness;

  for (int i = 0; i < int(delta); i++) {
    frag += increment;
    uv.xy = frag / texSize;
    positionTo = texture(gPosition, uv.xy);

    //
    search1 = mix((frag.y - startFrag.y) / deltaY,
                  (frag.x - startFrag.x) / deltaX, useX);

    // clamp to value to uv range
    search1 = clamp(search1, 0.0, 1.0);

    viewDistance =
        (startView.y * endView.y) / mix(endView.y, startView.y, search1);
    depth = viewDistance - positionTo.y;

    if (depth > 0 && depth < pixThickness) {
      hit0 = 1;
    } else {
      search0 = search1;
    }
  }
  search1 = search0 + ((search1 - search0) / 2.0);
  steps *= hit0;
  for (int i = 0; i < steps; i++) {
    frag = mix(startFrag.xy, endFrag.xy, search1);
    uv.xy = frag / texSize;
    positionTo = texture(gPosition, uv.xy);

    viewDistance =
        (startView.y * endView.y) / mix(endView.y, startView.y, search1);
    depth = viewDistance - positionTo.y;

    if (depth > 0 && depth < pixThickness) {
      hit1 = 1;
      search1 = search0 + ((search1 - search0) / 2.0);
    } else {
      float temp = search1;
      search1 = search1 + ((search1 - search0) / 2.0);
      search0 = temp;
    }
  }
  //
  float depthAlpha = positionTo.w;
  float costheta = max(dot(-viewDir, pivot), 0.0);
  // depth to uv range
  float uvdepth = (1 - clamp(depth / pixThickness, 0, 1));
  // scale ray length to uv range
  float scaled_ray = length(positionTo - positionFrom) / pixMaxDistance;
  float ray_len = 1 - clamp(scaled_ray, 0.0, 1.0);
  float visibility = hit1 * depthAlpha * (1 - costheta) * uvdepth * ray_len *
                     (inScreen(uv) ? 1.0 : 0.0);

  visibility = clamp(visibility, 0.0, 1.0); // alpha channel
  uv.ba = vec2(visibility);
  return uv;
}

void main() {
  //
  FragColor = vec4(TexCoords, 0, 1);
}
