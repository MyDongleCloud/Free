import { Component, ViewChild, ChangeDetectorRef } from '@angular/core';
import { DomSanitizer, SafeResourceUrl } from '@angular/platform-browser';
import { ActivatedRoute } from '@angular/router';
import { Global } from '../env';
import modulesMeta from '../modulesmeta.json';

@Component({
	selector: 'app-wrapper',
	templateUrl: 'wrapper.page.html',
	styleUrls: ['wrapper.page.scss'],
	standalone: false
})

export class Wrapper {
module: string = "";
page: string = "";
title: string = "";
nameP: string = "";
safeUrl: SafeResourceUrl;

constructor(public global: Global, private route: ActivatedRoute, private sanitizer: DomSanitizer, private cdr: ChangeDetectorRef) {
	this.route.queryParams.subscribe((params) => {
		this.module = params?.module;
		this.page = params?.page;
		this.title = modulesMeta[this.module].title;
		this.nameP = " (" + modulesMeta[this.module].name + ")";
	});
}

ngOnInit() {
	let url;
	if (this.global.demo) {
		this.global.presentAlert("Limited demo", "Not accessible", "This app is not accessible in the limited demo. Download and run MyDongle.Cloud on your Raspberry Pi to access it.", "limitedDemo");
		url = "about:blank";
	}else
		url = location.protocol + "//" + location.host + "/m/" + this.module + (this.page ?? "");
	this.safeUrl = this.sanitizer.bypassSecurityTrustResourceUrl(url);
}

}
