import { CapacitorElectronConfig } from "@capacitor-community/electron";

const config: CapacitorElectronConfig = {
  appId: 'cloud.mydongle.app',
  appName: 'MyDongle Cloud',
  webDir: 'app',
  bundledWebRuntime: false,
  //loggingBehavior: none,
  electron: {
    trayIconAndMenuEnabled: true,
  }
};

export default config;
