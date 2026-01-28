#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for pam [-h]"
echo "h:	Print this usage and exit"
exit 0
}

while getopts h opt
do
	case "$opt" in
		h) helper;;
	esac
done

json_payload=$(printf '%s\n' "$PAM_USER" "$PAM_SERVICE" "$PAM_TYPE" "$1" | jq -Rn '{ a:"pam", u:input, s:input, t:input, o:input }')
echo $json_payload | websocat -1 ws://localhost:8094
exit 0
