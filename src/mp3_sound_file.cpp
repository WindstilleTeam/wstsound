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

#include "mp3_sound_file.hpp"

#include <algorithm>
#include <assert.h>
#include <sstream>

class Mpg123System
{
public:
  Mpg123System() {
    int ret;
    ret = mpg123_init();
    if (ret != MPG123_OK) {
      throw std::runtime_error("mpg123_init() failed");
    }
  }

  ~Mpg123System() {
    mpg123_exit();
  }
};

namespace wstsound {

MP3SoundFile::MP3SoundFile(std::unique_ptr<std::istream> istream) :
  m_istream(std::move(istream)),
  m_mh(nullptr),
  m_samplerate(0),
  m_channels(0)
{
  // This is never cleaned up
  static Mpg123System mpg123_system;

  int ret;

  m_mh = mpg123_new(nullptr, &ret);

  ret = mpg123_replace_reader_handle(m_mh, cb_read, cb_lseek, cb_cleanup);
  if (ret != MPG123_OK) {
    throw std::runtime_error("mpg123_replace_reader_handle() failed");
  }

  ret = mpg123_open_handle(m_mh, this);
  if (ret != MPG123_OK) {
    throw std::runtime_error("mpg123_open_handle() failed");
  }

  // set what we want
  //mpg123_format(m_mh, 48000, MPG123_STEREO | MPG123_MONO, MPG123_ENC_SIGNED_16);

  // see what we got
  long rate;
  int channels;
  int encoding;
  mpg123_getformat(m_mh, &rate, &channels, &encoding);

  m_samplerate = static_cast<int>(rate);
  m_channels = channels;
}

MP3SoundFile::~MP3SoundFile()
{
  mpg123_close(m_mh);
}

size_t
MP3SoundFile::read(void* buffer, size_t buffer_size)
{
  size_t bytes_decoded;
  int ret = mpg123_read(m_mh,
                        static_cast<unsigned char*>(buffer), buffer_size,
                        &bytes_decoded);
  switch(ret) {
    case MPG123_OK:
    case MPG123_DONE:
      return bytes_decoded;

    default: {
      std::ostringstream os;
      os << "mpg123_read() failed with " << ret;
      throw std::runtime_error(os.str());
    }
  }
}

void
MP3SoundFile::seek_to_sample(int sample)
{
  if (mpg123_seek(m_mh, sample, SEEK_SET) < 0) {
    throw std::runtime_error("mpg123_seek() failed");
  }
}

int
MP3SoundFile::get_bits_per_sample() const
{
  return 16;
}

size_t
MP3SoundFile::get_size() const
{
  off_t samples_len = mpg123_length(m_mh);
  if (samples_len < 0) {
    return 0;
  }  else {
    return samples_len * get_channels() * get_bits_per_sample() / 8;
  }
}

int
MP3SoundFile::get_rate() const
{
  return m_samplerate;
}

int
MP3SoundFile::get_channels() const
{
  return m_channels;
}

ssize_t
MP3SoundFile::cb_read(void* userdata, void* buffer, size_t nbytes)
{
  MP3SoundFile& self = *reinterpret_cast<MP3SoundFile*>(userdata);

  if (!self.m_istream->read(reinterpret_cast<char*>(buffer), nbytes))
  {
    if (!self.m_istream->eof()) {
      return -1;
    } else {
      self.m_istream->clear(std::ios::eofbit);
    }
  }

  return static_cast<int>(self.m_istream->gcount());
}

off_t
MP3SoundFile::cb_lseek(void* userdata, off_t offset, int whence)
{
  MP3SoundFile& self = *reinterpret_cast<MP3SoundFile*>(userdata);

  switch(whence)
  {
    case SEEK_SET:
      if (!self.m_istream->seekg(offset, std::ios::beg)) {
        return -1;
      }
      break;

    case SEEK_CUR:
      if (!self.m_istream->seekg(offset, std::ios::cur)) {
        return -1;
      }
      break;

    case SEEK_END:
      if (!self.m_istream->seekg(offset, std::ios::end)) {
        return -1;
      }
      break;

    default:
      assert(false && "incorrect whence value");
  }
  return self.m_istream->tellg();
}

void
MP3SoundFile::cb_cleanup(void* userdata)
{
 MP3SoundFile& self = *reinterpret_cast<MP3SoundFile*>(userdata);

 self.m_istream.reset();
}

} // namespace wstsound

/* EOF */
