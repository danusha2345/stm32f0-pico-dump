#!/usr/bin/env bash
# Auto-attach first available /dev/ttyACM* at 115200 and log to stdout.
# Restart automatically if the device disappears/reappears.

set -euo pipefail

SPEED=115200

while :; do
  port=$(ls /dev/ttyACM* 2>/dev/null | head -n 1 || true)
  if [ -n "${port:-}" ]; then
    echo ">> Opening ${port} at ${SPEED} baud"
    stty -F "${port}" ${SPEED} raw -echo -ixon 2>/dev/null || true
    # stdbuf keeps line buffering so we see output immediately
    stdbuf -oL -eL cat "${port}" || true
  else
    echo ">> No /dev/ttyACM* found, retrying..."
  fi
  sleep 1
done
