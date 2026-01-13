#!/bin/sh

apt-get -y install libonig-dev
cd /usr/local/modules/tubesync
/home/ai/rootfs/usr/local/modules/mydonglecloud/pip.sh -f /usr/local/modules/tubesync/env -s
echo "PATH before any modif: $PATH"
PATHOLD=$PATH
PATH=/usr/local/modules/tubesync/env/bin:$PATHOLD
export PATH=/usr/local/modules/tubesync/env/bin:$PATHOLD
echo "PATH new: $PATH python: `python --version`"
pip install django django-huey django-sass-processor pillow whitenoise gunicorn httptools django-basicauth psycopg PySocks urllib3 requests yt-dlp emoji brotli html5lib bgutil-ytdlp-pot-provider babi curl-cffi libsass django-compressor
cd tubesync
sed -i -e 's|/config/tasks/|/disk/admin/modules/tubesync/tasks/|' common/huey.py
sed -i -e 's|import yt_dlp.patch|#import yt_dlp.patch|' sync/youtube.py
sed -i -e 's@/app/full_playlist\.sh@./full_playlist.sh@' sync/youtube.py
sed -i -e 's@/app/manage\.py@./manage.py@' full_playlist.sh
cp tubesync/local_settings.py.example tubesync/local_settings.py
cat >> tubesync/local_settings.py <<EOF
DOWNLOAD_ROOT = Path('/disk/admin/modules/tubesync/downloads')
ALLOWED_HOSTS = ['*']
#CSRF_TRUSTED_ORIGINS = ['*']
EOF
./manage.py migrate
cat >> tubesync/local_settings.py <<EOF
DATABASES = {
    'default': {
        'ENGINE': 'django.db.backends.sqlite3',
        'NAME': Path('/disk/admin/modules/tubesync/db.sqlite3'),
    }
}
DATABASE_CONNECTION_STR = f'sqlite at "{DATABASES["default"]["NAME"]}"'
YOUTUBE_DL_CACHEDIR = '/dev/shm'
EOF
./manage.py compilescss
./manage.py collectstatic
chown -R admin:admin /disk/admin/modules/tubesync
PATH=$PATHOLD
export PATH=$PATHOLD
echo "PATH restored: $PATH"
