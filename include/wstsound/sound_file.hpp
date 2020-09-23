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

#ifndef HEADER_WINDSTILLE_SOUND_SOUND_FILE_HPP
#define HEADER_WINDSTILLE_SOUND_SOUND_FILE_HPP

#include <filesystem>
#include <memory>
#include <string>

namespace wstsound {

/** SoundFile gives access to .wav, .ogg, .opus, .mod or .mp3 files as
    if they were one continous stream of PCM samples. */
class SoundFile
{
public:
  SoundFile() {}
  virtual ~SoundFile() {}

  /** Read bytes into buffer */
  virtual size_t read(void* buffer, size_t buffer_size) = 0;

  /** Move the current position in the virtual file to 'sample' */
  virtual void seek_to_sample(int sample) = 0;

  /** Bits per sample, usually 8 or 16 */
  virtual int get_bits_per_sample() const = 0;

  /** The size of the virtual file in bytes */
  virtual size_t get_size() const = 0;

  /** The sample rate or frequency of the file, usually 44100 or 48000 */
  virtual int get_rate() const = 0;

  /** The number of channels, 1 for mono, 2 for stereo */
  virtual int get_channels() const = 0;

  /** Returns the length of the file in seconds */
  float get_duration() const;

  /** Returns the number of samples in the file */
  int get_sample_duration() const;

public:
  static std::unique_ptr<SoundFile> from_file(std::filesystem::path const& filename);
  static std::unique_ptr<SoundFile> from_stream(std::unique_ptr<std::istream> istream);
};

} // namespace wstsound

#endif

/* EOF */
