#!/bin/bash
# noggit3 uses settings.ini next to the binary for configuration.
# On first run, open Settings and set your WoW client path and project path.
mkdir -p "$HOME/.config/noggit3"

docker run -it --rm \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -e XDG_RUNTIME_DIR=/tmp \
  --device /dev/dri \
  --ipc=host \
  -v /home/$USER/Games/acwow/ChromieCraft_3.3.5a:/wow \
  -v /home/$USER/noggit-projects:/projects \
  -v /home/$USER/.config/noggit3:/root/.config/noggit3 \
  noggit3 \
  /app/build/bin/noggit
