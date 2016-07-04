# sound_shovel
A simple, fast, light-weight sound editor.

...because I find Audacity's UI clunky and not optimized for simple wave file editing tasks.

The project was inspired by Wavosaur (http://www.wavosaur.com), which showed how much more usable a sound editor could be. Unfortunately it has some serious use-case breaking bugs and doesn't seem to be being maintained. And it isn't open source.

At the start of the project I wondered how to efficiently display a huge sound file on screen. I couldn't find a good answer on Google. With a little help from the good people of Stack Overflow (see http://stackoverflow.com/questions/37554058/fast-display-of-waveform-in-c-c) I created an algorithm that might be new and seems to enable better performance and render quality than anything else I've seen.
