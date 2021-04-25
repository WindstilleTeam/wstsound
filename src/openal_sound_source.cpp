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

#include "openal_sound_source.hpp"

#include <assert.h>
#include <iostream>

#define AL_ALEXT_PROTOTYPES
#include <efx.h>

#include "effect_slot.hpp"
#include "filter.hpp"
#include "sound_manager.hpp"

namespace wstsound {

OpenALSoundSource::OpenALSoundSource(SoundChannel& channel) :
  m_channel(channel),
  m_source(),
  m_gain(1.0f),
  m_direct_filter(),
  m_filter(),
  m_effect_slot()
{
  alGenSources(1, &m_source);
  OpenALSystem::check_al_error("Couldn't create audio source: ");
  set_reference_distance(128);
}

OpenALSoundSource::~OpenALSoundSource()
{
  stop();
  alDeleteSources(1, &m_source);
  OpenALSystem::check_al_error("Couldn't delete source: ");
}

void
OpenALSoundSource::stop()
{
  alSourceStop(m_source);
  alSourcei(m_source, AL_BUFFER, AL_NONE);
  OpenALSystem::check_al_error("Problem stopping audio source: ");
}

void
OpenALSoundSource::play()
{
  alSourcePlay(m_source);
  OpenALSystem::check_al_error("Couldn't start audio source: ");
}

void
OpenALSoundSource::seek_to(float sec)
{
  alSourcef(m_source, AL_SEC_OFFSET, sec);
  OpenALSystem::check_al_error("OpenALSoundSource::seek_to: ");
}

void
OpenALSoundSource::seek_to_sample(int sample)
{
  alSourcei(m_source, AL_SAMPLE_OFFSET, sample);
  OpenALSystem::check_al_error("OpenALSoundSource::seek_to_sample: ");
}

float
OpenALSoundSource::get_pos() const
{
  float sec = 0.0f;
  alGetSourcef(m_source, AL_SEC_OFFSET, &sec);
  OpenALSystem::check_al_error("OpenALSoundSource::get_pos: ");
  return sec;
}

int
OpenALSoundSource::get_sample_pos() const
{
  ALint sample_pos;
  alGetSourcei(m_source, AL_SAMPLE_OFFSET, &sample_pos);
  OpenALSystem::check_al_error("OpenALSoundSource::get_sample_pos: ");
  return sample_pos;
}

bool
OpenALSoundSource::is_playing() const
{
  ALint state = AL_PLAYING;
  alGetSourcei(m_source, AL_SOURCE_STATE, &state);
  return state != AL_STOPPED;
}

void
OpenALSoundSource::set_looping(bool looping)
{
  alSourcei(m_source, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
  OpenALSystem::check_al_error("OpenALSoundSource::set_looping: ");
}

void
OpenALSoundSource::set_relative(bool relative)
{
  alSourcei(m_source, AL_SOURCE_RELATIVE, relative ? AL_TRUE : AL_FALSE);
}

void
OpenALSoundSource::set_position(float x, float y, float z)
{
  alSource3f(m_source, AL_POSITION, x, y, z);
  OpenALSystem::check_al_error("OpenALSoundSource::set_position: ");
}

void
OpenALSoundSource::set_velocity(float x, float y, float z)
{
  alSource3f(m_source, AL_VELOCITY, x, y, z);
  OpenALSystem::check_al_error("OpenALSoundSource::set_velocity: ");
}

void
OpenALSoundSource::set_gain(float gain)
{
  m_gain = gain;
  alSourcef(m_source, AL_GAIN, m_channel.get_gain() * m_gain);
  OpenALSystem::check_al_error("OpenALSoundSource::set_gain: ");
}

float
OpenALSoundSource::get_gain() const
{
  return m_gain;
}

void
OpenALSoundSource::set_pitch(float pitch)
{
  alSourcef(m_source, AL_PITCH, pitch);
}

void
OpenALSoundSource::set_reference_distance(float distance)
{
  alSourcef(m_source, AL_REFERENCE_DISTANCE, distance);
  OpenALSystem::check_al_error("OpenALSoundSource::set_reference_distance: ");
}

void
OpenALSoundSource::set_rolloff_factor(float factor)
{
  alSourcef(m_source, AL_ROLLOFF_FACTOR, factor);
  OpenALSystem::check_al_error("OpenALSoundSource::set_rolloff_factor: ");
}

void
OpenALSoundSource::update_gain() const
{
  alSourcef(m_source, AL_GAIN, m_channel.get_gain() * get_gain());
  OpenALSystem::check_al_error("OpenALSoundSource::update_gain: ");
}

void
OpenALSoundSource::update(float delta)
{
  SoundSource::update(delta);
}

void
OpenALSoundSource::set_direct_filter(FilterPtr const& filter)
{
  m_direct_filter = filter;

  if (!filter) {
    alSourcei(m_source, AL_DIRECT_FILTER, AL_FILTER_NULL);
  } else {
    alSourcei(m_source, AL_DIRECT_FILTER, filter->handle());
  }
}

void
OpenALSoundSource::set_effect_slot(EffectSlotPtr const& slot, FilterPtr const& filter)
{
  m_effect_slot = slot;
  m_filter = filter;

  alSource3i(m_source, AL_AUXILIARY_SEND_FILTER,
             slot->handle(),
             1,
             filter ? m_filter->handle() : AL_FILTER_NULL);
  OpenALSystem::check_al_error("OpenALSoundSource::update_set_effect_slot: ");
}

} // namespace wstsound

/* EOF */
