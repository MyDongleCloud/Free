#!/bin/sh

curl -L -sS --fail https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp_linux_aarch64 -o /usr/local/bin/yt-dlp
chmod a+x /usr/local/bin/yt-dlp
chown admin:admin /usr/local/bin/yt-dlp
ln -sf /usr/local/bin/yt-dlp /usr/local/bin/youtube-dl
