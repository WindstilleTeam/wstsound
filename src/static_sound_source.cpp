/*
**  Windstille - A Sci-Fi Action-Adventure Game
**  Copyright (C) 2010 Ingo Ruhnke <grumbel@gmail.com>
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

#include "static_sound_source.hpp"

#include "sound_manager.hpp"

namespace {

int buffer_get_sample_duration(ALuint buffer)
{
  ALint frequency;
  alGetBufferi(buffer, AL_FREQUENCY, &frequency);

  ALint bits;
  alGetBufferi(buffer, AL_BITS, &bits);

  ALint channels;
  alGetBufferi(buffer, AL_CHANNELS, &channels);

  ALint size;
  alGetBufferi(buffer, AL_SIZE, &size);

  return 8 * size / channels / bits;
}

float buffer_get_duration(ALuint buffer)
{
  ALint frequency;
  alGetBufferi(buffer, AL_FREQUENCY, &frequency);

  ALint bits;
  alGetBufferi(buffer, AL_BITS, &bits);

  ALint channels;
  alGetBufferi(buffer, AL_CHANNELS, &channels);

  ALint size;
  alGetBufferi(buffer, AL_SIZE, &size);

  return static_cast<float>(size)
    / static_cast<float>(frequency)
    / static_cast<float>(channels)
    / static_cast<float>(bits) * 8.0f;
}

} // namespace

StaticSoundSource::StaticSoundSource(SoundChannel& channel, ALuint buffer) :
  OpenALSoundSource(channel),
  m_duration(buffer_get_duration(buffer)),
  m_sample_duration(buffer_get_sample_duration(buffer))
{
  alSourcei(m_source, AL_BUFFER, buffer);
  OpenALSystem::check_al_error("StaticSoundSource: ");
}

/* EOF */
