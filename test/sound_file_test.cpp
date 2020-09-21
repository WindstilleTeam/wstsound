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

#include <gtest/gtest.h>

#include "ogg_sound_file.hpp"
#include "opus_sound_file.hpp"
#include "wav_sound_file.hpp"

using namespace wstsound;

namespace {

size_t get_real_sample_duration(SoundFile& soundfile) {
  size_t total_bytesread = 0;
  while (true) {
    std::array<char, 1024 * 64> buffer;
    size_t const bytesread = soundfile.read(buffer.data(), buffer.size());
    if (bytesread == 0) {
      break;
    }
    total_bytesread += bytesread;
  }
  return total_bytesread / soundfile.get_channels() / (soundfile.get_bits_per_sample() / 8);
}

} // namespace

TEST(SoundFileTest, wav)
{
  auto fin = std::make_unique<std::ifstream>("data/sound.wav");
  WavSoundFile sound_file(std::move(fin));

  EXPECT_EQ(sound_file.get_bits_per_sample(), 16);
  EXPECT_EQ(sound_file.get_size(), 22788);
  EXPECT_EQ(sound_file.get_rate(), 44100);
  EXPECT_EQ(sound_file.get_channels(), 1);
  EXPECT_FLOAT_EQ(sound_file.get_duration(), 0.25836736f);
  EXPECT_EQ(sound_file.get_sample_duration(), get_real_sample_duration(sound_file));
}

TEST(SoundFileTest, ogg)
{
  auto fin = std::make_unique<std::ifstream>("data/sound.ogg");
  OggSoundFile sound_file(std::move(fin));

  EXPECT_EQ(sound_file.get_bits_per_sample(), 16);
  EXPECT_EQ(sound_file.get_size(), 22788);
  EXPECT_EQ(sound_file.get_rate(), 44100);
  EXPECT_EQ(sound_file.get_channels(), 1);
  EXPECT_FLOAT_EQ(sound_file.get_duration(), 0.25836736f);
  EXPECT_EQ(sound_file.get_sample_duration(), get_real_sample_duration(sound_file));
}

TEST(SoundFileTest, opus)
{
  auto fin = std::make_unique<std::ifstream>("data/sound.opus");
  OpusSoundFile sound_file(std::move(fin));

  EXPECT_EQ(sound_file.get_bits_per_sample(), 16);
  EXPECT_EQ(sound_file.get_size(), 24804);
  EXPECT_EQ(sound_file.get_rate(), 48000);
  EXPECT_EQ(sound_file.get_channels(), 1);
  EXPECT_FLOAT_EQ(sound_file.get_duration(), 0.25837499f);
  EXPECT_EQ(sound_file.get_sample_duration(), get_real_sample_duration(sound_file));
}

/* EOF */
