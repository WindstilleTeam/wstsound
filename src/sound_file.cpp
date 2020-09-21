/*
**  Windstille - A Sci-Fi Action-Adventure Game
**  Copyright (C) 2005 Matthias Braun <matze@braunis.de>
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

/** Used SDL_mixer and glest source as reference */

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string.h>

#include "ogg_sound_file.hpp"
#include "opus_sound_file.hpp"
#include "wav_sound_file.hpp"

namespace wstsound {

std::unique_ptr<SoundFile>
SoundFile::from_stream(std::unique_ptr<std::istream> istream)
{
  char magic[64];

  if (!istream->read(magic, sizeof(magic))) {
    throw std::runtime_error("Couldn't read magic, file too short");
  } else {
    // reset the stream before handing it over
    istream->seekg(0, std::ios::beg);

    // FIXME: this is a bit of a hack, there are probably better ways
    // to tell OggVorbis and Opus appart
    if (strncmp(magic, "RIFF", 4) == 0) {
      return std::make_unique<WavSoundFile>(std::move(istream));
    } else if (strncmp(magic, "OggS", 4) == 0 && strncmp(magic + 28, "OpusHead", 8) == 0) {
      return std::make_unique<OpusSoundFile>(std::move(istream));
    } else if (strncmp(magic, "OggS", 4) == 0 && strncmp(magic + 29, "vorbis", 4) == 0) {
      return std::make_unique<OggSoundFile>(std::move(istream));
    } else {
      throw std::runtime_error("Unknown file format");
    }
  }
}

std::unique_ptr<SoundFile>
SoundFile::from_file(std::filesystem::path const& filename)
{
  std::ifstream in(filename, std::ios::binary);

  if (!in) {
    std::stringstream msg;
    msg << "Couldn't open '" << filename << "'";
    throw std::runtime_error(msg.str());
  } else {
    try {
      return from_stream(std::make_unique<std::ifstream>(std::move(in)));
    } catch(std::exception& e) {
      std::stringstream msg;
      msg << "Couldn't read '" << filename << "': " << e.what();
      throw std::runtime_error(msg.str());
    }
  }
}

float
SoundFile::get_duration() const
{
  return static_cast<float>(get_sample_duration()) / static_cast<float>(get_rate());
}

int
SoundFile::get_sample_duration() const
{
  return 8 * static_cast<int>(get_size()) / get_channels() / get_bits_per_sample();
}

} // namespace wstsound

/* EOF */