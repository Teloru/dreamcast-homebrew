"""Check the original pack assets and PCM compatibility, without an audio device."""

from pathlib import Path
import hashlib
import struct
import unittest
import wave


ROOT = Path(__file__).resolve().parents[1] / "assets" / "sfx"
CLIPS = {
    "question.wav": (8820, "895bf3acff2227da64f41f0b4e90b7a68341d8c094aa5ee22a31f90f713654fe"),
    "star.wav": (26460, "18e4cf581ad42cc6644841ac3c3b8af1ddcbe25a7369b2fb1c6acf0669bc513f"),
    "slider-tick-1.wav": (1984, "dece2005a9de892dcf34e7927d6290dcfc7a2e8e002c00b30cba8cfd1275eb23"),
    "back-1.wav": (15876, "0260fba96687086738ae1370d842a57652b74761f91286d549b2cd2309723f49"),
}


class SoundEffectAssets(unittest.TestCase):
    def test_only_selected_original_clips_are_bundled(self):
        self.assertEqual({p.name for p in ROOT.glob("*.wav")}, set(CLIPS))
        for filename, (_, digest) in CLIPS.items():
            with self.subTest(clip=filename):
                self.assertEqual(hashlib.sha256((ROOT / filename).read_bytes()).hexdigest(), digest)

    def test_pcm_format_length_and_endpoints(self):
        for filename, (frame_count, _) in CLIPS.items():
            with self.subTest(clip=filename), wave.open(str(ROOT / filename), "rb") as clip:
                self.assertEqual(clip.getcomptype(), "NONE")
                self.assertEqual(clip.getnchannels(), 1)
                self.assertEqual(clip.getsampwidth(), 2)
                self.assertEqual(clip.getframerate(), 44100)
                self.assertEqual(clip.getnframes(), frame_count)
                pcm = clip.readframes(frame_count)
                self.assertEqual(len(pcm), frame_count * 2)
                samples = struct.unpack(f"<{frame_count}h", pcm)
                self.assertGreater(max(abs(value) for value in samples), 0)
                self.assertLess(max(abs(value) for value in samples), 32767)
                self.assertEqual((samples[0], samples[-1]), (0, 0))
                self.assertEqual(clip.readframes(1), b"")

if __name__ == "__main__":
    unittest.main()
