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

#include <wstsound/effect.hpp>
#include <wstsound/effect_slot.hpp>
#include <wstsound/filtered_sound_file.hpp>
#include <wstsound/sound_file.hpp>
#include <wstsound/sound_manager.hpp>
#include <wstsound/sound_source.hpp>

using namespace wstsound;

namespace {

void print_usage(int argc, char** argv)
{
  std::cout << "Usage: " << argv[0] << " [OPTION]... SOUNDS...\n"
            << "\n"
            << "  --help       Display this help text\n"
            << "  --loop       Loopt the sound\n"
            << "  --fadein     Fade-in the sound\n"
            << "  --fadeout    Fade-out the sound\n"
            << "  --seek SEC   Seek to position SEC\n"
            << "  --effect FX  Add effect\n";
}

int str2effect(std::string text) {
  std::transform(text.begin(), text.end(), text.begin(), ::toupper);

  std::map<std::string, int> str2int = {
    { "REVERB", AL_EFFECT_REVERB },
    { "CHORUS", AL_EFFECT_CHORUS },
    { "DISTORTION", AL_EFFECT_DISTORTION },
    { "ECHO", AL_EFFECT_ECHO },
    { "FLANGER", AL_EFFECT_FLANGER },
    { "FREQUENCY_SHIFTER", AL_EFFECT_FREQUENCY_SHIFTER },
    { "VOCAL_MORPHER", AL_EFFECT_VOCAL_MORPHER },
    { "PITCH_SHIFTER", AL_EFFECT_PITCH_SHIFTER },
    { "RING_MODULATOR", AL_EFFECT_RING_MODULATOR },
    { "AUTOWAH", AL_EFFECT_AUTOWAH },
    { "COMPRESSOR", AL_EFFECT_COMPRESSOR },
    { "EQUALIZER", AL_EFFECT_EQUALIZER },
  };

  auto it = str2int.find(text);
  if (it == str2int.end()) {
    std::ostringstream os;
    os << "unknown effect string: " << text << ", valid values are:\n";
    for(auto const& it2 : str2int) {
      os << "   " << it2.first << "\n";
    }
    throw std::runtime_error(os.str());
  } else {
    return it->second;
  }
}

} // namespace

struct Options
{
  bool loop = false;
  SoundSourceType source_type = SoundSourceType::STREAM;
  float seek = 0;
  FadeState fade = FadeState::NoFading;
  int effect = AL_EFFECT_NULL;
};

int main(int argc, char** argv)
{
  if (argc < 2)
  {
    print_usage(argc, argv);
    return 1;
  }
  else
  {
    Options opts;
    std::vector<std::string> files;
    for (int i = 1; i < argc; ++i) {
      if (argv[i][0] == '-') {
        if (strcmp(argv[i], "--help") == 0) {
          print_usage(argc, argv);
          return 0;
        } else if (strcmp(argv[i], "--loop") == 0) {
          opts.loop = true;
        } else if (strcmp(argv[i], "--stream") == 0) {
          opts.source_type = SoundSourceType::STREAM;
        } else if (strcmp(argv[i], "--static") == 0) {
          opts.source_type = SoundSourceType::STATIC;
        } else if (strcmp(argv[i], "--seek") == 0) {
          if (++i >= argc) {
            throw std::runtime_error("--seek needs an argument");
          }
          opts.seek = std::stof(argv[i]);
        } else if (strcmp(argv[i], "--fadein") == 0) {
          opts.fade = FadeState::FadingOn;
        } else if (strcmp(argv[i], "--fadeout") == 0) {
          opts.fade = FadeState::FadingOff;

        } else if (strcmp(argv[i], "--effect") == 0) {
          if (++i >= argc) {
            throw std::runtime_error("--effect needs an argument");
          }
          opts.effect = str2effect(argv[i]);
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
      SoundSourcePtr source = sound_manager.sound().prepare(filename, opts.source_type);

      source->set_looping(opts.loop);

      if (opts.seek != 0) {
        source->seek_to(opts.seek);
      }

      if (opts.fade != FadeState::NoFading) {
        source->set_fading(opts.fade, 5.0f);
      }

      if (opts.effect != AL_EFFECT_NULL) {
        auto slot = sound_manager.create_effect_slot();
        slot->set_effect(sound_manager.create_effect(opts.effect));
        source->set_effect_slot(slot);
      }

      sources.emplace_back(source);
    }

    for (auto& source : sources) {
      source->play();
    }

    while (!sources.empty()) {
      for (auto& source : sources) {
        std::cout << "pos: " << source->get_sample_pos() << " / " << source->get_sample_duration() << std::endl;
      }

      std::erase_if(sources, [](SoundSourcePtr const& source){
        return !source->is_playing();
      });

      usleep(10000);

      sound_manager.update(0.01f);
    }
  }
}

/* EOF */
