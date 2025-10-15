#!/bin/sh

json_payload=$(printf '%s\n' "$PAM_USER" "$PAM_SERVICE" "$PAM_TYPE" "$1" | jq -Rn '{ a: "pam", u:input, s:input, t:input, o:input }')
echo $json_payload | nc -w 1 127.0.0.1 8093
exit 0
