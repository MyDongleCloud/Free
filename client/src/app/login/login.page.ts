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
public password2Show:boolean = false;
public password4Show:boolean = false;
public ready:boolean = false;
public progress:boolean = false;
public showRegistration:boolean = false;
public showReseting:boolean = false;
public showResetingCode:boolean = false;
public showWrongLogin:boolean = false;
public showWrongRegister:boolean = false;
public showWrongReset:boolean = false;
public showWrongResetCode:boolean = false;
public formLogin: FormGroup;
public formRegister: FormGroup;
public formReset: FormGroup;
public formResetCode: FormGroup;
@ViewChild("email1Input") email1Input: IonInput;
@ViewChild("email2Input") email2Input: IonInput;
@ViewChild("email3Input") email3Input: IonInput;
@ViewChild("password1Input") password1Input: IonInput;
@ViewChild("resetCode4Input") resetCode4Input: IonInput;

constructor(public global: Global, private httpClient: HttpClient, private cdr: ChangeDetectorRef, private fb: FormBuilder) {
	this.formLogin = fb.group({
		"email1": ["", [Validators.required, Validators.email]],
		"password1": ["", [Validators.required, Validators.minLength(6)]]
	});
	this.formRegister = fb.group({
		'email2': ['', [Validators.required, Validators.email]],
		'name2': ['', [Validators.required]],
		'password2': ['', [Validators.required, Validators.minLength(2)]],
		'passwordConfirm2': ['', [Validators.required, Validators.minLength(2)]],
	}, { validator: this.checkPassword2 });
	this.formReset = fb.group({
		"email3": ["", [Validators.required, Validators.email]]
	});
	this.formResetCode = fb.group({
		"resetCode4": ["", [Validators.required, Validators.minLength(6), Validators.maxLength(6)]],
		"password4": ["", [Validators.required, Validators.minLength(6)]],
		"passwordConfirm4": ["", [Validators.required, Validators.minLength(6)]],
	}, { validator: this.checkPassword4 });
}

checkPassword2(group: FormGroup) {
	return group.controls.password2.value == group.controls.passwordConfirm2.value ? null : {'mismatch': true};
}

checkPassword4(group: FormGroup) {
	return group.controls.password4.value == group.controls.passwordConfirm4.value ? null : {"mismatch": true};
}

async ionViewDidEnter() {
	let count = 20;
	while (this.global.session === undefined && count-- > 0)
		await this.global.sleepms(100);
	if (this.global.session != null)
		this.global.logout();
	this.ready = true;
	let email = this.global.settings.email ?? null;
	if (email == null)
		email = this.global.getCookie("email");
	if (email != null) {
		this.formLogin.get("email1").setValue(email);
		setTimeout(() => { this.password1Input.setFocus(); }, 100);
	} else
		setTimeout(() => { this.email1Input.setFocus(); }, 100);
}

ionViewWillLeave() {
}

get email1() { return this.formLogin.get("email1"); }
get password1() { return this.formLogin.get("password1"); }
get email2() { return this.formRegister.get("email2"); }
get name2() { return this.formRegister.get("name2"); }
get password2() { return this.formRegister.get("password2"); }
get passwordConfirm2() { return this.formRegister.get("passwordConfirm2"); }
get email3() { return this.formReset.get("email3"); }
get resetCode4() { return this.formResetCode.get("resetCode4"); }
get password4() { return this.formResetCode.get("password4"); }
get passwordConfirm4() { return this.formResetCode.get("passwordConfirm4"); }

showLogin() {
	this.showRegistration = false;
	this.showReseting = false;
	this.showResetingCode = false;
	this.cdr.detectChanges();
}

async doLogin() {
	this.progress = true;
	const data = { email:this.email1.value, password: this.password1.value };
	let ret = null;
	try {
		ret = await this.httpClient.post("/MyDongleCloud/Auth/sign-in/email", JSON.stringify(data), {headers:{"content-type": "application/json"}}).toPromise();
		console.log("Auth sign-in: ", ret);
	} catch(e) {}
	this.progress = false;
	this.showWrongLogin = ret == null;
	if (ret != null) {
		await this.global.getSession();
		this.global.openPage("", false);
	} else
		this.cdr.detectChanges();
}

showRegister() {
	this.showRegistration = true;
	this.showReseting = false;
	this.showResetingCode = false;
	setTimeout(() => { this.email2Input.setFocus(); }, 100);
	this.cdr.detectChanges();
}

async doRegister() {
	this.progress = true;
	const data = { email:this.email2.value, name:this.name2.value, password: this.password2.value };
	let ret = null;
	try {
		ret = await this.httpClient.post("/MyDongleCloud/Auth/sign-up/email", JSON.stringify(data), {headers:{"content-type": "application/json"}}).toPromise();
		console.log("Auth sign-up: ", ret);
	} catch(e) {}
	this.progress = false;
	this.showWrongRegister = ret == null;
	if (ret != null) {
		await this.global.getSession();
		this.global.openPage("", false);
	} else
		this.cdr.detectChanges();
}

showReset() {
	this.showRegistration = false;
	this.showReseting = true;
	this.showResetingCode = false;
	setTimeout(() => { this.email3Input.setFocus(); }, 100);
	this.cdr.detectChanges();
}

async doReset() {
	this.progress = true;
	const data = { email:this.email3.value };
	let ret = null;
	try {
		ret = await this.httpClient.post("/MyDongleCloud/Auth/forget-password/email-otp", JSON.stringify(data), {headers:{"content-type": "application/json"}}).toPromise();
		console.log("Auth doReset: ", ret);
	} catch(e) {}
	this.progress = false;
	this.showWrongReset = ret == null;
	if (ret != null) {
		this.showReseting = false;
		this.showResetingCode = true;
		this.showWrongLogin = false;
		this.showWrongReset = false;
		this.showWrongResetCode = false;
	}
	this.cdr.detectChanges();
}

showResetCode() {
	this.showRegistration = false;
	this.showReseting = false;
	this.showResetingCode = true;
	setTimeout(() => { this.resetCode4Input.setFocus(); }, 100);
	this.cdr.detectChanges();
}

async doResetCode() {
	this.progress = true;
	const data = { email:this.email3.value, otp:this.resetCode4.value, password:this.password4.value };
	let ret = null;
	try {
		ret = await this.httpClient.post("/MyDongleCloud/Auth/email-otp/reset-password", JSON.stringify(data), {headers:{"content-type": "application/json"}}).toPromise();
		console.log("Auth doResetCode: " , ret);
	} catch(e) {}
	this.progress = false;
	this.showWrongResetCode = ret == null;
	if (ret != null) {
		this.showRegistration = false;
		this.showReseting = false;
		this.showResetingCode = false;
	}
	this.cdr.detectChanges();
}

}
