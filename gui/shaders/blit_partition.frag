#version 330 core

out vec4 color;

uniform sampler2D partition;

in vec2 tex_coords;

void
main()
{
  color = texture(partition, tex_coords);
}
