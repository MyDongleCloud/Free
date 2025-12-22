#!/bin/sh

cd /usr/local/modules/typesensedashboard
npm install
./node_modules/.bin/quasar prepare
cat > /tmp/typesensedashboard.patch << EOF
diff --git a/src/pages/Login.vue b/src/pages/Login.vue
index daeda97..7473b80 100644
--- a/src/pages/Login.vue
+++ b/src/pages/Login.vue
@@ -69,6 +69,12 @@ import { onMounted, ref } from 'vue';
 const store = useNodeStore();
 
 onMounted(() => {
+if (store.loginData?.node && store.loginData.node.host == 'AUTO') {
+	store.loginData.node.host = window.location.hostname;
+	store.loginData.node.port = Number(window.location.port || (window.location.protocol === 'https:' ? 443 : 80));
+	store.loginData.node.protocol = window.location.protocol.slice(0, -1);
+	store.loginData.node.tls = window.location.protocol === 'https:';
+}
   store.connectionCheck();
 });
EOF
patch -p1 < /tmp/typesensedashboard.patch
PUBLIC_PATH=./ ./node_modules/.bin/quasar build
ln -sf /disk/admin/modules/typesensedashboard/config.json /usr/local/modules/typesensedashboard/dist/spa/
