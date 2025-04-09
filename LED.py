'''
The code converts each MP3 file into an array of “LED ON/OFF” (1–13) 
for each small time window—with extra emphasis on the frames that correspond to a detected beat.
Outputs array to copy/paste into machine/src/main.cpp
'''

import numpy as np
from pydub import AudioSegment
import math
import librosa

song_length = 13
LED_update_freq_ms = 30

# Paths to mp3 files to process
paths = ["/Users/oskareidem/Documents/matautomatinator/df_player_mini/0001.mp3",
        "/Users/oskareidem/Documents/matautomatinator/df_player_mini/0002.mp3",
        "/Users/oskareidem/Documents/matautomatinator/df_player_mini/0003.mp3",
        "/Users/oskareidem/Documents/matautomatinator/df_player_mini/0004.mp3",
        "/Users/oskareidem/Documents/matautomatinator/df_player_mini/0005.mp3",
        "/Users/oskareidem/Documents/matautomatinator/df_player_mini/0006.mp3",
        "/Users/oskareidem/Documents/matautomatinator/df_player_mini/0007.mp3",
        "/Users/oskareidem/Documents/matautomatinator/df_player_mini/0008.mp3",
        "/Users/oskareidem/Documents/matautomatinator/df_player_mini/0009.mp3",
        "/Users/oskareidem/Documents/matautomatinator/df_player_mini/0010.mp3",
        "/Users/oskareidem/Documents/matautomatinator/df_player_mini/0011.mp3"]

def get_scaled_array(data):
    min_val = min(data)
    max_val = max(data)
    mapped_data = np.ceil(np.interp(data, (min_val, max_val), (1, 13)))
    return [int(x) for x in mapped_data]

def compute_window_size(song_length_s, num_samples, update_interval_ms):
    num_frames = (song_length_s * 1000.0) / update_interval_ms
    num_frames_int = max(1, int(math.floor(num_frames)))
    window_size = max(1, int(math.floor(num_samples / num_frames_int)))
    return window_size

def detect_beats(audio_samples, sr, update_interval_ms):
    onset_env = librosa.onset.onset_strength(y=audio_samples, sr=sr)
    
    tempo, beats = librosa.beat.beat_track(
        onset_envelope=onset_env,
        sr=sr,
        units='time'
    )
    
    beat_frames = [int(beat_time * 1000 / update_interval_ms) for beat_time in beats]
    return beat_frames

def get_down_sampled_amplitude_data_from_mp3_files(paths, song_length_s, update_interval_ms):
    amplitude_envelope_arr = []
    
    for path in paths:
        audio = AudioSegment.from_mp3(path)
        samples = np.array(audio.get_array_of_samples(), dtype=np.float32)
        
        # Normalize samples
        if audio.sample_width == 2:
            samples /= 32768.0
        elif audio.sample_width == 1:
            samples = (samples - 128.0) / 128.0

        beat_frames = detect_beats(samples, audio.frame_rate, update_interval_ms)

        # Calculate amplitude envelope
        window_size = compute_window_size(song_length_s, len(samples), update_interval_ms)
        amplitude_envelope = [
            np.sqrt(np.mean(frame**2))
            for frame in (samples[i:i+window_size] for i in range(0, len(samples), window_size))
            if len(frame) > 0
        ]

        # Scale amplitude to match number of LED's on strip
        scaled = get_scaled_array(amplitude_envelope)

        # Boost beat frames 
        for frame in beat_frames:
            if frame < len(scaled):
                scaled[frame] = min(13, scaled[frame] + 4)  # Strong beat boost

        amplitude_envelope_arr.append(scaled)
    
    return amplitude_envelope_arr



result = get_down_sampled_amplitude_data_from_mp3_files(paths, song_length, LED_update_freq_ms)

print("const uint8_t tracks[lastTrackNumber][num_updates] PROGMEM = {")
for i in range(0, len(result)):
    formatted_result = ", ".join(str(val) for val in result[i])
    print("{", formatted_result, "},")
print("};")