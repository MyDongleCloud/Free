#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include "macro.h"

int bluetoothAddr(char *addr, int downup) {
	for(int i = 0; i < 10; i++) {
		/*if (hci_get_route(NULL) == -1) {
			PRINTF("BluetoothAddr: Can't get HCI address (retry #%d)\n", i);
			usleep(1000 * 1000);
			continue;
		}*/
		if (i > 10)
			return -1;
		int dd = socket(31, SOCK_RAW | SOCK_CLOEXEC | SOCK_NONBLOCK, BTPROTO_HCI);
		if (dd <= 0) {
			PRINTF("BluetoothAddr: Error opening socket HCI\n");
			return -1;
		}
		struct hci_dev_info di = {dev_id: 0};
		if (ioctl(dd, HCIGETDEVINFO, (void*) &di) != 0) {
			PRINTF("BluetoothAddr: Error socket HCIGETDEVINFO (retry #%d)\n", i);
			close(dd);
			usleep(1000 * 1000);
			continue;
		}
		sprintf(addr, "%02X:%02X:%02X:%02X:%02X:%02X", di.bdaddr.b[5], di.bdaddr.b[4], di.bdaddr.b[3], di.bdaddr.b[2], di.bdaddr.b[1], di.bdaddr.b[0]);
		PRINTF("BluetoothAddr: %s\n", addr);
		if (di.bdaddr.b[5] == 0 && di.bdaddr.b[4] == 0 && di.bdaddr.b[3] == 0 && di.bdaddr.b[2] == 0 && di.bdaddr.b[1] == 0 && di.bdaddr.b[0] == 0) {
			PRINTF("BluetoothAddr: Error address zero (retry #%d)\n", i);
			close(dd);
			usleep(1000 * 1000);
			continue;
		}
		if (i > 0) {
			PRINTF("BluetoothAddr: Wait because retried\n");
			usleep(3 * 1000 * 1000);
		}
		if (downup != -1) {
			int ret = ioctl(dd, downup ? HCIDEVUP : HCIDEVDOWN, 0);
			PRINTF("BluetoothAddr: %s (ret:%d)\n", downup ? "HCIDEVUP" : "HCIDEVDOWN", ret);
		}
		close(dd);
		break;
	}
	return 0;
}
