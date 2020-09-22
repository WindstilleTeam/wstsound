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
#include <iostream>

namespace wstsound {

namespace {

// taken from /usr/share/doc/libmad0-dev/examples/minimad.c.gz
static inline
signed int scale(mad_fixed_t sample)
{
  /* round */
  sample += (1L << (MAD_F_FRACBITS - 16));

  /* clip */
  if (sample >= MAD_F_ONE)
    sample = MAD_F_ONE - 1;
  else if (sample < -MAD_F_ONE)
    sample = -MAD_F_ONE;

  /* quantize */
  return sample >> (MAD_F_FRACBITS + 1 - 16);
}

} // namespace

MP3SoundFile::MP3SoundFile(std::unique_ptr<std::istream> istream) :
  m_istream(std::move(istream)),
  m_decoder(),
  m_samplerate(0),
  m_channels(0),
  m_file_buffer(),
  m_buffer(),
  m_byte_pos(0)
{
  mad_decoder_init(&m_decoder, this,
                   cb_input,
                   nullptr /* header */,
                   nullptr /* filter */,
                   cb_output,
                   cb_error,
                   nullptr /* message */);

  // this is crude and will decode the complete mp3
  mad_decoder_run(&m_decoder, MAD_DECODER_MODE_SYNC);
}

MP3SoundFile::~MP3SoundFile()
{
  mad_decoder_finish(&m_decoder);
}

size_t
MP3SoundFile::read(void* buffer, size_t buffer_size)
{
  uint8_t* const start = m_buffer.data() + m_byte_pos;
  uint8_t* const end = m_buffer.data() + std::min(m_buffer.size(), m_byte_pos + buffer_size);

  std::copy(start, end, static_cast<uint8_t*>(buffer));

  size_t len = end - start;
  m_byte_pos += len;
  return len;
}

void
MP3SoundFile::reset()
{
  m_byte_pos = 0;
}

void
MP3SoundFile::seek_to_sample(int sample)
{
  m_byte_pos = sample * get_channels() * get_bits_per_sample() / 8;
  m_byte_pos = std::clamp(m_byte_pos, static_cast<size_t>(0), m_buffer.size());
}

int
MP3SoundFile::get_bits_per_sample() const
{
  return 16;
}

size_t
MP3SoundFile::get_size() const
{
  return m_buffer.size();
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

mad_flow
MP3SoundFile::cb_input(void* data, mad_stream* stream)
{
  MP3SoundFile& self = *reinterpret_cast<MP3SoundFile*>(data);

  if (self.m_file_buffer.size() != 0) {
    return MAD_FLOW_STOP;
  }

  // get the file size
  self.m_istream->seekg(0, std::ios::end);
  size_t file_size = static_cast<size_t>(self.m_istream->tellg()) / 10;
  self.m_istream->seekg(0, std::ios::beg);

  self.m_file_buffer.resize(file_size);

  self.m_istream->read(reinterpret_cast<char*>(self.m_file_buffer.data()), self.m_file_buffer.size());

  //std::cerr << "OK: " << self.m_istream->gcount() << std::endl;
  mad_stream_buffer(stream, self.m_file_buffer.data(), self.m_istream->gcount());
  return MAD_FLOW_CONTINUE;
}

mad_flow
MP3SoundFile::cb_output(void* data, mad_header const* header, mad_pcm* pcm)
{
  MP3SoundFile& self = *reinterpret_cast<MP3SoundFile*>(data);

  self.m_samplerate = header->samplerate;
  self.m_channels = (header->mode == MAD_MODE_SINGLE_CHANNEL) ? 1 : 2;

  self.m_buffer.reserve(self.m_buffer.size() + pcm->length * pcm->channels * 2);
  for (unsigned int i = 0; i < pcm->length; ++i) {
    for(unsigned int c = 0; c < pcm->channels; ++c) {
      int const sample = scale(pcm->samples[c][i]);
      self.m_buffer.push_back(static_cast<uint8_t>((sample >> 0) & 0xff));
      self.m_buffer.push_back(static_cast<uint8_t>((sample >> 8) & 0xff));
    }
  }

  return MAD_FLOW_CONTINUE;
}

mad_flow
MP3SoundFile::cb_error(void* data, mad_stream* stream, mad_frame* frame)
{
  //MP3SoundFile& self = *reinterpret_cast<MP3SoundFile*>(data);
  //std::cerr << "cb_error: " << mad_stream_errorstr(stream) << "\n";
  return MAD_FLOW_CONTINUE;
}

} // namespace wstsound

/* EOF */
