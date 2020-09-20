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
  m_format(OpenALSystem::get_sample_format(m_sound_file.get())),
  m_looping(false),
  m_total_samples_processed(0),
  m_fade_state(),
  m_fade_start_ticks(),
  m_fade_time(),
  m_total_time(0.0f)
{
  alGenBuffers(static_cast<ALsizei>(m_buffers.size()), m_buffers.data());
  OpenALSystem::check_al_error("Couldn't allocate audio buffers: ");
}

StreamSoundSource::~StreamSoundSource()
{
  stop();

  alDeleteBuffers(static_cast<ALsizei>(m_buffers.size()), m_buffers.data());
  OpenALSystem::check_al_error("Couldn't delete audio buffers: ");
}

void
StreamSoundSource::set_looping(bool looping)
{
  // native OpenAL looping will result in the queue being looped, not
  // the whole song as provided by the SoundFile, so we do it manually
  m_looping = looping;
}

void
StreamSoundSource::seek_to_sample(int sample)
{
  bool const was_playing = is_playing();

  if (was_playing) {
    // stop the source to cause all buffers getting marked as
    // processed, so we can unqueue them
    alSourceStop(m_source);
  }

  m_sound_file->seek_to_sample(sample);

  m_total_samples_processed = sample;

  ALint queued = 0;
  alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &queued);

  {
    ALint processed = 0;
    alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &processed);

    std::array<ALuint, STREAMFRAGMENTS> unqueue_buffers;
    alSourceUnqueueBuffers(m_source, processed, unqueue_buffers.data());
    OpenALSystem::check_al_error("Couldn't unqueue audio buffer: ");

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
    {
      ALint processed = 0;
      alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &processed);

      while (processed > 0)
      {
        processed--;

        m_total_samples_processed += samples_per_buffer();

        ALuint buffer;
        alSourceUnqueueBuffers(m_source, 1, &buffer);
        OpenALSystem::check_al_error("Couldn't unqueue audio buffer: ");

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
    if (bytesread == 0) { // EOF reached
      if (m_looping) {
        m_sound_file->reset();
      } else {
        break;
      }
    }
    total_bytesread += bytesread;
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

/* EOF */
