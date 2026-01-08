import { Component, ViewChild, ElementRef, HostListener, ChangeDetectorRef } from '@angular/core';
import { Global } from '../env';
import { HttpClient } from '@angular/common/http';
import modulesDefault from '../modulesdefault.json';
import modulesMeta from '../modulesmeta.json';

@Component({
	selector: 'app-users',
	templateUrl: './users.page.html',
	standalone: false
})

export class Users {
L(st) { return this.global.mytranslate(st); }
LM(st) { return this.global.mytranslateG(st); }
users;

constructor(public global: Global, private cdr: ChangeDetectorRef, private httpClient: HttpClient) {
	global.refreshUI.subscribe(event => {
		this.cdr.detectChanges();
	});
	this.getData();
}

async getData() {
	this.users = await this.httpClient.get("/_app_/auth/admin/list-users", {headers:{"content-type": "application/json"}}).toPromise();
	this.global.consolelog(2, "Auth list-users: ", this.users);
}


}
