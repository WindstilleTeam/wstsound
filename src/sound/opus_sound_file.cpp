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

#include "sound/opus_sound_file.hpp"

OpusSoundFile::OpusSoundFile(std::unique_ptr<std::istream> istream) :
  m_istream(std::move(istream)),
  m_file_size(0),
  m_opus_file(nullptr),
  m_channels(0),
  m_rate(48000),
  m_bits_per_sample(16),
  m_size(0)

{
  // get the file size
  m_istream->seekg(0, std::ios::end);
  m_file_size = static_cast<size_t>(m_istream->tellg());
  m_istream->seekg(0, std::ios::beg);

  OpusFileCallbacks callbacks = { cb_read, cb_seek, cb_tell, cb_close };

  int err;
  m_opus_file = op_open_callbacks(this, &callbacks, nullptr, 0, &err);
  if (m_opus_file == nullptr)
  {
    std::ostringstream str;

    switch(err)
    {
      case OP_EREAD:
      case OP_EFAULT:
      case OP_EIMPL:
      case OP_EINVAL:
      case OP_ENOTFORMAT:
      case OP_EBADHEADER:
      case OP_EVERSION:
      case OP_EBADLINK:
      case OP_EBADTIMESTAMP:
      default:
        str << "op_open_callbacks() failed with " << err;
    }

    throw std::runtime_error(str.str());
  }

  m_channels = op_channel_count(m_opus_file, -1);
  m_size = static_cast<size_t>(op_pcm_total(m_opus_file, -1) * 2);
}

OpusSoundFile::~OpusSoundFile()
{
  op_free(m_opus_file);
}

size_t
OpusSoundFile::read(void* buffer_, size_t buffer_size)
{
  opus_int16* buffer = reinterpret_cast<opus_int16*>(buffer_);

  // returns number of samples per channel, not bytes
  int pcm_read = op_read(m_opus_file, buffer, static_cast<int>(buffer_size / sizeof(opus_int16)), nullptr);

  return pcm_read * m_channels;
}

void
OpusSoundFile::reset()
{
  op_raw_seek(m_opus_file, 0);
}

void
OpusSoundFile::seek_to(float sec)
{
  op_pcm_seek(m_opus_file, static_cast<ogg_int64_t>(sec * 48000));
}

int
OpusSoundFile::cb_read(void* stream, unsigned char* buffer, int nbytes)
{
  OpusSoundFile& opus = *reinterpret_cast<OpusSoundFile*>(stream);

  size_t read_len = nbytes;

  // prevent std::istream from hitting eof(), needed as tellg() will
  // return -1 in that case instead of the from cb_tell expected filesize
  read_len = std::min(read_len, opus.m_file_size - opus.m_istream->tellg());

  return static_cast<int>(opus.m_istream->read(reinterpret_cast<char*>(buffer), read_len).gcount());
}

int
OpusSoundFile::cb_seek(void* stream, opus_int64 offset, int whence)
{
  OpusSoundFile& opus = *reinterpret_cast<OpusSoundFile*>(stream);

  switch(whence)
  {
    case SEEK_SET:
      //std::cout << "OpusSoundFile::cb_seek: " << offset << " BEG" << std::endl;
      if (!opus.m_istream->seekg(offset, std::ios::beg))
        return -1;
      break;

    case SEEK_CUR:
      //std::cout << "OpusSoundFile::cb_seek: " << offset << " CUR" << std::endl;
      if (!opus.m_istream->seekg(offset, std::ios::cur))
        return -1;
      break;

    case SEEK_END:
      //std::cout << "OpusSoundFile::cb_seek: " << offset << " END" << std::endl;
      if (!opus.m_istream->seekg(offset, std::ios::end))
        return -1;
      break;

    default:
      return -1;
  }
  return 0;
}

int
OpusSoundFile::cb_close(void* stream)
{
  OpusSoundFile& opus = *reinterpret_cast<OpusSoundFile*>(stream);
  opus.m_istream.reset();
  return 0;
}

long
OpusSoundFile::cb_tell(void* stream)
{
  OpusSoundFile& opus = *reinterpret_cast<OpusSoundFile*>(stream);
  return static_cast<long>(opus.m_istream->tellg());
}

/* EOF */
