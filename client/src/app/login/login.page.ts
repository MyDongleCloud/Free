import { Component, ViewChild, ChangeDetectorRef } from '@angular/core';
import { FormGroup, FormArray, Validators, FormBuilder } from '@angular/forms';
import { IonInput } from '@ionic/angular';
import { HttpClient } from '@angular/common/http';
import { Global } from '../env';
import { OnlineRet } from '../myinterface';

@Component({
	selector: 'app-login',
	templateUrl: 'login.page.html',
	styleUrls: ['login.page.scss'],
	standalone: false
})

export class Login {
public password1Show:boolean = false;
public password4Show:boolean = false;
public ready:boolean = false;
public progress:boolean = false;
public showReseting:boolean = false;
public showResetingCode:boolean = false;
public showWrongLogin:boolean = false;
public showWrongReset:boolean = false;
public showWrongResetCode:boolean = false;
public formLogin: FormGroup;
public formRegister: FormGroup;
public formReset: FormGroup;
public formResetCode: FormGroup;
@ViewChild("email1Input") email1Input: IonInput;
@ViewChild("email3Input") email3Input: IonInput;
@ViewChild("password1Input") password1Input: IonInput;

constructor(public global: Global, private httpClient: HttpClient, private cdr: ChangeDetectorRef, private fb: FormBuilder) {
	this.formLogin = fb.group({
		"email1": ["", [Validators.required, Validators.email]],
		"password1": ["", [Validators.required, Validators.minLength(2)]]
	});
	this.formReset = fb.group({
		"email3": ["", [Validators.required, Validators.email]]
	});
	this.formResetCode = fb.group({
		"resetCode4": ["", [Validators.required, Validators.minLength(6), Validators.maxLength(6)]],
		"password4": ["", [Validators.required, Validators.minLength(2)]],
		"passwordConfirm4": ["", [Validators.required, Validators.minLength(2)]],
	}, { validator: this.checkPassword4 });
}

checkPassword4(group: FormGroup) {
	return group.controls.password4.value == group.controls.passwordConfirm4.value ? null : {"mismatch": true};
}

async ionViewDidEnter() {
	if (this.global.loggedIn == 1)
		this.global.logout();
	if (this.global.loggedIn == -1)
		this.ready = true;
	else if (this.global.loggedIn == 0) {
		let count = 20;
		while (this.global.loggedIn == 0 && count-- > 0)
			this.global.sleepms(100);
		this.ready = true;
	}
	let email = this.global.settings.email ?? null;
	if (email == null)
		email = this.global.getCookie("email");
	if (email != null) {
		this.formLogin.get("email1").setValue(email);
		setTimeout(() => { this.password1Input.setFocus(); }, 100);
	}
		setTimeout(() => { this.email1Input.setFocus(); }, 100);
}

ionViewWillLeave() {
}

get email1() { return this.formLogin.get("email1"); }
get password1() { return this.formLogin.get("password1"); }
get email2() { return this.formRegister.get("email2"); }
get firstName2() { return this.formRegister.get("firstName2"); }
get lastName2() { return this.formRegister.get("lastName2"); }
get password2() { return this.formRegister.get("password2"); }
get passwordConfirm2() { return this.formRegister.get("passwordConfirm2"); }
get email3() { return this.formReset.get("email3"); }
get resetCode4() { return this.formResetCode.get("resetCode4"); }
get password4() { return this.formResetCode.get("password4"); }
get passwordConfirm4() { return this.formResetCode.get("passwordConfirm4"); }

showLogin() {
	this.showReseting = false;
	this.showResetingCode = false;
	this.cdr.detectChanges();
}

async doLogin() {
	this.progress = true;
	const ret = await this.httpClient.post(this.global.MASTERURL + "master/login.json", "email=" + encodeURIComponent(this.email1.value) + "&password=" + encodeURIComponent(this.password1.value), {headers:{"content-type": "application/x-www-form-urlencoded"}}).toPromise();
	//console.log("doLogin: " + JSON.stringify(ret));
	this.progress = false;
	this.showWrongLogin = ret["error"] != 0;
	if (ret["error"] == 0) {
		this.global.settings.user = ret["user"];
		this.global.settings.space = ret["space"];
		this.global.settings.token = ret["token"];
		this.global.settingsSave();
		this.global.loggedIn = 1;
		this.global.openPage("", false);
	} else
		this.cdr.detectChanges();
}

showReset() {
	this.showReseting = true;
	this.showResetingCode = false;
	setTimeout(() => { this.email3Input.setFocus(); }, 100);
	this.cdr.detectChanges();
}

async doReset() {
	this.progress = true;
	const ret = await this.httpClient.post(this.global.MASTERURL + "/master/login.json", "reset=1&email=" + encodeURIComponent(this.email3.value), {headers:{"content-type": "application/x-www-form-urlencoded"}}).toPromise();
	this.progress = false;
	this.showWrongReset = ret["error"] != 0;
	if (ret["error"] == 0) {
		this.showReseting = false;
		this.showResetingCode = true;
		this.showWrongLogin = false;
		this.showWrongReset = false;
		this.showWrongResetCode = false;
	}
	this.cdr.detectChanges();
}

showResetCode() {
	this.showReseting = false;
	this.showResetingCode = true;
	this.cdr.detectChanges();
}

async doResetCode() {
	this.progress = true;
	const response = await this.httpClient.post(this.global.MASTERURL + "/master/login.json", "resetCode=" + encodeURIComponent(this.resetCode4.value) + "&email=" + encodeURIComponent(this.email3.value) + "&password=" + encodeURIComponent(this.password4.value), {headers:{"content-type": "application/x-www-form-urlencoded"}}).toPromise();
	this.progress = false;
	const ret = response as OnlineRet;
	this.showWrongResetCode = ret["error"] != 0;
	if (ret["error"] == 0) {
		this.global.settings.user = ret["user"];
		this.global.settings.space = ret["space"];
		this.global.settings.token = ret["token"];
		this.global.settingsSave();
		this.global.loggedIn = 1;
		this.global.openPage("", false);
	}
	this.cdr.detectChanges();
}

}
