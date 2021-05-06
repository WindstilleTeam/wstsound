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

#include "stream_sound_source.hpp"

#include <array>
#include <iostream>

#include "sound_manager.hpp"
#include "sound_file.hpp"

namespace wstsound {

StreamSoundSource::StreamSoundSource(SoundChannel& channel, std::unique_ptr<SoundFile> sound_file) :
  OpenALSoundSource(channel),
  m_sound_file(std::move(sound_file)),
  m_buffers(),
  m_format(OpenALSystem::get_sample_format(*m_sound_file)),
  m_total_samples_processed(0),
  m_playing(false),
  m_loop()
{
  alGenBuffers(static_cast<ALsizei>(m_buffers.size()), m_buffers.data());
  OpenALSystem::check_al_error("Couldn't allocate audio buffers: ");
}

StreamSoundSource::~StreamSoundSource()
{
  stop();

  alDeleteBuffers(static_cast<ALsizei>(m_buffers.size()), m_buffers.data());
  OpenALSystem::warn_al_error("Couldn't delete audio buffers: ");
}

void
StreamSoundSource::set_looping(bool looping)
{
  // Native OpenAL looping will result in only the queue being looped, not
  // the whole song as provided by the SoundFile, so we do it manually.
  if (looping) {
    m_loop = Loop{0, m_sound_file->get_sample_duration()};
  } else {
    m_loop = std::nullopt;
  }
}

void
StreamSoundSource::set_loop(int sample_beg, int sample_end)
{
  m_loop = Loop{sample_beg % m_sound_file->get_sample_duration(), sample_end % m_sound_file->get_sample_duration()};
}

void
StreamSoundSource::seek_to_sample(int sample)
{
  bool const was_playing = m_playing;

  if (was_playing) {
    // stop the source to cause all buffers getting marked as
    // processed, so we can unqueue them
    alSourceStop(m_source);
  }

  m_sound_file->seek_to_sample(sample);

  m_total_samples_processed = sample;

  {
    ALint processed = 0;
    alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &processed);

    std::array<ALuint, STREAMFRAGMENTS> unqueue_buffers;
    alSourceUnqueueBuffers(m_source, processed, unqueue_buffers.data());
    OpenALSystem::warn_al_error("Couldn't unqueue audio buffer: ");

    for(int i = 0; i < processed; ++i) {
      fill_buffer_and_queue(unqueue_buffers[i]);
    }
  }

  if (was_playing) {
    alSourcePlay(m_source);
  }
}

void
StreamSoundSource::seek_to(float sec)
{
  seek_to_sample(sec_to_sample(sec));
}

float
StreamSoundSource::get_pos() const
{
  return sample_to_sec(get_sample_pos());
}

int
StreamSoundSource::get_sample_pos() const
{
  ALint sample_offset;
  alGetSourcei(m_source, AL_SAMPLE_OFFSET, &sample_offset);

  return (m_total_samples_processed + sample_offset);
}

int
StreamSoundSource::get_sample_duration() const
{
  return m_sound_file->get_sample_duration();
}

float
StreamSoundSource::get_duration() const
{
  return m_sound_file->get_duration();
}

void
StreamSoundSource::play()
{
  m_playing = true;

  for(auto const& buffer : m_buffers) {
    fill_buffer_and_queue(buffer);
  }

  OpenALSoundSource::play();
}

void
StreamSoundSource::update(float delta)
{
  OpenALSoundSource::update(delta);

  if (!OpenALSoundSource::is_playing() && !m_loop) {
    m_playing = false;
  }

  if (m_playing)
  {
    { // fill the buffer queue with new data
      ALint processed = 0;
      alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &processed);

      std::array<ALuint, STREAMFRAGMENTS> unqueue_buffers;
      alSourceUnqueueBuffers(m_source, processed, unqueue_buffers.data());
      OpenALSystem::warn_al_error("Couldn't unqueue audio buffer: ");

      for(int i = 0; i < processed; ++i) {
        fill_buffer_and_queue(unqueue_buffers[i]);
        m_total_samples_processed += samples_per_buffer();
      }
    }

    // we might have to restart the source if we had a buffer underrun
    if (!OpenALSoundSource::is_playing())
    {
      std::cerr << "Restarting audio source because of buffer underrun.\n";
      OpenALSoundSource::play();
    }
  }
}

void
StreamSoundSource::fill_buffer_and_queue(ALuint buffer)
{
  std::array<char, STREAMFRAGMENTSIZE> bufferdata;
  size_t total_bytesread = 0;

  // fill buffer with data from m_sound_file
  do {
    size_t bytesrequested = STREAMFRAGMENTSIZE - total_bytesread;

    if (m_loop) {
      bytesrequested = std::min(m_sound_file->sample2bytes(m_loop->sample_end) - m_sound_file->tell(),
                                bytesrequested);
    }

    size_t const bytesread = m_sound_file->read(bufferdata.data() + total_bytesread,
                                                bytesrequested);
    total_bytesread += bytesread;

    if (m_loop) {
      if (m_sound_file->tell() >= m_sound_file->sample2bytes(m_loop->sample_end)) {
        m_sound_file->seek_to_sample(m_loop->sample_beg);
      }
    } else {
      if (bytesread == 0) {
        /* EOF reached */
        break;
      }
    }
  } while(total_bytesread < STREAMFRAGMENTSIZE);

  if (total_bytesread > 0)
  {
    // upload data to the OpenAL buffer
    alBufferData(buffer, m_format, bufferdata.data(), static_cast<ALsizei>(total_bytesread), m_sound_file->get_rate());
    OpenALSystem::check_al_error("Couldn't refill audio buffer: ");

    // add buffer to the queue of this source
    alSourceQueueBuffers(m_source, 1, &buffer);
    OpenALSystem::check_al_error("Couldn't queue audio buffer: ");
  }
}

float
StreamSoundSource::sample_to_sec(int sample) const
{
  return static_cast<float>(sample) / static_cast<float>(m_sound_file->get_rate());
}

int
StreamSoundSource::sec_to_sample(float sec) const
{
  return static_cast<int>(sec * static_cast<float>(m_sound_file->get_rate()));
}

int
StreamSoundSource::samples_per_buffer() const
{
  return (8 * static_cast<int>(STREAMFRAGMENTSIZE)
          / m_sound_file->get_channels()
          / m_sound_file->get_bits_per_sample());
}

} // namespace wstsound

/* EOF */
