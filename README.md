flicker
=======

Simple flicker effect for Teensy/Arduino.  Sets pins high or low semi-randomly to imitate cheesy electrical shorting effects as seen in bad horror movies.

Pins are set on and off in grouped clusters, with larger gaps of off in between.  Number of on periods per group, and length of on periods grows randomly with time, and gaps shrink randomly with time, all with bounds.  Pins are indepdendant.

Two types of pins, one with longer minimums and one on period per group; these were used to control Jacob's ladders and a fog machine, while the other pins controlled electrical arc effects.
