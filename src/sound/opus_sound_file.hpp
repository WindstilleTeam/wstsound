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

#ifndef HEADER_WINDSTILLE_SOUND_OPUS_SOUND_FILE_HPP
#define HEADER_WINDSTILLE_SOUND_OPUS_SOUND_FILE_HPP

#include <istream>
#include <memory>
#include <opusfile.h>

#include "sound/sound_file.hpp"

class OpusSoundFile : public SoundFile
{
public:
  OpusSoundFile(std::unique_ptr<std::istream> istream);
  ~OpusSoundFile() override;

  size_t read(void* buffer, size_t buffer_size) override;
  void reset() override;

  int    get_bits_per_sample() const override { return m_bits_per_sample; }
  size_t get_size() const override { return m_size; }
  int    get_rate() const override { return m_rate; }
  int    get_channels() const override { return m_channels; }

  void seek_to(float sec) override;

private:
  static int cb_read(void* stream, unsigned char* buffer, int nbytes);
  static int cb_seek(void* stream, opus_int64 offset, int whence);
  static int cb_close(void* stream);
  static opus_int64 cb_tell(void* stream);

private:
  std::unique_ptr<std::istream> m_istream;
  size_t m_file_size;
  OggOpusFile* m_opus_file;

  int m_channels;
  int m_rate;
  int m_bits_per_sample;
  size_t m_size; /// size in bytes

private:
  OpusSoundFile(const OpusSoundFile&);
  OpusSoundFile& operator=(const OpusSoundFile&);
};

#endif

/* EOF */
