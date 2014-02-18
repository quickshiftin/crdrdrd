crdrdrd
=======

CaRD ReaDeR Daemon - C daemon to process swipes for MiniMag USB Card reader et al using libusb

About
-----
This program processes card swipes from [readers like these](http://www.ebay.com/bhp/usb-magnetic-card-reader). It runs in userspace and is shipped as a daemon. The daemwon is single-threaded.

Once a card is swiped it signals an external program with the number of the card that was just swiped. It's also designed to insert the card number into a SQLite database first, you'll almost certainly want to remove that feature. There's also some home-grown logging code that works over syslog.

This program was written years ago and used for a production application. These days it may best serve as example code. There is other code that you may not care for, again feel free to chop away and use what you wish.

History
-------
I want to thank my buddy David McBride for his help making this program usable. I did most of the original reasearch and coding. Originally I tried two Java libraries that were both insufficient. I thought I might need a kernel module, but then on some IRC channel somebody mentioned [libusb](http://www.libusb.org/) to me.

I read through some of [Advanced Programming in the Unix Environment](http://www.amazon.com/Programming-Environment-Addison-Wesley-Professional-Computing/dp/0321525949), enough to cobble together a little daemon. The code would run and crash after about 40 swipes.

My good buddy David McBride took it from there. Fixing numerous pointer problems and the like. The code runs like a champ now or did years ago, when it was fresh.
