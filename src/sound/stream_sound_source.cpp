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

#include "sound/stream_sound_source.hpp"

#include <array>
#include <iostream>

#include "sound/sound_manager.hpp"
#include "sound/sound_file.hpp"

StreamSoundSource::StreamSoundSource(SoundChannel& channel, std::unique_ptr<SoundFile> sound_file) :
  OpenALSoundSource(channel),
  m_sound_file(std::move(sound_file)),
  m_buffers(),
  m_format(SoundManager::get_sample_format(m_sound_file.get())),
  m_looping(false),
  m_total_buffers_processed(0),
  m_fade_state(),
  m_fade_start_ticks(),
  m_fade_time(),
  m_total_time(0.0f)
{
  alGenBuffers(static_cast<ALsizei>(m_buffers.size()), m_buffers.data());
  SoundManager::check_al_error("Couldn't allocate audio buffers: ");
}

StreamSoundSource::~StreamSoundSource()
{
  stop();

  alDeleteBuffers(static_cast<ALsizei>(m_buffers.size()), m_buffers.data());
  SoundManager::check_al_error("Couldn't delete audio buffers: ");
}

void
StreamSoundSource::set_looping(bool looping)
{
  // native OpenAL looping will result in the queue being looped, not
  // the whole song as provided by the SoundFile, so we do it manually
  m_looping = looping;
}

void
StreamSoundSource::seek_to(float sec)
{
  m_sound_file->seek_to(sec);

  if ((false))
  { // FIXME: clear the buffer or not on seek? see ov_time_seek_lap()
    // in OggSoundFile for possible reason why jumping might not be a good idea
    alSourceUnqueueBuffers(m_source, static_cast<ALsizei>(m_buffers.size()), m_buffers.data());
    SoundManager::check_al_error("Couldn't unqueue audio buffers: ");

    for(auto const& buffer : m_buffers) {
      fill_buffer_and_queue(buffer);
    }
  }
}

float
StreamSoundSource::get_pos() const
{
  return static_cast<float>(get_sample_pos()) / static_cast<float>(m_sound_file->get_rate());
}

int
StreamSoundSource::get_sample_pos() const
{
  int samples_total = m_total_buffers_processed * (static_cast<int>(STREAMFRAGMENTSIZE)
                                                   / m_sound_file->get_channels()
                                                   / (m_sound_file->get_bits_per_sample()/8));

  ALint sample_pos;
  alGetSourcei(m_source, AL_SAMPLE_OFFSET, &sample_pos);

  return (samples_total + sample_pos) % (static_cast<int>(m_sound_file->get_size())
                                         / m_sound_file->get_channels()
                                         / (m_sound_file->get_bits_per_sample()/8));
}

void
StreamSoundSource::play()
{
  for(auto const& buffer : m_buffers) {
    fill_buffer_and_queue(buffer);
  }

  OpenALSoundSource::play();
}

void
StreamSoundSource::update(float delta)
{
  m_total_time += delta;

  if (is_playing())
  {
    // fill the buffer queue with new data
    if (m_looping)
    {
      ALint processed = 0;
      alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &processed);

      while (processed > 0)
      {
        processed--;

        m_total_buffers_processed += 1;

        ALuint buffer;
        alSourceUnqueueBuffers(m_source, 1, &buffer);
        SoundManager::check_al_error("Couldn't unqueue audio buffer: ");

        fill_buffer_and_queue(buffer);
      }
    }

    // we might have to restart the source if we had a buffer underrun
    if (!is_playing())
    {
      std::cerr << "Restarting audio source because of buffer underrun.\n";
      play();
    }

    // handle fade-in/out
    if (m_fade_state == kFadingOn)
    {
      float time = m_fade_start_ticks - m_total_time;
      if (time >= m_fade_time)
      {
        set_gain(1.0);
        m_fade_state = kNoFading;
      }
      else
      {
        set_gain(time / m_fade_time);
      }
    }
    else if (m_fade_state == kFadingOff)
    {
      float time = m_fade_start_ticks - m_total_time;
      if (time >= m_fade_time)
      {
        stop();
        m_fade_state = kNoFading;
      }
      else
      {
        set_gain( (m_fade_time - time) / m_fade_time);
      }
    }
  }
}

void
StreamSoundSource::set_fading(FadeState fade_state, float fade_time)
{
  m_fade_state       = fade_state;
  m_fade_time        = fade_time;
  m_fade_start_ticks = m_total_time;
}

void
StreamSoundSource::fill_buffer_and_queue(ALuint buffer)
{
  std::array<char, STREAMFRAGMENTSIZE> bufferdata;
  size_t total_bytesread = 0;

  // fill buffer with data from m_sound_file
  do {
    size_t const bytesrequested = STREAMFRAGMENTSIZE - total_bytesread;
    size_t const bytesread = m_sound_file->read(bufferdata.data() + total_bytesread,
                                                bytesrequested);
    total_bytesread += bytesread;

    if (bytesread < bytesrequested) // EOF reached
    {
      if (m_looping) {
        m_sound_file->reset();
      } else {
        break;
      }
    }
  } while(total_bytesread < STREAMFRAGMENTSIZE);

  if (total_bytesread > 0)
  {
    // upload data to the OpenAL buffer
    alBufferData(buffer, m_format, bufferdata.data(), static_cast<ALsizei>(total_bytesread), m_sound_file->get_rate());
    SoundManager::check_al_error("Couldn't refill audio buffer: ");

    // add buffer to the queue of this source
    alSourceQueueBuffers(m_source, 1, &buffer);
    SoundManager::check_al_error("Couldn't queue audio buffer: ");
  }
}

/* EOF */
