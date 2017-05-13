<a href="http://www.youtube.com/watch?feature=player_embedded&v=-xVune0NEsA" target="_blank"><img src="http://img.youtube.com/vi/-xVune0NEsA/0.jpg" alt="Sound Shovel Demo Video" width="333" height="200" border="1" /></a>

# Sound Shovel
A simple, fast, light-weight sound editor.

...because I find Audacity's UI clunky and not optimized for simple wave file editing tasks.

The project was inspired by Wavosaur (http://www.wavosaur.com), which seems much more usable than Audacity. Unfortunately in 2015-2016 it had some serious use-case breaking bugs and didn't seem to be being maintained. And it isn't open source. (Since I started writing Sound Shovel, the Wavosaur developers fixed the bugs).

At the start of the project I wondered how to efficiently display a huge sound file on screen. I couldn't find a good answer on Google. With a little help from the good people of Stack Overflow (see http://stackoverflow.com/questions/37554058/fast-display-of-waveform-in-c-c) I created an algorithm that might be new and seems to enable better performance and render quality than anything else I've seen.

USAGE:

* Run the exe!
* Open a 16-bit stereo WAV file in the file dialog.
* Zoom with mouse wheel or cursor up/down.
* Scroll with middle mouse drag or cursor/left right.
* Page Up/Down and Home and End do stuff too.
* Copy, Paste and Save might work too, if you are lucky.
