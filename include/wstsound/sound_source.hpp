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

#ifndef HEADER_WINDSTILLE_SOUND_SOUND_SOURCE_HPP
#define HEADER_WINDSTILLE_SOUND_SOUND_SOURCE_HPP

#include "fwd.hpp"

namespace wstsound {

enum class FadeState
{
  NoFading,
  FadingOn,
  FadingOff
};

class SoundSource
{
public:
  SoundSource();
  virtual ~SoundSource() {}

  virtual void play() = 0;
  virtual void stop() = 0;
  virtual bool is_playing() const = 0;

  virtual float get_duration() const = 0;
  virtual int get_sample_duration() const = 0;

  virtual void set_looping(bool looping) = 0;

  /// Set volume (0.0 is silent, 1.0 is normal)
  virtual void  set_gain(float gain) = 0;
  virtual float get_gain() const = 0;

  virtual void  seek_to_sample(int sample) = 0;
  virtual void  seek_to(float sec) = 0;

  /** Return the current position in seconds */
  virtual float get_pos() const = 0;

  /** Return the current position in pcm samples */
  virtual int get_sample_pos() const = 0;

  virtual void set_position(float x, float y, float z) = 0;
  virtual void set_velocity(float x, float y, float z) = 0;

  /** Distances closer then reference distance will increase the gain
      (or keep it at 1.0f), while distances further away will lower it */
  virtual void set_reference_distance(float distance) = 0;

  /** Higher factor will mean the sound gain lowers quicker with
      distance, while lower factor will reduce the lowering of gain,
      0.0f disables distance based reduction completly */
  virtual void set_rolloff_factor(float factor) = 0;

  virtual void set_direct_filter(FilterPtr const& filter) = 0;
  virtual void set_effect_slot(EffectSlotPtr const& slot, FilterPtr const& filter = {}) = 0;

  /** Needs to be called whenever the SoundChannels gain changes */
  virtual void update_gain() const = 0;

  virtual void update(float delta);

  void set_fading(FadeState fade_state, float fade_time);
  FadeState get_fade_state() const { return m_fade_state; }

private:
  FadeState m_fade_state;
  float m_fade_start_ticks;
  float m_fade_time;

  // FIXME: simple time counter that summarizes all deltas, could be done better
  float m_total_time;

private:
  SoundSource(const SoundSource&);
  SoundSource& operator=(const SoundSource&);
};

} // namespace wstsound

#endif

/* EOF */
