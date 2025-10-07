import { Component, ChangeDetectorRef, signal } from '@angular/core';
import { FormControl, FormGroup, FormArray, Validators, FormBuilder } from '@angular/forms';
import { IonInput } from '@ionic/angular';
import { HttpClient } from '@angular/common/http';
import { Global } from '../env';
import { BleService } from '../ble';

@Component({
	selector: 'app-setup',
	templateUrl: 'setup.page.html',
	styleUrls: ['setup.page.scss'],
	standalone: false
})

export class Setup {
password2Show:boolean = false;
password3Show:boolean = true;
progress:boolean = false;
showDongle:boolean = true;
showRegister:boolean = false;
showWiFi:boolean = false;
formDongle: FormGroup;
formRegister: FormGroup;
formWiFi: FormGroup;
hasBlurredOnce: boolean = false;
errorSt = null;

constructor(public global: Global, private httpClient: HttpClient, private cdr: ChangeDetectorRef, private fb: FormBuilder, public ble: BleService) {
	global.refreshUI.subscribe(event => {
		this.cdr.detectChanges();
	});
	ble.communicationEvent.subscribe((event) => {
		if (event.msg == "communication")
			this.handleBleMessage(event.data);
		if (event.msg == "connection" && ble.connectedBLE == 2)
			setTimeout(() => { this.ble.writeData({ a:"space" }); }, 2000);
	});
	this.formDongle = fb.group({
		"spacename1": [ "", [Validators.required, Validators.minLength(6) ] ],
		"shortname1": [ "", [Validators.required, Validators.minLength(2) ] ],
		"domain1": [ "" ],
		"terms1": [ false, Validators.requiredTrue ]
	}, { validator: this.checkConnection });
	this.formRegister = fb.group({
		"name2": [ "", [ Validators.required, Validators.minLength(2) ] ],
		"email2": [ "", [ Validators.required, Validators.email ] ],
		"password2": [ "", [ Validators.required, Validators.minLength(6) ] ],
		"password2Confirm": [ "", [Validators.required ] ]
	}, { validator: this.checkPassword2 });
	this.formWiFi = fb.group({
		"ssid3": [ "", [ Validators.required, Validators.minLength(2), Validators.maxLength(2) ] ],
		"password3": [ "", [ Validators.required, Validators.minLength(8) ] ]
	});
}

async handleBleMessage(data) {
	if (data.a === "space" && data.name !== null) {
		await this.global.presentAlert("Denial", "This dongle is already setup. You need to reset it.", "Press the four buttons at the same time and follow the instructions on screen.");
		this.global.openPage("", false);
	}
	if (data.a === "setup") {
		if (data.success === 1) {
			await this.global.presentAlert("Success!", "Your dongle is setup and ready to use!", "You will be redirected to the dasboard now.");
			this.global.openPage("", false);
		} else {
			this.errorSt = "An error occured, please try again.";
			this.progress = false;
			this.cdr.detectChanges();
		}
	}
}

handleBlur(event, element) {
	const inputElement = event.target as HTMLInputElement;
	if (!this.hasBlurredOnce) {
		if (!inputElement.value)
			element.markAsUntouched();
		this.hasBlurredOnce = true;
	}
}

passwordStrength(password) {
	if (!password) return "weak";
	let score = 0;
	if (password.length >= 8) score += 1;
	if (password.length >= 12) score += 1;
	if (/[a-z]/.test(password)) score += 1;
	if (/[A-Z]/.test(password)) score += 1;
	if (/[0-9]/.test(password)) score += 1;
	if (/[^A-Za-z0-9]/.test(password)) score += 1;
	if (score <= 2) return "weak";
	if (score <= 4) return "medium";
	return "strong";
}

passwordStrengthPercentage(password) {
    if (!password) return 0;
    let score = 0;
    if (password.length >= 8) score += 1;
    if (password.length >= 12) score += 1;
    if (/[a-z]/.test(password)) score += 1;
    if (/[A-Z]/.test(password)) score += 1;
    if (/[0-9]/.test(password)) score += 1;
    if (/[^A-Za-z0-9]/.test(password)) score += 1;
    return Math.min((score / 6) * 100, 100);
}

checkConnection() {
	return this?.ble?.connectedBLE === 2 ? null : { "notconnected": true };
}

checkPassword2(group: FormGroup) {
	return group.controls.password2.value == group.controls.password2Confirm.value ? null : {"mismatch": true};
}

checkDomain1(st) {
	return /^[a-z0-9]+([\-\.]{1}[a-z0-9]+)*\.[a-z]{2,6}$/i.test(st);
}

async verifyDns(st) {
	const ret = await this.httpClient.post(this.global.SERVERURL + "/master/checkDns.json", "domain=" + encodeURIComponent(st), { headers:{ "content-type":"application/x-www-form-urlencoded" } }).toPromise();
	console.log(ret);
}

connectToggle() {
	this.ble.connectToggle();
}

get spacename1() { return this.formDongle.get("spacename1"); }
get shortname1() { return this.formDongle.get("shortname1"); }
get domain1() { return this.formDongle.get("domain1"); }
get terms1() { return this.formDongle.get("terms1"); }
get email2() { return this.formRegister.get("email2"); }
get name2() { return this.formRegister.get("name2"); }
get password2() { return this.formRegister.get("password2"); }
get password2Confirm() { return this.formRegister.get("password2Confirm"); }
get ssid3() { return this.formWiFi.get("ssid3"); }
get password3() { return this.formWiFi.get("password3"); }

show_Dongle() {
	this.showDongle = true;
	this.showRegister = false;
	this.showWiFi = false;
	this.hasBlurredOnce = false;
	setTimeout(() => { (document.getElementById("spacename1") as HTMLInputElement).focus(); }, 100);
	this.cdr.detectChanges();
}

async doDongle() {
	this.progress = true;
	this.errorSt = null;
	const data = { a:"setup", name:this.spacename1.value, alias:this.shortname1.value, domains:[this.domain1.value] };
	this.show_Register();
	this.progress = false;
}

show_Register() {
	this.showDongle = false;
	this.showRegister = true;
	this.showWiFi = false;
	setTimeout(() => { (document.getElementById("name2") as HTMLInputElement).focus(); }, 100);
	this.hasBlurredOnce = false;
	this.cdr.detectChanges();
}

async doRegister() {
	this.progress = true;
	this.errorSt = null;
	this.show_WiFi();
	this.progress = false;
}

show_WiFi() {
	this.showDongle = false;
	this.showRegister = false;
	this.showWiFi = true;
	this.hasBlurredOnce = false;
	this.cdr.detectChanges();
}

async doWiFi() {
	this.progress = true;
	this.errorSt = null;
	const ret = await this.global.getCertificate(this.spacename1.value); //Not used: ret.accountKey, ret.accountKeyId
	const data = { a:"setup", spacename:this.spacename1.value, shortname:this.shortname1.value, domains:[this.domain1.value], email:this.email2.value, name:this.name2.value, password: this.password2.value, ssid:this.ssid3.value, security:this.password3.value, fullchain:ret.fullChain, privatekey:ret.privateKey };
	await this.ble.writeData(data);
	this.cdr.detectChanges();
}

}
