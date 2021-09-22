#version 330 core

layout(location = 0) in vec4 vertex;

uniform float x_partition_size = 0.0;
uniform float y_partition_size = 0.0;

uniform float x_pixel_offset = 0.0;
uniform float y_pixel_offset = 0.0;

uniform float x_pixel_stride = 1.0;
uniform float y_pixel_stride = 1.0;

out vec2 tex_coords;

void
main()
{
  float x_max = x_partition_size * x_pixel_stride;
  float y_max = y_partition_size * y_pixel_stride;

  vec2 offset = vec2(0.0, 0.0);

  offset += vec2(vertex.x * x_partition_size * x_pixel_stride,
                 vertex.y * y_partition_size * y_pixel_stride);

  offset += vec2(vertex.z * x_partition_size,
                 vertex.w * y_partition_size);

  offset += vec2(x_pixel_offset, y_pixel_offset);

  tex_coords = vertex.xy;

  vec2 position = vec2(offset.x / x_max,
                       offset.y / y_max);

  float ndc_x = (position.x * 2.0) - 1.0;
  float ndc_y = 1.0 - (position.y * 2.0);
  gl_Position = vec4(ndc_x, ndc_y, 0.0, 1.0);
}
