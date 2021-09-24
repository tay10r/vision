#include <iostream>
#include <string>
#include <vector>

#include <stdlib.h>

int
main()
{
  int i = 0;

  int w = 0;
  int h = 0;
  int padded_w = 0;
  int padded_h = 0;

  int render_x_pixel_count = 0;
  int render_y_pixel_count = 0;
  int render_x_pixel_offset = 0;
  int render_y_pixel_offset = 0;
  int render_x_pixel_stride = 0;
  int render_y_pixel_stride = 0;
  int render_request_id = 0;

  while (std::cin) {

    std::string command;

    std::getline(std::cin, command);

    if (command.empty() || (command == "q"))
      break;

    switch (command[0]) {
      case 'r':
        sscanf(&command[1],
               "%d %d  %d %d  %d %d  %d",
               &render_x_pixel_count,
               &render_y_pixel_count,
               &render_x_pixel_offset,
               &render_y_pixel_offset,
               &render_x_pixel_stride,
               &render_y_pixel_stride,
               &render_request_id);
        break;
      case 's':
        sscanf(&command[1], "%d %d %d %d", &w, &h, &padded_w, &padded_h);
        break;
      case 'k': // <- keyboard input
        break;
      case 'm': // <- mouse movement input
        break;
      case 'b': // <- mouse button input
        break;
    }

    if (command[0] != 'r')
      continue;

    std::vector<unsigned char> buffer(render_x_pixel_count *
                                      render_y_pixel_count * 3);

    for (int y = 0; y < render_y_pixel_count; y++) {

      for (int x = 0; x < render_x_pixel_count; x++) {

        const int abs_x = (x * render_x_pixel_stride) + render_x_pixel_offset;
        const int abs_y = (y * render_y_pixel_stride) + render_y_pixel_offset;

        const float u = (abs_x + 0.5f) / w;
        const float v = (abs_y + 0.5f) / h;

        unsigned char* pixel = &buffer[((y * render_x_pixel_count) + x) * 3];
        pixel[0] = 255 * u;
        pixel[1] = 255 * v;
        pixel[2] = 255;
      }
    }

    printf("rgb buffer %d %d %d\n",
           render_x_pixel_count,
           render_y_pixel_count,
           render_request_id);

    fwrite(&buffer[0], 1, buffer.size(), stdout);
  }

  return EXIT_SUCCESS;
}
