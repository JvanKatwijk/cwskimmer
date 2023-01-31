

-----------------------------------------------------------------------------
experimental cw skimmer
-----------------------------------------------------------------------------

I was completely fed up with tuning to cw transmissions, too late too
large offset.
So, I figured, why not compute a spectrum a few times a second
and try to decode what is coming in.


So, the resulting cwskimmer computes 500 times a second a spectrum
and from that spectrum 32 "bin"s are chosen, the bits around the
central frequency, under the suumption that that is wjere I want
do decode cw messages - if any.

The binwidth. i.e. the frequency distance between successive bins, is
192000 / 2048, i.e. slightly less than 100 Hz. so, with 32 bins
that are further being processed gives an span of just over 3 KHz.

The approach per bin is straightforward, it is known that  "sample"s
arrive with a frequency of 500 samples/second.

For a given bin, the samples are translated into (potential) space
and (potential) data samples. For each mode (i.e. space of data), the duration
is determined and stored in a vector.

It is a given fact that the longest CW word takes less than 14 space/data
combinations, so we collect up to 7 pairs (data, space), then loo for
the (by far) largest space and assume that the (data,space) combination
in front represent some CW word. Of course, sizes are compared to figure
out what dot and what dashes there are.

The result is compared to a vector with the known "dot/dash" combinations
for the cw letters/

---------------------------------------------------------------------------
The widget
---------------------------------------------------------------------------

The widget shows on the top scope the spectrum of an area of 192K,
the second scope the spectrum of 48 bins, each with a width of
slightly less than 100 Hz, so just over 4 K, centered around
the tuned frequency.

![cw skimmer]/cw-skimmer.png?raw=true)

Decoding is over at most 48 consecutive bins. Experience shows
that watching 48 channels at the same time might give a slight headache,
that is why there are controls to select just a segment of these 48 bins.
Btw, the binwidth is less than 100 Hz, so in many cases the same signal
is detected in a few consecutive bins.

The top line of the widget shows a few controls, such as
selecting a frequency. The three spinboxes are
 - the step size for frequency changes with the mousewheel
 - the center bin of the frequencies that are being decoded;
 - the width, i.e. the number of bins being decoded.


-------------------------------------------------------------------------------
Copyright
----------------------------------------------------------------------------

Copyright

        Copyright (C)   2022
        Jan van Katwijk (J.vanKatwijk@gmail.com)
        Lazy Chair Computing

        The skimmer software is made available under the GPL-2.0. and
        is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

