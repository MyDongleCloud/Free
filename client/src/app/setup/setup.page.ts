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
		"spacename1": [ "", [ Validators.required, Validators.minLength(5), Validators.maxLength(20) ] ],
		"shortname1": [ "", [ Validators.required, Validators.minLength(2), Validators.maxLength(20) ] ],
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
		"ssid3": [ "", [ Validators.required ] ],
		"password3": [ "", [ Validators.required, Validators.minLength(8) ] ]
	});
}

async handleBleMessage(data) {
	if (data.a === "space" && data.name !== undefined) {
		await this.global.presentAlert("Denial", "This dongle is already setup. You need to reset it.", "Press the four buttons at the same time and follow the instructions on screen.");
		this.global.openPage("find");
	}
	if (data.a === "setup") {
		if (data.success === 1) {
			await this.global.presentAlert("Success!", "Your dongle is setup and ready to use!", "You will be redirected to the dasboard now.");
			this.global.openPage("");
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

checkConnection = () => {
	return this?.ble?.connectedBLE === 2 ? null : { "notconnected": true };
}

checkPassword2(group: FormGroup) {
	return group.controls.password2.value == group.controls.password2Confirm.value ? null : {"mismatch": true};
}

checkDomain1(st) {
	return /^[a-z0-9]+([\-\.]{1}[a-z0-9]+)*\.[a-z]{2,6}$/i.test(st);
}

async verifyDns(st) {
	this.progress = true;
	const ret = await this.httpClient.post(this.global.SERVERURL + "/master/dns.json", "domain=" + encodeURIComponent(st), { headers:{ "content-type":"application/x-www-form-urlencoded" } }).toPromise();
	console.log("Master dns", ret);
	let res = false;
	if (Array.isArray(ret)) 
		ret.forEach((dns) => {
			if (/^ns[1-2]\.mydongle\.cloud$/i.test(dns?.target))
				res = true;
		});
	this.errorSt = res ? null : "Domain DNS doesn't point correctly.";
	this.progress = false;
	this.cdr.detectChanges();
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
	let ret1 = null;
	let ret2 = null;
	try {
		const ret1 = await this.global.getCertificate(this.spacename1.value); //Not used: ret1.accountKey, ret1.accountKeyId
		console.log("Certificates", ret1);
	} catch(e) { ret1 = { fullChain:"", privateKey: "" }; }
	try {
		ret2 = await this.httpClient.post(this.global.SERVERURL + "/master/setup.json", "spacename=" + encodeURIComponent(this.spacename1.value) + "shortname=" + encodeURIComponent(this.shortname1.value) + "domains=" + encodeURIComponent(this.domain1.value) + "email=" + encodeURIComponent(this.email2.value) + "name=" + encodeURIComponent(this.name2.value) + "fullchain=" + encodeURIComponent(ret1.fullChain) + "privatekey=" + encodeURIComponent(ret1.privateKey), { headers:{ "content-type":"application/x-www-form-urlencoded" } }).toPromise();
		console.log("Server", ret2);
	} catch(e) { ret2 = { proxy:{} }; }
	const data = { a:"setup", space:{ name:this.spacename1.value, shortname:this.shortname1.value, domains:[this.domain1.value] }, email:this.email2.value, name:this.name2.value, password: this.password2.value, ssid:this.ssid3.value, security:this.password3.value, fullchain:ret1.fullChain, privatekey:ret1.privateKey, proxy:ret2.proxy };
	await this.ble.writeData(data);
	this.cdr.detectChanges();
}

}
