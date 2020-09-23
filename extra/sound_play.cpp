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
#include <variant>

#include <wstsound/effect.hpp>
#include <wstsound/effect_slot.hpp>
#include <wstsound/filter.hpp>
#include <wstsound/filtered_sound_file.hpp>
#include <wstsound/sound_file.hpp>
#include <wstsound/sound_manager.hpp>
#include <wstsound/sound_source.hpp>

using namespace wstsound;

namespace {

std::vector<std::string> string_split(std::string_view text, char delimiter)
{
  std::vector<std::string> result;

  std::string::size_type start = 0;
  for(std::string::size_type i = 0; i != text.size(); ++i)
  {
    if (text[i] == delimiter)
    {
      result.emplace_back(text.substr(start, i - start));
      start = i + 1;
    }
  }

  result.emplace_back(text.substr(start));

  return result;
}

void print_usage(int argc, char** argv)
{
  std::cout << "Usage: " << argv[0] << " [OPTION]... SOUNDS...\n"
            << "\n"
            << "  --help             Display this help text\n"
            << "  --loop             Loopt the sound\n"
            << "  --fadein           Fade-in the sound\n"
            << "  --fadeout          Fade-out the sound\n"
            << "  --seek SEC         Seek to position SEC\n"
            << "  --effect FX        Add effect\n"
            << "  --fx-param VALUE:...\n"
            << "                     List of effects parameter\n"
            << "  --filter FILTER    Add filter\n"
            << "  --flt-param VALUE:...\n"
            << "                     List of filter parameter\n"
            << "  --direct-filter FILTER    Add filter\n"
            << "  --dflt-param VALUE:...\n"
            << "                     List of direct filter parameter\n";
}

int str2filter(std::string text) {
  std::transform(text.begin(), text.end(), text.begin(), ::toupper);

  static std::map<std::string, int> const str2int = {
    { "LOWPASS", AL_FILTER_LOWPASS },
    { "HIGHPASS", AL_FILTER_HIGHPASS },
    { "BANDPASS", AL_FILTER_BANDPASS },
  };

  auto it = str2int.find(text);
  if (it == str2int.end()) {
    std::ostringstream os;
    os << "unknown filter string: " << text << ", valid values are:\n";
    for(auto const& it2 : str2int) {
      os << "   " << it2.first << "\n";
    }
    throw std::runtime_error(os.str());
  } else {
    return it->second;
  }
}

int str2effect(std::string text) {
  std::transform(text.begin(), text.end(), text.begin(), ::toupper);

  static std::map<std::string, int> const str2int = {
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

struct Options
{
  std::vector<std::string> files = {};
  bool loop = false;
  SoundSourceType source_type = SoundSourceType::STREAM;
  float seek = 0;
  FadeState fade = FadeState::NoFading;
  int effect = AL_EFFECT_NULL;
  std::vector<std::optional<std::variant<float, int>>> fxparam = {};
  int filter = AL_FILTER_NULL;
  std::vector<std::optional<std::variant<float, int>>> fltparam = {};
  int direct_filter = AL_FILTER_NULL;
  std::vector<std::optional<std::variant<float, int>>> dfltparam = {};
};

Options parse_args(int argc, char** argv)
{
  if (argc < 2) {
    print_usage(argc, argv);
    exit(EXIT_SUCCESS);
  }

  Options opts;

  for (int i = 1; i < argc; ++i) {
    auto next_arg = [&]{
      if (++i >= argc) {
        std::ostringstream msg;
        msg << argv[i - 1] << " needs an argument";
        throw std::runtime_error(msg.str());
      }
    };

    auto arg_parse_list = [&](){
      std::vector<std::optional<std::variant<float, int>>> result;
      auto const& values = string_split(argv[i], ':');
      for(auto const& value : values) {
        if (!value.empty()) {
          if (value.back() == 'f') {
            result.emplace_back(std::stof(value));
          } else if (value.back() == 'i') {
            result.emplace_back(std::stoi(value));
          } else {
            std::ostringstream os;
            os << argv[i - 1] << " must have a 'i' or 'f' suffix to indicate their type";
            throw std::runtime_error(os.str());
          }
        } else {
          result.emplace_back();
        }
      }
      return result;
    };

    if (argv[i][0] == '-') {
      if (strcmp(argv[i], "--help") == 0) {
        print_usage(argc, argv);
        exit(EXIT_SUCCESS);
      } else if (strcmp(argv[i], "--loop") == 0) {
        opts.loop = true;
      } else if (strcmp(argv[i], "--stream") == 0) {
        opts.source_type = SoundSourceType::STREAM;
      } else if (strcmp(argv[i], "--static") == 0) {
        opts.source_type = SoundSourceType::STATIC;
      } else if (strcmp(argv[i], "--seek") == 0) {
        next_arg();
        opts.seek = std::stof(argv[i]);
      } else if (strcmp(argv[i], "--fadein") == 0) {
        opts.fade = FadeState::FadingOn;
      } else if (strcmp(argv[i], "--fadeout") == 0) {
        opts.fade = FadeState::FadingOff;
      } else if (strcmp(argv[i], "--effect") == 0) {
        next_arg();
        opts.effect = str2effect(argv[i]);
      } else if (strcmp(argv[i], "--fx-param") == 0) {
        next_arg();
        opts.fxparam = arg_parse_list();
      } else if (strcmp(argv[i], "--filter") == 0) {
        next_arg();
        opts.filter = str2filter(argv[i]);
      } else if (strcmp(argv[i], "--flt-param") == 0) {
        next_arg();
        opts.fltparam = arg_parse_list();
      } else if (strcmp(argv[i], "--direct-filter") == 0) {
        next_arg();
        opts.direct_filter = str2filter(argv[i]);
      } else if (strcmp(argv[i], "--dflt-param") == 0) {
        next_arg();
        opts.dfltparam = arg_parse_list();
      } else {
        std::ostringstream os;
        os << "unknown option " << argv[i];
        throw std::runtime_error(os.str());
      }
    } else {
      opts.files.emplace_back(argv[i]);
    }
  }

  return opts;
}

int run(int argc, char** argv)
{
  Options opts = parse_args(argc, argv);

  SoundManager sound_manager;

  sound_manager.set_gain(1.0f);
  sound_manager.sound().set_gain(1.0f);
  sound_manager.voice().set_gain(1.0f);

  std::vector<SoundSourcePtr> sources;
  for (auto const& filename : opts.files)
  {
    SoundSourcePtr source = sound_manager.sound().prepare(filename, opts.source_type);

    source->set_looping(opts.loop);

    if (opts.seek != 0) {
      source->seek_to(opts.seek);
    }

    if (opts.fade != FadeState::NoFading) {
      source->set_fading(opts.fade, 5.0f);
    }

    if (opts.direct_filter != AL_FILTER_NULL) {
      FilterPtr direct_filter = sound_manager.create_filter(opts.direct_filter);
      for (size_t i = 0; i < opts.dfltparam.size(); ++i) {
        if (opts.dfltparam[i]) {
          if (std::holds_alternative<int>(*opts.dfltparam[i])) {
            direct_filter->seti(static_cast<int>(i) + 1, std::get<int>(*opts.dfltparam[i]));
          } else if (std::holds_alternative<float>(*opts.dfltparam[i])) {
            direct_filter->setf(static_cast<int>(i) + 1, std::get<float>(*opts.dfltparam[i]));
          }
        }
      }
      source->set_direct_filter(direct_filter);
    }

    if (opts.effect != AL_EFFECT_NULL || opts.filter != AL_FILTER_NULL) {
      auto slot = sound_manager.create_effect_slot();

      if (opts.effect != AL_EFFECT_NULL) {
        auto effect = sound_manager.create_effect(opts.effect);
        for (size_t i = 0; i < opts.fxparam.size(); ++i) {
          if (opts.fxparam[i]) {
            if (std::holds_alternative<int>(*opts.fxparam[i])) {
              effect->seti(static_cast<int>(i) + 1, std::get<int>(*opts.fxparam[i]));
            } else if (std::holds_alternative<float>(*opts.fxparam[i])) {
              effect->setf(static_cast<int>(i) + 1, std::get<float>(*opts.fxparam[i]));
            }
          }
        }
        slot->set_effect(effect);
      }

      FilterPtr filter;
      if (opts.filter != AL_FILTER_NULL) {
        filter = sound_manager.create_filter(opts.filter);
        for (size_t i = 0; i < opts.fltparam.size(); ++i) {
          if (opts.fltparam[i]) {
            if (std::holds_alternative<int>(*opts.fltparam[i])) {
              filter->seti(static_cast<int>(i) + 1, std::get<int>(*opts.fltparam[i]));
            } else if (std::holds_alternative<float>(*opts.fltparam[i])) {
              filter->setf(static_cast<int>(i) + 1, std::get<float>(*opts.fltparam[i]));
            }
          }
        }
      }

      source->set_effect_slot(slot, filter);
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

  return 0;
}

} // namespace

int main(int argc, char** argv) {
  try {
    return run(argc, argv);
  } catch (std::exception const& err) {
    std::cerr << "error: " << err.what() << std::endl;
    return EXIT_FAILURE;
  }
}

/* EOF */
