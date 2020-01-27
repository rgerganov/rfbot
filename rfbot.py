#!/usr/bin/env python3
import os
import re
import sys
import subprocess
from slack import RTMClient

MENTION_REGEX = "^<@(|[WU].+?)>(.*)"

bot_user_id = None

@RTMClient.run_on(event="message")
def ring(**payload):
    global bot_user_id
    data = payload['data']
    web_client = payload['web_client']
    if bot_user_id is None:
        bot_user_id = web_client.auth_test()['user_id']
        print("Bot user id: " + bot_user_id)
    if 'text' not in data:
        return
    msg = data['text']
    matches = re.search(MENTION_REGEX, msg)
    if not matches:
        return
    user_id, cmd = matches.group(1), matches.group(2).strip()
    if user_id != bot_user_id:
        return
    resp = "Not sure what you mean. Try *ring*."
    if cmd.lower() == "ring":
        ring_code = os.environ["RING_CODE"]
        ret = subprocess.call("./ring -c {} -x 30".format(ring_code).split())
        if ret == 0:
            resp = "Ding dong!"
        else:
            resp = "Oops, something went wrong :("
    channel_id = data['channel']
    thread_ts = data['ts']
    user = data['user']
    web_client.chat_postMessage(
        channel=channel_id,
        text=resp,
        #thread_ts=thread_ts
    )

if __name__ == '__main__':
    if "RING_CODE" not in os.environ:
        print("RING_CODE is not set")
        sys.exit(1)
    slack_token = os.environ["SLACK_API_TOKEN"]
    rtm_client = RTMClient(token=slack_token)
    rtm_client.start()
