/*
**  Windstille - A Sci-Fi Action-Adventure Game
**  Copyright (C) 2010 Ingo Ruhnke <grumbel@gmail.com>
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <string.h>
#include <unistd.h>

#include <iostream>
#include <random>
#include <vector>

#include "sound/sound_manager.hpp"
#include "sound/sound_source.hpp"
#include "sound/sound_file.hpp"
#include "sound/filtered_sound_file.hpp"

namespace {

void print_usage(int argc, char** argv)
{
  std::cout << "Usage: " << argv[0] << " [OPTION]... SOUNDS...\n"
            << "\n"
            << "  --help   Display this help text\n"
            << "  --loop   Loopt the sound\n";
}

} // namespace

int main(int argc, char** argv)
{
  if (argc < 2)
  {
    print_usage(argc, argv);
    return 1;
  }
  else
  {
    bool loop = false;
    SoundSourceType source_type = SoundSourceType::STREAM;

    std::vector<std::string> files;
    for (int i = 1; i < argc; ++i) {
      if (argv[i][0] == '-') {
        if (strcmp(argv[i], "--help") == 0) {
          print_usage(argc, argv);
          return 0;
        } else if (strcmp(argv[i], "--loop") == 0) {
          loop = true;
        } else if (strcmp(argv[i], "--stream") == 0) {
          source_type = SoundSourceType::STREAM;
        } else if (strcmp(argv[i], "--static") == 0) {
          source_type = SoundSourceType::STATIC;
        } else {
          std::cerr << "error: unknown option " << argv[i] << std::endl;
        }
      } else {
        files.emplace_back(argv[i]);
      }
    }

    SoundManager sound_manager;

    sound_manager.set_gain(1.0f);
    sound_manager.sound().set_gain(1.0f);
    sound_manager.voice().set_gain(1.0f);

    std::vector<SoundSourcePtr> sources;
    for (auto const& filename : files)
    {
      SoundSourcePtr source = sound_manager.sound().prepare(filename, source_type);
      source->set_looping(loop);
      sources.emplace_back(source);
    }

    for (auto& source : sources) {
      source->play();
    }

    while (!sources.empty()) {
      for (auto& source : sources) {
        std::cout << "pos: " << source->get_pos() << " / " << source->get_duration() << std::endl;
      }

      std::erase_if(sources, [](SoundSourcePtr const& source){
        return !source->is_playing();
      });

      usleep(10000);

      sound_manager.update(100);
    }
  }
}

/* EOF */
