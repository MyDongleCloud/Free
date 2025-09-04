import { Component, ChangeDetectorRef } from '@angular/core';
import { Global } from '../env';
import { HttpClient } from '@angular/common/http';

@Component({
	selector: 'app-splash',
	templateUrl: 'splash.page.html',
	styleUrls: ['splash.page.scss'],
	standalone: false
})

export class Splash {
showUpgrade: boolean = false;

constructor(public global: Global, private httpClient: HttpClient) {}

ionViewDidEnter() {
	this.forwardWhenReady();
}

async forwardWhenReady() {
	try {
		const response = await Promise.race([
			new Promise( resolve => { setTimeout(resolve, 2000) } ),
			this.httpClient.post(this.global.BASEURL + "/master/version.json", "", {headers:{"content-type": "application/x-www-form-urlencoded"}}).toPromise()
		]);
		if (response === undefined)
			console.log("forwardWhenReady: Timeout");
		else {
			const appVersionRequired = response["version"] ?? "";
			console.log("Required App Version: " + appVersionRequired);
			if (this.global.VERSION < appVersionRequired) {
				this.showUpgrade = true;
				return;
			}
		}
	} catch(e) { console.log("forwardWhenReady: " + e); }
	this.global.openPage("selection", false);
}

openUpgrade() {
	let url = this.global.BASEURL + "";
	if (this.global.plt.is("android"))
		url = "https://play.google.com/store/apps/details?id=cloud.mydongle.app";
	else if (this.global.plt.is("ios"))
		url = "https://apps.apple.com/us/app/in-out-sport/id";
	this.global.openBrowser(url);
}

}
