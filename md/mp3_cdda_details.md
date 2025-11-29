MP3/CDDA Details (NCDZ)
========================

File Layout and Naming
----------------------
- MP3 directory is set by the browser when selecting a CD game: `mp3_dir = <game_dir>/mp3` (see `src/psp/filer.c` around selection logic).
- Tracks are searched with pattern `%02d.mp3` where `track` is the requested CDDA track number (e.g., `01.mp3`, `02.mp3`).
- `find_file(pattern, mp3_dir)` locates the matching file; if not found, CDDA command is ACKed without playback.

Playback Path
-------------
- `cdda_play(track)` in `src/ncdz/cdda.c`:
  - Sets state, then if `option_mp3_enable`, builds pattern and tries to play the MP3 via `mp3_play(path)`.
  - Resets autoframeskip to re-sync pacing when playback starts.
- `cdda_stop()` stops playback and clears track state; `cdda_pause()` pauses and calls `mp3_pause`.
- `neogeo_cdda_check()` runs each frame to maintain playback state and handle looping.
- CD seek commands are acknowledged even if MP3 is missing; state machine avoids hard failures to mirror CDDA behaviour.

MP3 Decoder (src/psp/mp3.c)
---------------------------
- Uses libmad to decode; buffers: `MP3_BUFFER_SIZE = 736*2*4` bytes per ping-pong buffer.
- Thread-driven: `mp3_thread_start()` spawns decoder thread; `mp3_thread_stop()` halts it.
- Options:
  - `option_mp3_enable`: master switch.
  - `option_mp3_volume`: mapped to internal volume for output.
- Sleep handling: If PSP sleeps, MP3 thread closes file, waits, and reopens at prior offset; if reopen fails, shows popup `COULD_NOT_REOPEN_MP3_FILE`.
- Looping: If `cdda_autoloop` is set and file reaches end, seeks to start; otherwise stops and sets `cdda_playing = CDDA_STOP`.

Formats and Expectations
------------------------
- MP3s should be standard CBR/VBR compatible with libmad; no specific bitrate enforced in code.
- File extension `.mp3` required; exact two-digit track numbers used for lookup.
- Ensure MP3 files are placed in `mp3` subfolder alongside the game’s CD image to be found.

Integration Notes
-----------------
- CD image loading (`cdrom_process_ipl`) is separate; MP3s only handle CDDA.
- Snapshots and pauses mute MP3s: `save_snapshot` calls `mp3_pause(1)` when `EMU_SYSTEM == NCDZ`.
- On resume from sleep, MP3 playback resumes at stored `mp3_filepos`.
