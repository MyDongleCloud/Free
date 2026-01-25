import { Component, ViewChild, ElementRef, ChangeDetectorRef } from '@angular/core';
import { Router } from '@angular/router';
import { FormGroup, Validators, FormBuilder } from '@angular/forms';
import { HttpClient } from '@angular/common/http';
import { Global } from '../env';
import { BleService } from '../ble';

@Component({
	selector: 'app-setup',
	templateUrl: 'setup.page.html',
	standalone: false
})

export class Setup {
L(st) { return this.global.mytranslate(st); }
@ViewChild("name1E") name1E: ElementRef;
@ViewChild("name2E") name2E: ElementRef;
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

constructor(public global: Global, private router: Router, private httpClient: HttpClient, private cdr: ChangeDetectorRef, private fb: FormBuilder, public ble: BleService) {
	global.refreshUI.subscribe(event => {
		this.cdr.detectChanges();
	});
	ble.communicationEvent.subscribe((event) => {
		if (event.msg == "communication")
			this.handleBleMessage(event.data);
		if (event.msg == "connection" && ble.connectedBLE == 2)
			setTimeout(() => { this.ble.writeData({ a:"cloud" }); }, 2000);
	});
	this.formDongle = fb.group({
		"name1": [ "", [ this.checkname1 ] ],
		"shortname1": [ "", [ this.checkShortname1 ] ],
		"domain1": [ "", [ this.checkDomain1 ] ],
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
	if (data.a === "cloud" && data.all.name !== undefined) {
		this.ble.disconnect();
		await this.global.presentAlert("Denial", "This dongle is already setup. You need to reset it.", "Press the four buttons at the same time and follow the instructions on screen.");
		this.router.navigate(["/find"]);
	}
	if (data.a === "setup") {
		if (data.success === 1) {
			await this.global.presentAlert("Success!", "Your dongle is setup and ready to use!", "You will be redirected to the dasboard now.");
			this.router.navigate(["/"]);
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
	if (this?.ble?.connectedBLE !== 2 && this?.formDongle?.["controls"]?.["terms1"]?.value)
		this.errorSt = "You need to connect to a dongle";
	return this?.ble?.connectedBLE === 2 ? null : { "notconnected": true };
}

checkname1(group: FormGroup) {
	return /[a-z0-9-_]{5,20}$/i.test(group.value) ? null : {"invalid": true};
}

checkShortname1(group: FormGroup) {
	return /[a-z0-9-_]{2,20}$/i.test(group.value) ? null : {"invalid": true};
}

checkDomain1(group: FormGroup) {
	return group.value == "" || /^[a-z0-9]+([\-\.]{1}[a-z0-9]+)*\.[a-z]{2,6}$/i.test(group.value) ? null : {"invalid": true};
}

checkPassword2(group: FormGroup) {
	return group.controls.password2.value == group.controls.password2Confirm.value ? null : {"mismatch": true};
}

async verifyDns(st) {
	this.progress = true;
	const ret = await this.httpClient.post(this.global.SERVERURL + "/master/dns.json", "domain=" + encodeURIComponent(st), { headers:{ "content-type":"application/x-www-form-urlencoded" } }).toPromise();
	this.global.consolelog(1, "Master dns", ret);
	let res = false;
	if (Array.isArray(ret)) 
		ret.forEach((dns) => {
			if (/^ns[1-2]\.mydongle\.cloud$/i.test(dns?.target))
				res = true;
		});
	this.errorSt = res ? null : "DNS doesn't point correctly. You can setup later (or it can take time to propagate)";
	this.progress = false;
	this.cdr.detectChanges();
}

connectToggle() {
	this.ble.connectToggle();
}

get name1() { return this.formDongle.get("name1"); }
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
	setTimeout(() => { this.name1E.nativeElement.focus(); }, 100);
	this.cdr.detectChanges();
}

async doDongle() {
	this.progress = true;
	this.errorSt = null;
	this.show_Register();
	this.progress = false;
}

show_Register() {
	this.showDongle = false;
	this.showRegister = true;
	this.showWiFi = false;
	setTimeout(() => { this.name2E.nativeElement.focus(); }, 100);
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
		ret1 = await this.global.getCertificate(this.name1.value, this.shortname1.value, this.domain1.value != "" ? [this.domain1.value] : []); //Not used: ret1.accountKey, ret1.accountKeyId
		this.global.consolelog(2, "SETUP: Certificates", ret1);
	} catch(e) { ret1 = { fullChain:"", privateKey: "" }; }
	try {
		ret2 = await this.httpClient.post(this.global.SERVERURL + "/master/setup.json", "name=" + encodeURIComponent(this.name1.value) + "shortname=" + encodeURIComponent(this.shortname1.value) + "domains=" + encodeURIComponent(this.domain1.value) + "email=" + encodeURIComponent(this.email2.value) + "name=" + encodeURIComponent(this.name2.value) + "fullchain=" + encodeURIComponent(ret1.fullChain) + "privatekey=" + encodeURIComponent(ret1.privateKey), { headers:{ "content-type":"application/x-www-form-urlencoded" } }).toPromise();
		this.global.consolelog(2, "SETUP: Server", ret2);
	} catch(e) { ret2 = { frp:{}, ollama:{}, postfix:{} }; }
	const data = {
		a:"setup",
		cloud: {
			info: {
				name: this.name1.value,
				shortname: this.shortname1.value,
				domains: this.domain1.value != "" ? [this.domain1.value] : []
			},
			frp: ret2.frp,
			postfix: ret2.postfix,
			security: {
				signInNotification: true,
				adminSudo: false
			},
			connectivity: {
				wifi: {
					ssid: this.ssid3.value,
					password: this.password3.value
				}
			}
		},
		betterauth: {
			email: this.email2.value,
			name: this.name2.value,
			password: this.password2.value
		},
		letsencrypt: {
			fullchain: ret1.fullChain,
			privatekey :ret1.privateKey
		}
	};
	this.global.consolelog(2, "SETUP: Sending to dongle:", data);
	await this.ble.writeData(data);
	this.cdr.detectChanges();
}

}
