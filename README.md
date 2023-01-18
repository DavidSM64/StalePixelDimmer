# Stale Pixel Dimmer (Proof of concept)

This is a simple tool that can dim pixels that haven't changed on the screen. The idea is to reduce wear on OLED screens by dramatically dimming parts of the screen that don't change. This tool is *incredibly* basic. This is my first project that uses the Windows API / GDI+. Normally I try to write cross-platform apps, but since I'm just making this for myself I don't really need to care about other platforms.

I'm putting this code out in the public domain (CC0), since I want someone who is smarter than me to come up with a better version. I don't really care who makes it, or if they even try to sell it. You don't need to credit me, although that would be a nice gesture. I just want something that will extend the life of my new laptop lol.

## Issues

* It takes ~120 ms to update the screen on my machine. Ideally, this would be as low as possible. But it seems that GDI+ is the bottleneck. Specifically `StretchBlt` and `graphics.DrawImage` are the main culprits. I heard switching to DirectX can improve performance, but I have no idea how that library works.
* Lots of ghosting! Probably due to the low update rate.
* Mouse seems to fliker a bit?
* Doesn't seem to affect videos at all, but that might not be a bad thing.
* Some windows can flicker a bit. Might be some kind of Z-Fighting with the drawing routine?

## Video demonstration:

https://www.youtube.com/watch?v=Pt7J3dQEMSI

