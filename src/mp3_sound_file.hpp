/*
**  Windstille - A Sci-Fi Action-Adventure Game
**  Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_WSTSOUND_MP3_SOUND_FILE_HPP
#define HEADER_WSTSOUND_MP3_SOUND_FILE_HPP

#include <mpg123.h>
#include <vector>

#include "sound_file.hpp"

namespace wstsound {

class MP3SoundFile : public SoundFile
{
public:
  MP3SoundFile(std::unique_ptr<std::istream> istream);
  ~MP3SoundFile() override;

  size_t read(void* buffer, size_t buffer_size) override;
  void reset() override;

  int get_bits_per_sample() const override;
  size_t get_size() const override;
  int get_rate() const override;
  int get_channels() const override;

  void seek_to_sample(int sample) override;

private:
  static ssize_t cb_read(void* userdata, void* buffer, size_t nbytes);
  static off_t cb_lseek(void* userdata, off_t offset, int whence);
  static void cb_cleanup(void* userdata);

private:
  std::unique_ptr<std::istream> m_istream;
  mpg123_handle* m_mh;
  int m_samplerate;
  int m_channels;

private:
  MP3SoundFile(const MP3SoundFile&) = delete;
  MP3SoundFile& operator=(const MP3SoundFile&) = delete;
};

} // namespace wstsound

#endif

/* EOF */
