#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for test [-a -c -d -h -l -m -r -s -t -u -v -z]"
echo "a:	Sign-in, login, token, revoke, sign-out"
echo "c:	Create with email, password and name"
echo "d:	Update username"
echo "h:	Print this usage and exit"
echo "l:	Login with cookie"
echo "m:	Magic link"
echo "r:	Revoke with cookie"
echo "s:	Sign-in with email"
echo "t:	Get token with cookie"
echo "u:	Sign-out"
echo "v:	Sign-in with username"
echo "z:	Delete"
exit 0
}

CREATE=0
UPDATE=0
LOGIN=0
MAGIC=0
REVOKE=0
TOKEN=0
SIGNIN=0
SIGNOUT=0
DELETE=0
while getopts acdhlmrstuvz opt; do
	case "$opt" in
		a) SIGNIN=1;LOGIN=1;TOKEN=1;REVOKE=1;SIGNOUT=1;;
		c) CREATE=1;;
		d) UPDATE=1;;
		h) helper;;
		l) LOGIN=1;;
		m) MAGIC=1;;
		r) REVOKE=1;;
		s) SIGNIN=1;;
		t) TOKEN=1;;
		u) SIGNOUT=1;;
		v) SIGNIN=2;;
		z) DELETE=1;;
	esac
done

if [ $CREATE = 1 ]; then
	echo "############### Create"
	RET_CREATE=`curl -sS --fail -X POST http://localhost:8091/MyDongleCloud/Auth/sign-up/email -H "Content-Type: application/json" -d '{"email":"john.doe@example.com", "name":"John Doe", "password":"demodemo"}' -c /tmp/cookie.txt`
	echo $RET_CREATE | jq
fi
if [ $UPDATE = 1 ]; then
	echo "############### Update"
	RET_UPDATE=`curl -sS --fail -b /tmp/cookie.txt -X POST http://localhost:8091/MyDongleCloud/Auth/update-user -H "Content-Type: application/json" -d '{"username":"admin"}' -c /tmp/cookie.txt`
	echo $RET_UPDATE | jq
fi
if [ $SIGNIN = 1 ]; then
	RET_SIGNIN=`curl -sS --fail -X POST http://localhost:8091/MyDongleCloud/Auth/sign-in/email -H "Content-Type: application/json" -d '{"email":"john.doe@example.com", "password":"demodemo"}' -c /tmp/cookie.txt`
	echo $RET_SIGNIN | jq
fi
if [ $SIGNIN = 2 ]; then
	RET_SIGNIN=`curl -sS --fail -X POST http://localhost:8091/MyDongleCloud/Auth/sign-in/username -H "Content-Type: application/json" -d '{"username":"admin", "password":"demodemo"}' -c /tmp/cookie.txt`
	echo $RET_SIGNIN | jq
fi
if [ $LOGIN = 1 ]; then
	echo "############### Login"
	RET_LOGIN=`curl -sS --fail -b /tmp/cookie.txt http://localhost:8091/MyDongleCloud/Auth/get -sS --failession -H "Content-Type: application/json"`
	echo $RET_LOGIN | jq
fi
if [ $MAGIC = 1 ]; then
	echo "############### Magic"
	RET_MAGIC=`curl -sS --fail -X POST http://localhost:8091/MyDongleCloud/Auth/sign-in/magic-link -H "Content-Type: application/json" -d '{"email":"john.doe@example.com", "callbackURL":"http://localhost:8100" }'`
	echo $RET_MAGIC | jq
fi
if [ $TOKEN = 1 ]; then
	echo "############### Token"
	RET_JWKS=`curl -sS --fail http://localhost:8091/MyDongleCloud/Auth/jwks -H "Content-Type: application/json"`
	#echo $RET_JWKS
	RET_TOKEN=`curl -sS --fail -b /tmp/cookie.txt http://localhost:8091/MyDongleCloud/Auth/token -H "Content-Type: application/json"`
	echo "$RET_TOKEN" | jq -R 'split(".") | .[1] | @base64d | fromjson'
fi
if [ $REVOKE = 1 ]; then
	echo "############### Revoke"
	TOKEN=`echo $RET_LOGIN | jq -r '.session.token'`
	RET_REVOKE=`curl -sS --fail -b /tmp/cookie.txt -X POST http://localhost:8091/MyDongleCloud/Auth/revoke -sS --failession -H "Content-Type: application/json" -d "{ \"token\":\"${TOKEN}\"}"`
	echo $RET_REVOKE | jq
fi
if [ $SIGNOUT = 1 ]; then
	echo "############### Sign-out"
	RET_SIGNOUT=`curl -sS --fail -b /tmp/cookie.txt http://localhost:8091/MyDongleCloud/Auth/sign-out -H "Content-Type: application/json"`
	echo $RET_SIGNOUT | jq
fi
if [ $DELETE = 1 ]; then
	echo "############### Delete"
	RET_DELETE=`curl -sS --fail -b /tmp/cookie.txt -X POST http://localhost:8091/MyDongleCloud/Auth/delete-user -H "Content-Type: application/json" -d '{"password":"demodemo"}' -c /tmp/cookie.txt`
	echo $RET_DELETE | jq
fi
