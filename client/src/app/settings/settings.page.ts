import { Component, ChangeDetectorRef, signal } from '@angular/core';
import { Global } from '../env';
import { HttpClient } from '@angular/common/http';
import modulesDefault from '../modulesdefault.json';
import modulesMeta from '../modulesmeta.json';



import { FormsModule } from '@angular/forms';

interface SettingsTab {
	id: string;
	name: string;
}

interface GeneralSettings {
	appName: string;
	appDescription: string;
	timezone: string;
	language: string;
}

interface SecuritySettings {
	minPasswordLength: number;
	requireUppercase: boolean;
	requireNumbers: boolean;
	requireSpecialChars: boolean;
	sessionTimeout: number;
	enable2FA: boolean;
}

interface NotificationSettings {
	email: {
		newUsers: boolean;
		systemAlerts: boolean;
		backupCompletion: boolean;
	};
	inApp: {
		showBadges: boolean;
		autoDismiss: boolean;
	};
}

interface SystemSettings {
	cacheDuration: number;
	debugMode: boolean;
	maintenanceMode: boolean;
	autoBackup: boolean;
}

interface AppSettings {
	general: GeneralSettings;
	security: SecuritySettings;
	notifications: NotificationSettings;
	system: SystemSettings;
}








@Component({
	selector: 'app-settings',
	templateUrl: './settings.page.html',
	styleUrls: ['./settings.page.scss'],
	standalone: false
})

export class Settings {
L(st) { return this.global.mytranslate(st); }
LG(st) { return this.global.mytranslateG(st); }
dResetSave: boolean = true;

constructor(public global: Global, private cdr: ChangeDetectorRef, private httpClient: HttpClient) {
	global.refreshUI.subscribe(event => {
		this.cdr.detectChanges();
	});
}

async save() {
}

async getData() {
}











 settingsTabs = signal<SettingsTab[]>([
		{ id: 'general', name: 'General' },
		{ id: 'security', name: 'Security' },
		{ id: 'notifications', name: 'Notifications' },
		{ id: 'system', name: 'System' }
	]);

	// Active tab
	activeTab = signal<string>('general');

	// Saving state
	saving = signal<boolean>(false);

	// Message state
	message = signal<string>('');
	messageType = signal<'success' | 'error'>('success');

	// Settings data
	settings = signal<AppSettings>({
		general: {
			appName: this.global.session.cloud.name,
			appDescription: 'A modern admin dashboard for managing your application',
			timezone: 'UTC',
			language: 'en'
		},
		security: {
			minPasswordLength: 8,
			requireUppercase: true,
			requireNumbers: true,
			requireSpecialChars: false,
			sessionTimeout: 30,
			enable2FA: false
		},
		notifications: {
			email: {
				newUsers: true,
				systemAlerts: true,
				backupCompletion: false
			},
			inApp: {
				showBadges: true,
				autoDismiss: true
			}
		},
		system: {
			cacheDuration: 2,
			debugMode: false,
			maintenanceMode: false,
			autoBackup: true
		}
	});

	// Set active tab
	setActiveTab(tabId: string): void {
		this.activeTab.set(tabId);
	}

	// Save settings
	async saveSettings(): Promise<void> {
		this.saving.set(true);
		
		try {
			// Simulate API call
			await new Promise(resolve => setTimeout(resolve, 1000));
			
			// TODO: Implement actual settings save logic
			this.global.consolelog(1, 'Saving settings:', this.settings());
			
			this.showMessage('Settings saved successfully!', 'success');
		} catch (error) {
			this.showMessage('Failed to save settings. Please try again.', 'error');
		} finally {
			this.saving.set(false);
		}
	}

	// Reset to defaults
	resetToDefaults(): void {
		if (confirm('Are you sure you want to reset all settings to defaults? This action cannot be undone.')) {
			this.settings.set({
				general: {
					appName: 'ngXpress Admin',
					appDescription: 'A modern admin dashboard for managing your application',
					timezone: 'UTC',
					language: 'en'
				},
				security: {
					minPasswordLength: 8,
					requireUppercase: true,
					requireNumbers: true,
					requireSpecialChars: false,
					sessionTimeout: 30,
					enable2FA: false
				},
				notifications: {
					email: {
						newUsers: true,
						systemAlerts: true,
						backupCompletion: false
					},
					inApp: {
						showBadges: true,
						autoDismiss: true
					}
				},
				system: {
					cacheDuration: 2,
					debugMode: false,
					maintenanceMode: false,
					autoBackup: true
				}
			});
			
			this.showMessage('Settings reset to defaults!', 'success');
		}
	}

	// Show message
	private showMessage(text: string, type: 'success' | 'error'): void {
		this.message.set(text);
		this.messageType.set(type);
		
		// Auto-hide message after 3 seconds
		setTimeout(() => {
			this.message.set('');
		}, 3000);
	}



}
