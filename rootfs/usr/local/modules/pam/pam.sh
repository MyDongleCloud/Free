#!/bin/sh

json_payload=$(printf '%s\n' "$PAM_USER" "$PAM_SERVICE" "$PAM_TYPE" | jq -Rn '{ a: "pam", u:input, s:input, t:input }')
printf '%s\n' "$json_payload" | nc -w 1 127.0.0.1 8093
exit 0
