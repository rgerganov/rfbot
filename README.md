What's this?
===
Slack bot for controlling nearby RF devices. For now it can only ring a wireless doorbell.

But why?
===
Because ringing a doorbell from Slack is a huge productivity boost for everyone in the office.

Awesome! What do I need?
===
* Silvercrest wireless doorbell that works on 433MHz. I got mine from LIDL. ($7)
* HackRF ($300)
* Some sort of computer that can run Linux and Python3 ($20 - $20000)

How to run this crap?
===
First you need to sniff the code which triggers the bell. The modulation is ASK, on-off keying.
You can use [rtl_433](https://github.com/merbanan/rtl_433), [inspectrum](https://github.com/miek/inspectrum) or whatever.
Install `libhackrf-dev`, the Python dependencies and then finally build with `make`:

```shell
$ sudo apt install libhackrf-dev
$ pip install -r requirements.txt
$ make
```

set the ring code and the Slack token as env. variables and you are ready to go:

```shell
$ export RING_CODE=<your_ring_code_written_with_1s_and_0s>
$ export SLACK_API_TOKEN=<your_slack_token>
$ ./rfbot.py
```

What's next?
===
Who knows... There are tons of crappy devices on 433MHz around us, waiting to be exploited. Patches are always welcome! :)
