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

#include <algorithm>
#include <bit>
#include <filesystem>
#include <stdexcept>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <typeinfo>

#include "sound/wav_sound_file.hpp"

namespace {

template <class To, class From>
To bit_cast(const From& src) noexcept
{ // FIXME: replace with std::bit_cast when available
  To dst;
  memcpy(&dst, &src, sizeof(To));
  return dst;
}

template<typename T>
T read_type(std::istream& in)
{
  char data[sizeof(T)];

  if (!in.read(data, sizeof(data)))
  {
    std::ostringstream msg;
    msg << "Problem reading " << typeid(T).name() << ": " << strerror(errno);
    throw std::runtime_error(msg.str());
  }
  else
  {
    if constexpr (std::endian::native == std::endian::big) {
      std::reverse(std::begin(data), std::end(data));
    }

    return bit_cast<T>(data);
  }
}

} // namespace

WavSoundFile::WavSoundFile(std::unique_ptr<std::istream> istream) :
  m_istream(std::move(istream)),
  m_datastart(),
  m_channels(),
  m_rate(),
  m_bits_per_sample(),
  m_size()
{
  char magic[4];
  if (!m_istream->read(magic, sizeof(magic)))
  {
    throw std::runtime_error("Couldn't read file magic (not a wave file)");
  }

  if(strncmp(magic, "RIFF", 4) != 0)
  {
    printf("MAGIC: %4s.\n", magic);
    throw std::runtime_error("file is not a RIFF wav file");
  }

  /*uint32_t wavelen =*/ read_type<uint32_t>(*m_istream);

  if (!m_istream->read( magic, sizeof(magic)))
  {
    throw std::runtime_error("Couldn't read chunk header (not a wav file?)");
  }

  if(strncmp(magic, "WAVE", 4) != 0)
  {
    throw std::runtime_error("file is not a valid RIFF/WAVE file");
  }

  char chunkmagic[4];
  uint32_t chunklen;

  // search audio data format chunk
  do {
    if (!m_istream->read(chunkmagic, sizeof(chunkmagic)))
    {
      throw std::runtime_error("EOF while searching format chunk");
    }
    chunklen = read_type<uint32_t>(*m_istream);

    if (strncmp(chunkmagic, "fmt ", 4) == 0)
    {
      break;
    }
    else if (strncmp(chunkmagic, "fact", 4) == 0 ||
             strncmp(chunkmagic, "LIST", 4) == 0)
    {
      // skip chunk

      if (!m_istream->seekg(chunklen, std::ios::cur))
      {
        throw std::runtime_error("EOF while searching fmt chunk");
      }
    }
    else
    {
      throw std::runtime_error("complex WAVE files not supported");
    }
  } while(true);

  if (chunklen < 16)
    throw std::runtime_error("Format chunk too short");

  // parse format
  uint16_t encoding = read_type<uint16_t>(*m_istream);
  if (encoding != 1)
  {
    std::ostringstream str;
    str << "WavSoundFile(): only PCM encoding supported, got " << encoding;
    throw std::runtime_error(str.str());
  }
  m_channels = read_type<uint16_t>(*m_istream);
  m_rate = read_type<uint32_t>(*m_istream);
  /*uint32_t byterate =*/ read_type<uint32_t>(*m_istream);
  /*uint16_t blockalign =*/ read_type<uint16_t>(*m_istream);
  m_bits_per_sample = read_type<uint16_t>(*m_istream);

  if(chunklen > 16)
  {
    if(m_istream->seekg(chunklen-16, std::ios::cur).fail())
      throw std::runtime_error("EOF while reading reast of format chunk");
  }

  // set file offset to DATA chunk data
  do {
    if (!m_istream->read(chunkmagic, sizeof(chunkmagic)))
      throw std::runtime_error("EOF while searching data chunk");
    chunklen = read_type<uint32_t>(*m_istream);

    if(strncmp(chunkmagic, "data", 4) == 0)
      break;

    // skip chunk
    if(m_istream->seekg(chunklen, std::ios::cur).fail())
      throw std::runtime_error("EOF while searching fmt chunk");
  } while(true);

  m_datastart = m_istream->tellg();
  m_size = static_cast<size_t>(chunklen);
}

WavSoundFile::~WavSoundFile()
{
}

void
WavSoundFile::reset()
{
  if (!m_istream->seekg(m_datastart)) {
    throw std::runtime_error("Couldn't seek to data start");
  }
}

void
WavSoundFile::seek_to(float sec)
{
  std::streamoff byte_pos = static_cast<size_t>(sec * static_cast<float>(m_rate * m_bits_per_sample/8 * m_channels));

  if (!m_istream->seekg(m_datastart + byte_pos))
    throw std::runtime_error("Couldn't seek to data start");
}

size_t
WavSoundFile::read(void* buffer, size_t buffer_size)
{
  if (!m_istream->read(static_cast<char*>(buffer), buffer_size))
  {
    if (!m_istream->eof()) {
      throw std::runtime_error("read error while reading samples");
    }
    m_istream->clear();
    return m_istream->gcount();
  }
  else
  {
    std::streamsize bytesread = m_istream->gcount();

    // handle endian swaping
    if constexpr (std::endian::native == std::endian::big) {
      char* p = static_cast<char*>(buffer);
      for(size_t i = 0; i < bytesread; i += sizeof(uint16_t)) {
        std::reverse(p, p + sizeof(uint16_t));
      }
    }

    return bytesread;
  }
}

/* EOF */
