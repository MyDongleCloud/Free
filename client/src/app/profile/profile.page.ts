import { Component, ViewChild, ChangeDetectorRef } from '@angular/core';
import { FormGroup, Validators, FormBuilder } from '@angular/forms';
import { IonModal } from '@ionic/angular';
import { Global } from '../env';
import { HttpClient } from '@angular/common/http';

@Component({
	selector: 'app-profile',
	templateUrl: './profile.page.html',
	standalone: false
})

export class Profile {
L(st) { return this.global.mytranslate(st); }
LG(st) { return this.global.mytranslateG(st); }
@ViewChild("modalTwoFA") modalTwoFA: IonModal;
activeTab = "security";
TwoFA;
formTwoFA: FormGroup;
showQRCode = false;
dataQRCode = " ";
password1Show:boolean = false;
progress:boolean = false;
errorSt = null;

constructor(public global: Global, private cdr: ChangeDetectorRef, private httpClient: HttpClient, private fb: FormBuilder) {
	global.refreshUI.subscribe(event => {
		this.cdr.detectChanges();
	});
	this.formTwoFA = fb.group({
		"password1": [ "", [ Validators.required ] ]
	});
}

get password1() { return this.formTwoFA.get("password1"); }

async doTwoFA() {
	this.progress = true;
	this.errorSt = null;
	const data = { password:this.password1.value, issuer:"MyDongle.Cloud" };
	let ret = null;
	try {
		ret = await this.httpClient.post("/_app_/auth/two-factor/enable", JSON.stringify(data), {headers:{"content-type": "application/json"}}).toPromise();
		this.global.consolelog(2, "Auth two-factor/enable: ", ret);
	} catch(e) { this.errorSt = e.error.message; }
	this.progress = false;
	if (ret != null) {
		this.dataQRCode = ret["totpURI"];
		this.showQRCode = true;
	} else
		this.cdr.detectChanges();
}

async showTwoFA() {
	await this.modalTwoFA.present();
}

closeTwoFA() {
	this.modalTwoFA.dismiss();
}

}
