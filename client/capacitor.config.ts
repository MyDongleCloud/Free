import { CapacitorElectronConfig } from "@capacitor-community/electron";
import type { CapacitorConfig } from '@capacitor/cli';

const config: CapacitorElectronConfig = {
  appId: 'cloud.mydongle.app',
  appName: 'MyDongle Cloud',
  webDir: 'app',
  bundledWebRuntime: false,
  //loggingBehavior: none,
  electron: {
    trayIconAndMenuEnabled: true,
  }

const config: CapacitorConfig = {
  appId: 'cloud.mydongle.app',
  appName: 'MyDongle Cloud',
  webDir: 'app'
};

export default config;
